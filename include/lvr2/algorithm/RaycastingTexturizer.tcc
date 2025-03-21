// lvr2 includes
#include "RaycastingTexturizer.hpp"
#include "lvr2/texture/Triangle.hpp"
#include "lvr2/util/Util.hpp"
#include "lvr2/util/TransformUtils.hpp"
#include "lvr2/algorithm/UtilAlgorithms.hpp"

// opencv includes
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// std includes
#include <fstream>
#include <numeric>
#include <variant>
#include <atomic>

using Eigen::Quaterniond;
using Eigen::Quaternionf;
using Eigen::AngleAxisd;
using Eigen::Translation3f;
using Eigen::Vector2i;


namespace lvr2
{

template <typename BaseVecT>
RaycastingTexturizer<BaseVecT>::RaycastingTexturizer(
    float texelMinSize,
    int texMinClusterSize,
    int texMaxClusterSize,
    const BaseMesh<BaseVector<float>>& mesh,
    const ClusterBiMap<FaceHandle>& clusters,
    const ScanProjectPtr project
): Texturizer<BaseVecT>(texelMinSize, texMinClusterSize, texMaxClusterSize)
 , m_mesh(mesh)
{
    this->setGeometry(mesh);
    this->setClusters(clusters);
    this->setScanProject(project);
}

template <typename BaseVecT>
void RaycastingTexturizer<BaseVecT>::setGeometry(const BaseMesh<BaseVecT>& mesh)
{
    m_mesh = std::cref(mesh);
    m_embreeToHandle.clear();
    MeshBufferPtr buffer = std::make_shared<MeshBuffer>();
    std::vector<float> vertices;
    std::vector<unsigned int> faceIndices;

    std::map<VertexHandle, size_t> vertexHToIndex;

    for (auto vertexH: mesh.vertices())
    {
        vertexHToIndex.insert({vertexH, vertices.size() / 3});
        auto v = mesh.getVertexPosition(vertexH);
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
    }

    // Build vertex and face array
    for (auto face: mesh.faces())
    {
        m_embreeToHandle.insert({faceIndices.size() / 3, face});
        auto faceVertices = mesh.getVerticesOfFace(face);
        for (auto vertexH: faceVertices)
        {
            faceIndices.push_back(vertexHToIndex.at(vertexH));
        }
    }

    buffer->setVertices(Util::convert_vector_to_shared_array(vertices), vertices.size() / 3);
    buffer->setFaceIndices(Util::convert_vector_to_shared_array(faceIndices), faceIndices.size() / 3);

    m_tracer = std::make_shared<EmbreeRaycaster<IntersectionT>>(buffer);
}

template <typename BaseVecT>
void RaycastingTexturizer<BaseVecT>::setClusters(const ClusterBiMap<FaceHandle>& clusters)
{
    this->m_clusters = clusters;
}

template <typename BaseVecT>
void RaycastingTexturizer<BaseVecT>::setScanProject(const ScanProjectPtr project)
{
    m_images.clear();

    // Iterate over all images and add them to m_images
    for (auto position: project->positions)
    {
        for (auto camera: position->cameras)
        {
            std::queue<std::pair<CameraImagePtr, ImageInfo>> processList;

            // Load all images
            for (auto group : camera->groups)
            { 
                for (auto elem : group->images)
                {
                    ImageInfo info;
                    info.model = camera->model;
                    auto pair = std::make_pair(elem, info);
                    processList.push(pair);
                }
            }

            while (!processList.empty())
            {
                // Get the element to process
                auto [imagePtr, info] = processList.front();                
                // Pop the element to be processed
                processList.pop();

                // Set the image
                info.image = imagePtr;

                // Total transform from image to world
                Eigen::Isometry3d total(position->transformation * camera->transformation * info.image->transformation);
                info.transform = total.cast<float>();
                info.inverse_transform = total.inverse().cast<float>();

                // Calculate camera origin in World space
                Vector3d origin_world = total * Vector3d::Zero();
                info.cameraOrigin = origin_world.cast<float>();

                // Precalculate the view direction vector in world space (only needs rotation no translation)
                Vector3d view_vec_world = total.rotation().inverse().transpose() * Vector3d::UnitZ();
                info.viewDirectionWorld = view_vec_world.normalized().cast<float>();

                //=== The Camera Matrix is stored adjusted for image resolution -> the values have to be scaled ===//
                info.image->load();
                this->equalizeHistogram(info.image->image);

                m_images.push_back(info);
            }
        }
    }

    lvr2::logout::get() << "[RaycastingTexturizer] Loaded " << m_images.size() << " images" << lvr2::endl;
}

template <typename BaseVecT>
void RaycastingTexturizer<BaseVecT>::equalizeHistogram(cv::Mat& img) const
{
    // Apply histogram equalization (works only on one channel matrices)
    // Convert to LAB color format
    cv::Mat lab;
    cv::cvtColor(img, lab, cv::COLOR_BGR2Lab);

    // Extract L channel
    std::vector<cv::Mat> channels(3);
    cv::split(lab, channels);
    cv::Mat LChannel = channels[0];

    // Apply clahe algorithm
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(); // Create algorithm
    clahe->apply(LChannel, LChannel);

    // Write the result back to the image
    channels[0] = LChannel;
    cv::merge(channels, lab);
    cv::cvtColor(lab, img, cv::COLOR_Lab2BGR);
}


inline Vector2i texelFromUV(const TexCoords& uv, const Texture& tex)
{
    size_t x = uv.u * tex.m_width;
    size_t y = uv.v * tex.m_height;
    x = std::min<size_t>({x, (size_t) tex.m_width - 1});
    y = std::min<size_t>({y, (size_t) tex.m_height - 1});
    return Vector2i(x, y);
}

inline TexCoords uvFromTexel(const Vector2i& texel, const Texture& tex)
{
    return TexCoords(
        ((float) texel.x() + 0.5f) / tex.m_width,
        ((float) texel.y() + 0.5f) / tex.m_height
    );
}

inline void setPixel(size_t index, Texture& tex, cv::Vec3b color)
{
    tex.m_data[3 * index + 0] = color[0];
    tex.m_data[3 * index + 1] = color[1];
    tex.m_data[3 * index + 2] = color[2];
}

inline void setPixel(uint16_t x, uint16_t y, Texture& tex, cv::Vec3b color)
{
    size_t index = (y * tex.m_width) + x;
    setPixel(index, tex, color);
}

inline void setPixel(TexCoords uv, Texture& tex, cv::Vec3b color)
{
    auto p = texelFromUV(uv, tex);
    setPixel(p.x(), p.y(), tex, color);
}

template <typename BaseVecT>
TextureHandle RaycastingTexturizer<BaseVecT>::generateTexture(
    int index,
    const PointsetSurface<BaseVecT>&,
    const BoundingRectangle<typename BaseVecT::CoordType>& boundingRect,
    ClusterHandle clusterH
)
{
    // Calculate the texture size
    unsigned short int sizeX = ceil(abs(boundingRect.m_maxDistA - boundingRect.m_minDistA) / this->m_texelSize);
    unsigned short int sizeY = ceil(abs(boundingRect.m_maxDistB - boundingRect.m_minDistB) / this->m_texelSize);
    // Make sure the texture contains at least 1 pixel
    sizeX = std::max<unsigned short int>({sizeX, 1});
    sizeY = std::max<unsigned short int>({sizeY, 1});
    
    TextureHandle texH = this->m_textures.push(
        this->initTexture(
            index,
            sizeX,
            sizeY,
            3,
            1,
            this->m_texelSize
        )
    );

    if (m_images.size() == 0)
    {
        std::cout << timestamp << "[RaycastingTexturizer] No images set, cannot texturize cluster" << std::endl;
        return texH;
    }

    // Paint all faces
    #pragma omp parallel for
    for (FaceHandle faceH: m_clusters.getCluster(clusterH))
    {
        this->paintTriangle(texH, faceH, boundingRect);
    }

    return texH;
}

template <typename BaseVecT>
template <typename... Args>
Texture RaycastingTexturizer<BaseVecT>::initTexture(Args&&... args) const
{
    Texture ret(std::forward<Args>(args)...);

    ret.m_layerName = "RGB";
    size_t num_pixel = ret.m_width * ret.m_height;

    // Init black
    for (int i = 0; i < num_pixel; i++)
    {
        ret.m_data[i * 3 + 0] = 0;
        ret.m_data[i * 3 + 1] = 0;
        ret.m_data[i * 3 + 2] = 0;
    }

    return std::move(ret);
}


template <typename BaseVecT>
void RaycastingTexturizer<BaseVecT>::paintTriangle(
    TextureHandle texH,
    FaceHandle faceH,
    const BoundingRectangle<typename BaseVecT::CoordType>& bRect)
{
    // Texture
    Texture& tex = this->m_textures[texH];
    // Corners in 3D
    std::array<BaseVecT, 3UL> vertices = m_mesh.get().getVertexPositionsOfFace(faceH);
    // The triangle in 3D World space
    auto worldTriangle = Triangle<Vector3f, float>(
        Vector3f(vertices[0].x, vertices[0].y, vertices[0].z),
        Vector3f(vertices[1].x, vertices[1].y, vertices[1].z),
        Vector3f(vertices[2].x, vertices[2].y, vertices[2].z)
    );
    // Vertices in texture uvs
    std::array<TexCoords, 3UL> triUV;
    triUV[0] = this->calculateTexCoords(texH, bRect, vertices[0]);
    triUV[1] = this->calculateTexCoords(texH, bRect, vertices[1]);
    triUV[2] = this->calculateTexCoords(texH, bRect, vertices[2]);

    // The triangle in texel space
    Triangle<Vector2i, double> texelTriangle(
        texelFromUV(triUV[0], tex),
        texelFromUV(triUV[1], tex),
        texelFromUV(triUV[2], tex)
    );

    Triangle<Vector2f, float> subPixelTriangle(
        Vector2f(triUV[0].u * tex.m_width, triUV[0].v * tex.m_height),
        Vector2f(triUV[1].u * tex.m_width, triUV[1].v * tex.m_height),
        Vector2f(triUV[2].u * tex.m_width, triUV[2].v * tex.m_height)
    );

    // Rank images for triangle
    std::vector<ImageInfo> images = this->rankImagesForTriangle(worldTriangle);

    // Determine texel bb
    auto [minP, maxP] = texelTriangle.getAABoundingBox();
    
    // Lambda to process a texel // uv has to be between 0 and tex_width/tex_heigth not 0 and 1
    auto ProcessTexel = [&](Vector2f uv, Vector2i texel)
    {
        // Skip texel if not inside this triangle
        if (!subPixelTriangle.contains(uv)) return;
        // Calc barycentric coordinates
        Vector3f barycentrics = subPixelTriangle.barycentric(uv);
        // Calculate 3D point using barycentrics
        Vector3f pointWorld = worldTriangle.point(barycentrics);
        // Set pixel color
        this->paintTexel(texH, faceH, texel, pointWorld, images);
    };

    // Lambda for processing the texel on the triangle sides
    auto ProcessSideTexel = [&](Vector2i texel)
    {
        // Calculate uv coordiantes of the corners and the center;
        Vector2f center(texel.x() + 0.5f, texel.y() + 0.5f);
        // Top Left
        Vector2f topLeft(
            (float) texel.x(),
            (float) texel.y()
        );
        // Top Right
        Vector2f topRight(
            (float) texel.x() + 1.0f,
            (float) texel.y()
        );
        // Bottom Left
        Vector2f botLeft(
            (float) texel.x(),
            (float) texel.y() + 1.0f
        );
        // Bottom Right
        Vector2f botRight(
            (float) texel.x() + 1.0f,
            (float) texel.y() + 1.0f
        );

        if (subPixelTriangle.contains(center))
        {
            ProcessTexel(center, texel);
        }
        else if (subPixelTriangle.contains(topLeft))
        {
            ProcessTexel(topLeft, texel);
        }
        else if (subPixelTriangle.contains(topRight))
        {
            ProcessTexel(topRight, texel);
        }
        else if (subPixelTriangle.contains(botLeft))
        {
            ProcessTexel(botLeft, texel);
        }
        else if (subPixelTriangle.contains(botRight))
        {
            ProcessTexel(botRight, texel);
        }

    };

    // Iterate sides of the triangle because these texels need special treatment due to the center not always being inside the triangle
    rasterize_line(subPixelTriangle.A(), subPixelTriangle.B(), ProcessSideTexel);
    rasterize_line(subPixelTriangle.B(), subPixelTriangle.C(), ProcessSideTexel);
    rasterize_line(subPixelTriangle.C(), subPixelTriangle.A(), ProcessSideTexel);

    // Iterate bb and check if texel center is inside the triangle
    for (int y = minP.y(); y <= maxP.y(); y++ )
    {
        for (int x = minP.x(); x <= maxP.x(); x++)
        {
            ProcessTexel(Vector2f(x + 0.5f, y + 0.5f), Vector2i(x, y));
        }
    }
}

template <typename BaseVecT>
void RaycastingTexturizer<BaseVecT>::paintTexel(
    TextureHandle texH,
    FaceHandle faceH,
    Vector2i texel,
    Vector3f point,
    const std::vector<ImageInfo>& images)
{
    for (ImageInfo img: images)
    {   
        // Check if the point is visible
        if (!this->isVisible(img.cameraOrigin, point, faceH)) continue;

        cv::Vec3b color;
        // If the color could not be calculated process next image
        if (!this->calcPointColor(point, img, color)) continue;
        
        setPixel(texel.x(), texel.y(), this->m_textures[texH], color);
        
        // After the pixel is texturized we are done
        return;
    }
}

template <typename BaseVecT>
std::vector<typename RaycastingTexturizer<BaseVecT>::ImageInfo> RaycastingTexturizer<BaseVecT>::rankImagesForTriangle(const Triangle<Vector3f, float>& triangle) const
{
    Vector3f normal = triangle.normal();
    Vector3f center = triangle.center();

    // Initially all images are considered
    std::vector<ImageInfo> consideration_set = m_images;

    // Filter out images pointing away from the triangle
    auto partition_point = std::partition(
        consideration_set.begin(),
        consideration_set.end(),
        [&](ImageInfo elem)
        {
            Vector3f to_triangle = (center - elem.cameraOrigin).normalized();
            
            // 1 is 0 deg -1 is 180
            float sin_angle = to_triangle.dot(elem.viewDirectionWorld);
            // If the angle is smaller than 90 deg keep the image
            return sin_angle > 0;
        }
    );
    // Remove all elements in the second partition
    consideration_set.erase(partition_point, consideration_set.end());

    std::vector<std::pair<ImageInfo, float>> ranked;

    std::transform(
        consideration_set.begin(),
        consideration_set.end(),
        std::back_insert_iterator(ranked),
        [normal, center](ImageInfo img)
        {
            // View vector in world coordinates
            Vector3f view = img.viewDirectionWorld;
            // Check if triangle is seen from the back // Currently not working because the normals are not always correct
            // if (normal.dot(view) < 0)
            // {
            //     return std::make_pair(img, 0.0f);
            // }

            // Cosine of the angle between the view vector of the image and the cluster normal
            float angle = view.dot(normal);

            return std::make_pair(img, std::abs(angle)); // Use abs because values can be negative if the normal points the wrong direction
        });

    // Sort the Images by the angle between viewvec and normal
    std::sort(
        ranked.begin(),
        ranked.end(),
        [](const auto& first, const auto& second)
        {
            return first.second > second.second;
        });

    auto is_valid = [](auto& elem){ return elem.second > 0.0f;};
    // Extract all images which face the camera
    // (currently not working because the normals are not consistently on the
    // right side therefore no images have negative value because we use abs)
    size_t numValidImages = std::count_if(ranked.begin(), ranked.end(), is_valid);
    auto pastValidImageIt = std::partition_point(ranked.begin(), ranked.end(), is_valid);
    std::vector<ImageInfo> ret;

    std::transform(
        ranked.begin(),
        pastValidImageIt,
        std::back_insert_iterator(ret),
        [](auto pair)
        {
            return pair.first;
        }
    );

    // Stable sort the images by distance to the cluster
    // This keeps the relative order of images which have the same distance
    std::stable_sort(
        ret.begin(),
        ret.end(),
        [center](const auto& lhs, const auto& rhs)
        {
            double lhs_dist = (center - lhs.cameraOrigin).squaredNorm();
            double rhs_dist = (center - rhs.cameraOrigin).squaredNorm();
            double diff = std::abs(lhs_dist - rhs_dist);
            // Centimeter accuracy is enough
            if (diff < 0.01f) return false;

            return lhs_dist < rhs_dist;
        }
    );

    // Return max 10 Images, this should be enough if we need 
    // more then 10 the images are probably to far away to have good data anyway
    // and we can reduce the amount of traced rays especially for large amounts of images
    if (ret.size() <= 10)
    {
        return ret;
    }

    std::vector<ImageInfo> reduced;
    std::copy_n(ret.begin(), 10, std::back_inserter(reduced));

    return reduced;
}

template <typename BaseVecT>
bool RaycastingTexturizer<BaseVecT>::isVisible(Vector3f origin, Vector3f point, FaceHandle faceH) const
{
    // Cast ray to point
    IntersectionT intersection;
    bool hit = this->m_tracer->castRay( origin, (point - origin).normalized(), intersection);
    // Did not hit anything
    if (!hit) return false;
    // Dit not hit the cluster we are interested in
    FaceHandle hitFaceH = m_embreeToHandle.at(intersection.face_id);
    // Wrong face
    if (faceH != hitFaceH) return false;
    float dist = (intersection.point - point).norm();
    // Default
    return true;
}

template <typename BaseVecT>
bool RaycastingTexturizer<BaseVecT>::calcPointColor(Vector3f point, const ImageInfo& img, cv::Vec3b& color) const
{
    // Transform the point to camera space
    Vector3f camPoint = img.inverse_transform * point;
    // If the point is behind the camera no color will be extracted
    if (camPoint.z() <= 0) return false;

    // Project the point to the camera image
    Vector2f uv = img.model.projectPoint(camPoint);
    // Distort the uv coordinates
    auto distorted = img.model.distortPoint(uv);
    size_t imgX = std::floor(distorted.x());
    size_t imgY = std::floor(distorted.y());

    // Skip if the projected pixel is outside the camera image
    if (imgX < 0 || imgY < 0 || imgX >= img.image->image.cols || imgY >= img.image->image.rows) return false;

    // Calculate color
    color = img.image->image.template at<cv::Vec3b>(imgY, imgX);
    return true;
}

} // namespace lvr2

