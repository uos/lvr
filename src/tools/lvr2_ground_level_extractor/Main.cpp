/*
 * Main.cpp
 *
 *  Created on: 18.11.2020
 *      Author: Mario Dyczka (mdyczka@uos.de)
 */


#include <iostream>
#include <memory>
#include <tuple>
#include <map>
#include <chrono>
#include <ctime>  

#include <boost/optional.hpp>

#include "lvr2/config/lvropenmp.hpp"

#include "lvr2/geometry/HalfEdgeMesh.hpp"
#include "lvr2/geometry/BaseVector.hpp"
#include "lvr2/geometry/Normal.hpp"
#include "lvr2/attrmaps/StableVector.hpp"
#include "lvr2/attrmaps/VectorMap.hpp"
#include "lvr2/algorithm/FinalizeAlgorithms.hpp"
#include "lvr2/algorithm/NormalAlgorithms.hpp"
#include "lvr2/algorithm/ColorAlgorithms.hpp"
#include "lvr2/geometry/BoundingBox.hpp"
#include "lvr2/algorithm/Tesselator.hpp"
#include "lvr2/algorithm/ClusterPainter.hpp"
#include "lvr2/algorithm/ClusterAlgorithms.hpp"
#include "lvr2/algorithm/CleanupAlgorithms.hpp"
#include "lvr2/algorithm/ReductionAlgorithms.hpp"
#include "lvr2/algorithm/Materializer.hpp"
#include "lvr2/algorithm/Texturizer.hpp"
#include "lvr2/texture/TextureFactory.hpp"
//#include "lvr2/algorithm/ImageTexturizer.hpp"

#include "lvr2/reconstruction/AdaptiveKSearchSurface.hpp"
#include "lvr2/reconstruction/BilinearFastBox.hpp"
#include "lvr2/reconstruction/TetraederBox.hpp"
#include "lvr2/reconstruction/FastReconstruction.hpp"
#include "lvr2/reconstruction/PointsetSurface.hpp"
#include "lvr2/reconstruction/SearchTree.hpp"
#include "lvr2/reconstruction/SearchTreeFlann.hpp"
#include "lvr2/reconstruction/HashGrid.hpp"
#include "lvr2/reconstruction/PointsetGrid.hpp"
#include "lvr2/reconstruction/SharpBox.hpp"
#include "lvr2/io/PointBuffer.hpp"
#include "lvr2/io/MeshBuffer.hpp"
#include "lvr2/io/ModelFactory.hpp"
#include "lvr2/io/PlutoMapIO.hpp"
#include "lvr2/util/Factories.hpp"
#include "lvr2/algorithm/GeometryAlgorithms.hpp"
#include "lvr2/algorithm/UtilAlgorithms.hpp"
#include "lvr2/registration/KDTree.hpp"
#include "lvr2/display/ColorMap.hpp"

#include "gdal.h"
#include "gdalwarper.h"

#include <Eigen/QR>

#include "lvr2/geometry/BVH.hpp"

#include "lvr2/reconstruction/DMCReconstruction.hpp"

#include "lvr2/io/PLYIO.hpp"
#include "lvr2/io/GeoTIFFIO.hpp"

#include "Options.hpp"

#if defined CUDA_FOUND
    #define GPU_FOUND

    #include "lvr2/reconstruction/cuda/CudaSurface.hpp"

    typedef lvr2::CudaSurface GpuSurface;
#elif defined OPENCL_FOUND
    #define GPU_FOUND

    #include "lvr2/reconstruction/opencl/ClSurface.hpp"
    typedef lvr2::ClSurface GpuSurface;
#endif

using boost::optional;
using std::unique_ptr;
using std::make_unique;

using namespace lvr2;

using Vec = BaseVector<float>;
using VecD = BaseVector<double>;
using PsSurface = lvr2::PointsetSurface<Vec>;

int globalTexIndex = 0;
bool preventTranslation = true;

template <typename BaseVecT>
PointsetSurfacePtr<BaseVecT> loadPointCloud(string data)
{
    // Load point cloud data and create adaptiveKSearchSuface
    
    ModelPtr baseModel = ModelFactory::readModel(data);
    if (!baseModel)
    {
        std::cout << timestamp.getElapsedTime() << "IO Error: Unable to parse " << data << std::endl;
        return nullptr;
    }
    PointBufferPtr baseBuffer = baseModel->m_pointCloud;
    PointsetSurfacePtr<BaseVecT> surface;
    surface = make_shared<AdaptiveKSearchSurface<BaseVecT>>(baseBuffer,"FLANN");
    surface->calculateSurfaceNormals();
    return surface;
}

template <typename BaseVecT, typename Data>
Texture generateHeightDifferenceTexture(const PointsetSurface<Vec>& surface ,SearchTreeFlann<BaseVecT>& tree,const lvr2::HalfEdgeMesh<VecD>& mesh, Data texelSize, 
Eigen::MatrixXd affineMatrix, string colorScale)
{
    // =======================================================================
    // Generate Bounding Box and prepare Variables
    // =======================================================================
    auto bb = surface.getBoundingBox();
    auto max = bb.getMax();
    auto min = bb.getMin();
    
    ssize_t xMax = (ssize_t)(std::round(max.x));
    ssize_t xMin = (ssize_t)(std::round(min.x));
    ssize_t yMax = (ssize_t)(std::round(max.y));
    ssize_t yMin = (ssize_t)(std::round(min.y));
    ssize_t zMax = (ssize_t)(std::round(max.z));
    ssize_t zMin = (ssize_t)(std::round(min.z));

    // Adjust Max/Min if affineMatrix was used
    if(affineMatrix.size() != 0)
    {
        Eigen::Vector4d pointMax(xMax,yMax,zMax,1);
        Eigen::Vector4d pointMin(xMin,yMin,zMin,1);

        Eigen::Vector4d solution;
        solution = affineMatrix*pointMax;
            
        auto xOldMax = solution.coeff(0);
        auto yOldMax = solution.coeff(1);

        solution = affineMatrix*pointMin;

        auto xOldMin = solution.coeff(0);
        auto yOldMin = solution.coeff(1); 

        if(xOldMin < xOldMax)
        {
            xMax = xOldMax;
            xMin = xOldMin;
        }
        else
        {
            xMax = xOldMin;
            xMin = xOldMax;
        }

        if(yOldMin < yOldMax)
        {
            yMax = yOldMax;
            yMin = yOldMin;
        }
        else
        {
            yMax = yOldMin;
            yMin = yOldMax;
        }                    
    }
    
    ssize_t xDim = (abs(xMax) + abs(xMin))/texelSize; 
    ssize_t yDim = (abs(yMax) + abs(yMin))/texelSize;    

    // Initialise the texture that will contain the height information
    Texture texture(globalTexIndex++, xDim, yDim, 3, 1, texelSize);

    // Contains the distances from each relevant point in the mesh to its closest neighbor
    Data* distance = new Data[xDim * yDim];
    Data maxDistance = 0;
    Data minDistance = std::numeric_limits<Data>::max();

    // Initialise distance vector
    for (int y = 0; y < yDim; y++)
    {
        for (int x = 0; x < xDim; x++)
        {
            distance[(yDim - y - 1) * (xDim) + x] = std::numeric_limits<Data>::min();
        }
    }

    // Get the Channel containing the point coordinates
    PointBufferPtr baseBuffer = surface.pointBuffer();   
    FloatChannel arr =  *(baseBuffer->getFloatChannel("points"));   
    MeshHandleIteratorPtr<FaceHandle> iterator = mesh.facesBegin();

    // =======================================================================
    // Iterate over all faces + calculate which Texel they are represented by
    // =======================================================================
    std::cout << timestamp.getElapsedTime() + "Generating Height Difference Texture" << std::endl;
    ProgressBar progressDistance(mesh.numFaces(), timestamp.getElapsedTime() + "Calculating Distance from Point Cloud to Model");
    
    for (size_t i = 0; i < mesh.numFaces(); i++)
    {
        BaseVecT correct(xMin,yMin,0);
        auto realPoint1 = mesh.getVertexPositionsOfFace(*iterator)[0];
        auto realPoint2 = mesh.getVertexPositionsOfFace(*iterator)[1];
        auto realPoint3 = mesh.getVertexPositionsOfFace(*iterator)[2];
        // Normalise Point Coordinates
        auto point1 = realPoint1 - correct;
        auto point2 = realPoint2 - correct;
        auto point3 = realPoint3 - correct;
        auto maxX = std::max(point1[0],std::max(point2[0],point3[0]));
        ssize_t fmaxX = (ssize_t)(maxX +1);
        auto minX = std::min(point1[0],std::min(point2[0],point3[0]));
        ssize_t fminX = (ssize_t)(minX -1);
        auto maxY = std::max(point1[1],std::max(point2[1],point3[1]));
        ssize_t fmaxY = (ssize_t)(maxY +1);
        auto minY = std::min(point1[1],std::min(point2[1],point3[1]));
        ssize_t fminY = (ssize_t)(minY -1);

        fminY = std::round(fminY/texelSize);
        fmaxY = std::round(fmaxY/texelSize);
        fminX = std::round(fminX/texelSize);
        fmaxX = std::round(fmaxX/texelSize);

        // Calculate the faces surface necessary for barycentric coordinate calculation
        Data faceSurface = 0.5 *((point2[0] - point1[0])*(point3[1] - point1[1])
            - (point2[1] - point1[1]) * (point3[0] - point1[0]));
        
        // Check Texels around the faces
        #pragma omp parallel for collapse(2)
        for (ssize_t y = fminY; y < fmaxY; y++)
        {
            for (ssize_t x = fminX; x < fmaxX; x++)
            {
                // We want the information in the center of the pixel
                Data u_x = x * texelSize + texelSize/2;
                Data u_y = y * texelSize + texelSize/2;

                // Check, if this face carries the information for texel (x,y)
                Data surface1 = 0.5 *((point2[0] - u_x)*(point3[1] - u_y)
                - (point2[1] - u_y) * (point3[0] - u_x));

                Data surface2 = 0.5 *((point3[0] - u_x)*(point1[1] - u_y)
                - (point3[1] - u_y) * (point1[0] - u_x));

                Data surface3 = 0.5 *((point1[0] - u_x)*(point2[1] - u_y)
                - (point1[1] - u_y) * (point2[0] - u_x));

                surface1 = surface1/faceSurface;
                surface2 = surface2/faceSurface;
                surface3 = surface3/faceSurface;                

                if(surface1 < 0 || surface2 < 0 || surface3 < 0)
                {
                    continue;
                }
                else
                {
                    ssize_t xTex = u_x/texelSize;
                    ssize_t yTex = u_y/texelSize;
                    if(((yDim - yTex  - 1) * (xDim) + xTex) < 0 || ((yDim - yTex  - 1) * (xDim) + xTex) > (yDim * xDim))
                    {
                        continue;
                    }

                    // Interpolate point via Barycentric Coordinates
                    // Then find nearest point in the point cloud
                    BaseVecT point = realPoint1 * surface1 + realPoint2 * surface2 + realPoint3 * surface3;
                    if(affineMatrix.size() != 0)
                    {            
                        Eigen::Vector4d p(point[0],point[1],point[2],1);

                        Eigen::Vector4d solution;
                        solution = affineMatrix.inverse() * p;
                        point[0] = solution.coeff(0);
                        point[1] = solution.coeff(1);
                        point[2] = solution.coeff(2);
                    }
                    vector<size_t> cv;  
                    vector<Data> distances;                      
                    
                    BaseVecT pointDist; 
                    pointDist[0] = point[0];
                    pointDist[1] = point[1];
                    pointDist[2] = zMax;                         
                    
                    size_t bestPoint = -1;
                    Data highestZ = zMin;

                    // Search from maximum to minimum height
                    // If we reach minimum height, stop looking --> texel is left blank
                    do
                    {
                        if(pointDist[2] <= zMin)
                        {
                            break;
                        }

                        cv.clear();
                        
                        distances.clear();
                        size_t neighbors = tree.radiusSearch(pointDist, 1000, texelSize, cv, distances);
                        for (size_t j = 0; j < neighbors; j++)
                        {
                            size_t pointIdx = cv[j];
                            auto cp = arr[pointIdx];

                            // The point we are looking for is the one with the highest z value inside the texel range
                            if(cp[2] >= highestZ)
                            {
                                if(sqrt(pow(point[0] - cp[0],2)) <= texelSize/2 && sqrt(pow(point[1] - cp[1],2)) <= texelSize/2)
                                {
                                    highestZ = cp[2];
                                    bestPoint = pointIdx;                                        
                                }
                            }
                        }   
                        // We make small steps so we dont accidentally miss points
                        pointDist[2] -= texelSize/4; 

                    } while(bestPoint == -1);  

                    if(bestPoint == -1)
                    {
                        distance[(yDim - yTex  - 1) * (xDim) + xTex] = std::numeric_limits<Data>::min();
                        continue;
                    }
                    auto p = arr[bestPoint];
                    // We only care about the height difference
                    distance[(yDim - yTex  - 1) * (xDim) + xTex] =  
                    sqrt(pow(point[2] - p[2],2));
                    if(maxDistance < distance[(yDim - yTex  - 1) * (xDim) + xTex])
                    {
                        maxDistance = distance[(yDim - yTex  - 1) * (xDim) + xTex];
                    }   

                    if(minDistance > distance[(yDim - yTex  - 1) * (xDim) + xTex])
                    {
                        minDistance = distance[(yDim - yTex  - 1) * (xDim) + xTex];
                    } 

                }

            }
            
        }
        ++progressDistance;
        ++iterator;
    }  
    std::cout << std::endl;

    // =======================================================================
    // Color the texels according to the recorded height difference
    // =======================================================================
    // color gradient behaves according to the highest distance
    // the jet color gradient is used

    ProgressBar progressColor(xDim * yDim, timestamp.getElapsedTime() + "Setting colors ");     

    ColorMap colorMap(maxDistance - minDistance);
    float color[3];
    GradientType type;
    // Get Color Scale --> Default to JET if not supported
    if(colorScale == "GREY")
    {
        type = GREY;
    }
    else if(colorScale == "JET")
    {
        type = JET;
    }
    else if(colorScale == "HOT")
    {
        type = HOT;
    }
    else if(colorScale == "HSV")
    {
        type = HSV;
    }
    else if(colorScale == "SHSV")
    {
        type = SHSV;
    }
    else if(colorScale == "SIMPSONS")
    {
        type = SIMPSONS;
    }
    else
    {
        type = JET;
    }

    for (int y = 0; y < yDim; y++)
    {
        for (int x = 0; x < xDim; x++)
        {
            if(distance[(yDim - y - 1) * (xDim) + x] == std::numeric_limits<Data>::min())
            {
                texture.m_data[(yDim - y - 1) * (xDim * 3) + x * 3 + 0] = 0;
                texture.m_data[(yDim - y - 1) * (xDim * 3) + x * 3 + 1] = 0;
                texture.m_data[(yDim - y - 1) * (xDim * 3) + x * 3 + 2] = 0;
            }
            else
            {
            colorMap.getColor(color,distance[(yDim - y - 1) * (xDim) + x] - minDistance,type);

            texture.m_data[(yDim - y - 1) * (xDim * 3) + x * 3 + 0] = color[0] * 255;
            texture.m_data[(yDim - y - 1) * (xDim * 3) + x * 3 + 1] = color[1] * 255;
            texture.m_data[(yDim - y - 1) * (xDim * 3) + x * 3 + 2] = color[2] * 255;
            }

            ++progressColor;
        }
    }
    delete distance;
    std::cout << std::endl;
    return texture;
}

std::tuple<Eigen::MatrixXd, Eigen::MatrixXd> computeAffineGeoRefMatrix(VecD* srcPoints, VecD* destPoints, int numberPoints)
{
    /* Create one M x 12 Matrix (contains the reference point coordinates in the point clouds systems), 
        one M x 1 Vector (contains the reference point coordinates in the target system)
        and one 12 x 1 Vector (will contain the transformation matrix values)*/
    // M = 3 * numberPoints
    Eigen::MatrixXd src(3 * numberPoints,12);
    Eigen::VectorXd dest(3 * numberPoints);
    for(int i = 0; i < numberPoints; i++)
    {
        src.row(i*3) << srcPoints[i].x, srcPoints[i].y, srcPoints[i].z, 1, 0, 0, 0, 0, 0, 0, 0, 0;
        src.row(i*3+1) << 0, 0, 0, 0, srcPoints[i].x, srcPoints[i].y, srcPoints[i].z, 1, 0, 0, 0, 0;
        src.row(i*3+2) << 0, 0, 0, 0, 0, 0, 0, 0, srcPoints[i].x, srcPoints[i].y, srcPoints[i].z, 1;
        dest.row(i*3) << destPoints[i].x;
        dest.row(i*3+1) << destPoints[i].y;
        dest.row(i*3+2) << destPoints[i].z;       
    }
       
    Eigen::VectorXd affineValues(12);
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(src, Eigen::ComputeFullU | Eigen::ComputeFullV);
    affineValues = svd.solve(dest);    
    Eigen::MatrixXd affineMatrix(4,4);
    // Here we seperate Translation and Rotation, because we cannot ouput models with large coordinates
    auto ar = affineValues.array();
    affineMatrix << 
    ar[0], ar[1], ar[2], 0,
    ar[4], ar[5], ar[6], 0,  
    ar[8], ar[9], ar[10], 0, 
    0, 0, 0, 1;

    Eigen::MatrixXd affineTranslation(4,4);
    affineTranslation<< 
    1, 0, 0, ar[3],
    0, 1, 0, ar[7], 
    0, 0, 1, ar[11], 
    0, 0, 0, 1;
   
    return {affineMatrix,affineTranslation};
}

// Based on the "GDAL Warp API tutorial" on https://gdal.org/tutorials/warp_tut.html
void warpGeoTIFF(GDALDatasetH& src,GDALDatasetH& dt,const std::string& geogCS, const std::string& newGeoTIFFName )
{    
    const char *pszSrcWKT = NULL;
    char *pszDstWKT  = NULL;

    // Initialise Driver
    GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
    CPLAssert( hDriver != NULL );

    // Get Coordinate Information from Source
    pszSrcWKT = GDALGetProjectionRef(src);
    CPLAssert( pszSrcWKT != NULL && strlen(src) > 0 );

    GDALDataType eDT = GDALGetRasterDataType(GDALGetRasterBand(src,1));

    // Create Coordinate Informatiom for Destination    
    OGRSpatialReference oSRS;
    oSRS.SetFromUserInput(geogCS.c_str());
    oSRS.exportToWkt(&pszDstWKT);
    CPLAssert( Og == 0 );

    void *hTransformArg;
    hTransformArg =
        GDALCreateGenImgProjTransformer( src, pszSrcWKT, NULL, pszDstWKT,
                                        FALSE, 0, 1 );
    CPLAssert( hTransformArg != NULL );
    
    // Approximated output
    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;
    CPLErr eErr;
    eErr = GDALSuggestedWarpOutput( src,
                                    GDALGenImgProjTransform, hTransformArg,
                                    adfDstGeoTransform, &nPixels, &nLines );
    
    CPLAssert( eErr == CE_None );
    GDALDestroyGenImgProjTransformer( hTransformArg );
    dt = GDALCreate( hDriver, newGeoTIFFName.c_str(), nPixels, nLines,
                        GDALGetRasterCount(src), eDT, NULL );
    CPLAssert( dt != NULL );
    
    
    GDALSetProjection( dt, pszDstWKT );
    GDALSetGeoTransform( dt, adfDstGeoTransform );  

    // Extract and Set additional raster data
    for (size_t i = 1; i <= GDALGetRasterCount(src); i++)
    {
        auto bandSrc = GDALGetRasterBand(src,i);
        auto bandDst = GDALGetRasterBand(dt,i);
        auto noData = GDALGetRasterNoDataValue( bandSrc, nullptr);
        GDALSetRasterNoDataValue(bandDst, noData);
        auto unitType = GDALGetRasterUnitType(bandSrc);
        GDALSetRasterUnitType(bandDst,unitType);
        double max;
        double min;
        double mean;
        double stdDev;
        GDALGetRasterStatistics(bandSrc,FALSE,TRUE,&min,&max,&mean,&stdDev);
        GDALSetRasterStatistics(bandDst,min,max,mean,stdDev);

        GDALColorTableH hCT;
        hCT = GDALGetRasterColorTable( GDALGetRasterBand(src,i) );
        if( hCT != NULL )
        {
            GDALSetRasterColorTable( GDALGetRasterBand(dt,i), hCT );
        }
        
    }    

    // Warp Image
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->hSrcDS = src;
    psWarpOptions->hDstDS = dt;
    psWarpOptions->nBandCount = 0;
    psWarpOptions->pfnProgress = GDALTermProgress;
    psWarpOptions->papszWarpOptions = 
    CSLSetNameValue(psWarpOptions->papszWarpOptions,"OPTIMIZE_SIZE","TRUE");

    // Reprojections transformer
    psWarpOptions->pTransformerArg =
        GDALCreateGenImgProjTransformer( src,
                                        GDALGetProjectionRef(src),
                                        dt,
                                        GDALGetProjectionRef(dt),
                                        FALSE, 0.0, 1 );
    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

    // Execute warp
    GDALWarpOperation oOperation;
    oOperation.Initialize( psWarpOptions );
    oOperation.ChunkAndWarpImage( 0, 0,
                                GDALGetRasterXSize( dt ),
                                GDALGetRasterYSize( dt ) );
    
    GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
    
    GDALDestroyWarpOptions( psWarpOptions );    

    GDALClose( dt );
    GDALClose( src );
}

template <typename BaseVecT>
void transformPoints(string src,string dt, BaseVecT* srcPoints, BaseVecT* destPoints,int numberOfPoints)
{
    // =======================================================================
    // Get Coordinate Systems and Transform Points using GDAL
    // =======================================================================
    GDALAllRegister();
    OGRSpatialReference source, target;
    source.SetFromUserInput(src.c_str());
    std::cout << std::endl;
    target.SetFromUserInput(dt.c_str());
    OGRPoint p;
    
    for(int i = 0; i < numberOfPoints; i++)
    {    
        p.assignSpatialReference(&source);  
        p.setX(srcPoints[i].x);
        p.setY(srcPoints[i].y);
        p.setZ(srcPoints[i].z);
        p.transformTo(&target); 
        
        destPoints[i] = BaseVecT(p.getX(),p.getY(),p.getZ());
    }

}

Texture readGeoTIFF(GeoTIFFIO* io, int firstBand, int lastBand, string colorScale)
{
    // =======================================================================
    // Read key Information from the TIFF
    // ======================================================================= 

    GradientType type;
    // Get Color Scale --> Default to JET if not supported
    if(colorScale == "GREY")
    {
        type = GREY;
    }
    else if(colorScale == "JET")
    {
        type = JET;
    }
    else if(colorScale == "HOT")
    {
        type = HOT;
    }
    else if(colorScale == "HSV")
    {
        type = HSV;
    }
    else if(colorScale == "SHSV")
    {
        type = SHSV;
    }
    else if(colorScale == "SIMPSONS")
    {
        type = SIMPSONS;
    }
    else
    {
        type = JET;
    }
    
    int yDimTiff = io->getRasterHeight();
    int xDimTiff = io->getRasterWidth();
    int numBands = io->getNumBands();
    double geoTransform[6];
    io->getGeoTransform(geoTransform);
    int bandRange = lastBand - firstBand + 1;
    float texelSize = geoTransform[1]; 
    // Create Texture with GeoTIFF's resolution
    Texture texture(globalTexIndex++, xDimTiff, yDimTiff, 3, 1, texelSize);
    // =======================================================================
    // Insert Band Information into Texture
    // =======================================================================
    if(bandRange == 1)
    {
        cv::Mat *mat = io->readBand(firstBand);
        double noData = io->getNoDataValue(firstBand);
        // Get minimum/maximum of band and remove comma
        int counter = 0;
        //float values[2];

        // Since faulty GeoTIFFs with no Max/Min exists, this is done manually
        /*io->getMaxMinOfBand(values,firstBand);
        
        auto max = values[0];
        auto min = values[1];*/
        double max = std::numeric_limits<double>::min();
        double min = std::numeric_limits<double>::max();
        for (ssize_t y = 0; y < yDimTiff; y++)
        {
            for (ssize_t x = 0; x < xDimTiff; x++)
            {                
                auto n = mat->at<float>((yDimTiff - y - 1) * (xDimTiff) + x);
                if(n == noData)
                {
                    continue;
                }
                if(n >= max)
                {
                    max = n;
                }
                if(n <= min)
                {
                    min = n;
                }
            }              
        }
        int multi = 1;
        if(abs(min) < 1 || abs(max) < 1)
        {
            multi = 1000;
        }
        max = (max-min)/(min+1);
        
        size_t maxV = (size_t)(max*multi);      
        // Build colorMap based on max/min
        ColorMap colorMap(maxV);
        float color[3];
        ProgressBar progressGeoTIFF(yDimTiff* xDimTiff, timestamp.getElapsedTime() + "Extracting GeoTIFF data ");
        for (ssize_t y = 0; y < yDimTiff; y++)
        {
            for (ssize_t x = 0; x < xDimTiff; x++)
            {                
                auto n = mat->at<float>((yDimTiff - y - 1) * (xDimTiff) + x);
                if(n < 0)
                {
                    texture.m_data[(yDimTiff  - y - 1) * (xDimTiff * 3) + x * 3 + 0] = 0;
                    texture.m_data[(yDimTiff  - y - 1) * (xDimTiff * 3) + x * 3 + 1] = 0;
                    texture.m_data[(yDimTiff  - y - 1) * (xDimTiff * 3) + x * 3 + 2] = 0;  
                    ++progressGeoTIFF;
                    continue;
                }
                colorMap.getColor(color, (n-min)/(min+1)*multi,type);
        
                texture.m_data[(yDimTiff  - y - 1) * (xDimTiff * 3) + x * 3 + 0] = color[0] * 255;
                texture.m_data[(yDimTiff  - y - 1) * (xDimTiff * 3) + x * 3 + 1] = color[1] * 255;
                texture.m_data[(yDimTiff  - y - 1) * (xDimTiff * 3) + x * 3 + 2] = color[2] * 255;
                ++progressGeoTIFF;                
            }              
        }
        delete(mat);
            
    }
    else if (bandRange == 3)
    {
        ProgressBar progressGeoTIFF(yDimTiff* xDimTiff*3, timestamp.getElapsedTime() + "Extracting GeoTIFF data ");
        for (int b = firstBand; b <= lastBand; b++)
        {
            cv::Mat *mat = io->readBand(b);
            double noData = io->getNoDataValue(firstBand);
            // Get minimum/maximum of band and find multipler that removes comma
            int counter = 0;
            /*float values[2];
            io->getMaxMinOfBand(values,b);
            
            int multi = 1;
            auto max = values[0];
            auto min = values[1];*/

            double max = std::numeric_limits<double>::min();
            double min = std::numeric_limits<double>::max();
            for (ssize_t y = 0; y < yDimTiff; y++)
            {
                for (ssize_t x = 0; x < xDimTiff; x++)
                {                
                    auto n = mat->at<float>((yDimTiff - y - 1) * (xDimTiff) + x);
                    if(n == noData)
                    {
                        continue;
                    }
                    if(n >= max)
                    {
                        max = n;
                    }
                    if(n <= min)
                    {
                        min = n;
                    }
                }              
            }
            auto dimV = max - min;
            for (ssize_t y = 0; y < yDimTiff; y++)
            {
                for (ssize_t x = 0; x < xDimTiff; x++)
                {                
                    auto n = mat->at<float>((yDimTiff - y - 1) * (xDimTiff) + x);
                    n /= dimV;
                    n = round(n*255);
                    if(n < 0 || n > 255)
                    {
                        n = 0;
                    }
                    texture.m_data[(yDimTiff  - y - 1) * (xDimTiff * 3) + x * 3 + (b - 1)] = n; 
                    ++progressGeoTIFF;              
                }              
            }
            delete(mat);
        }
        
    }
    else
    {
        std::cerr << "Wrong Number Of Bands ! Only 1 or 3 Bands are allowed!" << std::endl;
    } 
    std::cout << std::endl;
    return texture; 
}

template<typename BaseVecT>
MaterializerResult<BaseVecT> projectTexture(const lvr2::HalfEdgeMesh<BaseVecT>& mesh, const ClusterBiMap<FaceHandle>& clusters, const PointsetSurface<Vec>& surface, 
float texelSize, Eigen::MatrixXd affineMatrix, Eigen::MatrixXd fullAffineMatrix, GeoTIFFIO* io,SearchTreeFlann<BaseVecT>& tree, int startingBand, int numberOfBands,
string colorScale, bool noTransformation)
{
    // =======================================================================
    // Prepare necessary preconditions to create MaterializerResult
    // =======================================================================
    DenseClusterMap<Material> clusterMaterials;
    SparseVertexMap<ClusterTexCoordMapping> vertexTexCoords;
    // The keypoint_map is never utilised in the finalizer and will be ignored henceforth
    std::unordered_map<BaseVecT, std::vector<float>> keypoints_map;
    StableVector<TextureHandle, Texture> textures;

    for (auto clusterH : clusters)
    {   
        // =======================================================================
        // Generate Bounding Box for Texture coordinate calculations
        // =======================================================================
        const Cluster<FaceHandle>& cluster = clusters.getCluster(clusterH);

        auto bb = surface.getBoundingBox();
        auto min = bb.getMin();
        auto max = bb.getMax();

        ssize_t xMax = (ssize_t)(std::round(max.x));
        ssize_t xMin = (ssize_t)(std::round(min.x));
        ssize_t yMax = (ssize_t)(std::round(max.y));
        ssize_t yMin = (ssize_t)(std::round(min.y));
        ssize_t zMax = (ssize_t)(std::round(max.z));
        ssize_t zMin = (ssize_t)(std::round(min.z));

        Texture tex;
        // If a GeoTIFF was read --> Extract its Bands
        // Else, create a height difference texture
        if(io)
        {           
            tex = readGeoTIFF(io,startingBand,startingBand + numberOfBands -1, colorScale);           
        }
        else
        {
            tex = generateHeightDifferenceTexture<VecD,double>(surface,tree,mesh,texelSize,affineMatrix,colorScale);
        }     

        // Rotates the extreme Values to fit the Texture
        if(affineMatrix.size() != 0)
        {
            Eigen::Vector4d pointMax(xMax,yMax,zMax,1);
            Eigen::Vector4d pointMin(xMin,yMin,zMin,1);

            Eigen::Vector4d solution;
            solution = affineMatrix*pointMax;
            auto xOldMax = solution.coeff(0);
            auto yOldMax = solution.coeff(1);

            solution = affineMatrix*pointMin;
            auto xOldMin = solution.coeff(0);
            auto yOldMin = solution.coeff(1); 

            if(xOldMin < xOldMax)
            {
                xMax = xOldMax;
                xMin = xOldMin;
            }
            else
            {
                xMax = xOldMin;
                xMin = xOldMax;
            }

            if(yOldMin < yOldMax)
            {
                yMax = yOldMax;
                yMin = yOldMin;
            }
            else
            {
                yMax = yOldMin;
                yMin = yOldMax;
            }                
        }

        ssize_t xDim = (abs(xMax) + abs(xMin));
        ssize_t yDim = (abs(yMax) + abs(yMin));   
        BaseVecT correct(xMin,yMin,0); 

        // Code copied from Materializer.tcc; this part essentially does what the materializer does
        // save Texture as Material so it can be correctly generated by the finalizer
        Material material;
        material.m_texture = textures.push(tex);
        
        std::array<unsigned char, 3> arr = {255, 255, 255};
        
        material.m_color = std::move(arr);            
        clusterMaterials.insert(clusterH, material);

        std::unordered_set<VertexHandle> clusterVertices;
        // Get a set of all unique vertices
        for (auto faceH : cluster.handles)
        {
            for (auto vertexH : mesh.getVerticesOfFace(faceH))
            {
                clusterVertices.insert(vertexH);
            }
        }        

        // Calculate the Texture Coordinates for all Vertices
        for (auto vertexH : clusterVertices)
        {            
            auto pos = mesh.getVertexPosition(vertexH);

            // Correct coordinates            
            float yPixel = 0;
            float xPixel = 0;
            
            if(io)
            {
                // Calculate Texture Coordinates based on GeoTIFF Data
                double geoTransform[6];
                int y_dim_tiff = io->getRasterHeight();
                int x_dim_tiff = io->getRasterWidth();
                float values[2];
                io->getMaxMinOfBand(values,1);
                io->getGeoTransform(geoTransform);   
                if(preventTranslation) 
                {
                    // To correctly depict the GeoTIFFs data we need the referenced coordinates
                    if(noTransformation)
                    {
                        Eigen::Vector4d point(pos[0],pos[1],0,1);

                        Eigen::Vector4d solution;
                        solution = fullAffineMatrix*point;
                        pos[0] = solution.coeff(0);
                        pos[1] = solution.coeff(1);
                    }
                    else
                    {
                        pos[0] = pos[0] + fullAffineMatrix(12);
                        pos[1] = pos[1] + fullAffineMatrix(13);
                    } 
                    
                }
                
                xPixel = (pos[0] - geoTransform[0])/geoTransform[1];
                xPixel /= x_dim_tiff;
                yPixel = (pos[1] - geoTransform[3])/geoTransform[5];
                yPixel /= y_dim_tiff;
            }
            else
            {
                // Calculate Texture Coordinates based on normalised Model coordinates
                pos = pos - correct;
                xPixel = pos[0]/xDim;
                yPixel = 1 - pos[1]/yDim;
            }         
            
            TexCoords texCoords(xPixel,yPixel);
            if (vertexTexCoords.get(vertexH))
            {
                vertexTexCoords.get(vertexH).get().push(clusterH, texCoords);
            }
            else
            {
                ClusterTexCoordMapping mapping;
                mapping.push(clusterH, texCoords);
                vertexTexCoords.insert(vertexH, mapping);
            }
        }
    }
    
    return MaterializerResult<BaseVecT>(
        clusterMaterials,
        textures,
        vertexTexCoords,
        keypoints_map
    );

}

template <typename Data>
Data weight(Data distance)
{    
    // Calculates inverted distance
    return 1/distance;                
}

template <typename BaseVecT, typename Data>
Data findLowestZ(Data x, Data y, Data lowestZ, Data highestZ, Data searchArea, SearchTreeFlann<BaseVecT>& tree,FloatChannel& points)
{    
    Data bestZ = highestZ;
    Data currentZ = lowestZ;
    bool found = false;

    // =======================================================================
    // Look for Point with the Lowest height in the x/y coordinate
    // =======================================================================
    // Ctilises radiusSearch
    do
    {
        vector<size_t> neighbors;  
        vector<Data> distances;        
        // Look for the closest point whith a z-value lower then our currently best point
        // We increase the radius so we have the whole area the node is affected by covered
        // And later check if the nodes found are inside the square
        size_t numNeighbors = tree.radiusSearch(BaseVecT(x,y,currentZ), 100, 1.5 * searchArea, neighbors, distances);
        for (size_t j = 0; j < numNeighbors; j++)
        {
            size_t pointIdx = neighbors[j];
            auto cp = points[pointIdx];

            if(cp[2] <= bestZ)
            {
                if(sqrt(pow(x - cp[0],2)) <= searchArea && sqrt(pow(y - cp[1],2)) <= searchArea)
                {
                    bestZ = cp[2];
                    found = true;                   
                }
            }
        }   
        if(found)
        {
            return bestZ;
        }
        currentZ += searchArea;

    } while (currentZ <= highestZ);

    return std::numeric_limits<Data>::max();
    
}

template <typename BaseVecT, typename Data>
void thresholdMethod(lvr2::HalfEdgeMesh<VecD>& mesh,FloatChannel& points, PointsetSurfacePtr<Vec>& surface, float resolution,
 SearchTreeFlann<BaseVecT>& tree, int smallWindow, float smallWindowHeight, int largeWindow, float largeWindowHeight, float slopeThreshold, Eigen::MatrixXd affineMatrix)
{
    // =======================================================================
    // Generate the Bounding Box and calculate the Grids size
    // =======================================================================
    auto bb = surface->getBoundingBox();
    auto max = bb.getMax();
    auto min = bb.getMin();
    
    ssize_t xMax = (ssize_t)(std::round(max.x));
    ssize_t xMin = (ssize_t)(std::round(min.x));
    ssize_t yMax = (ssize_t)(std::round(max.y));
    ssize_t yMin = (ssize_t)(std::round(min.y));
    ssize_t zMax = (ssize_t)(std::round(max.z));
    ssize_t zMin = (ssize_t)(std::round(min.z));
    ssize_t xDim = abs(xMax) + abs(xMin);
    ssize_t yDim = abs(yMax) + abs(yMin);

    int maxNeighbors = 1;
    Data averageHeight = 0;
    Data xR = xDim/resolution;
    int xReso = std::round(xR);
    Data yR = yDim/resolution;
    int yReso = std::round(yR);

    vector<vector<Data>> workGrid;
    workGrid.resize(xReso+1, vector<Data>(yReso+1,0));
    vector<size_t> indices;  
    vector<Data> distances;  
    
    // =======================================================================
    // Generate the Grid
    // =======================================================================
    ProgressBar progressGrid(xReso * yReso, timestamp.getElapsedTime() + "Calculating Grid");
    for(ssize_t y = 0; y < yReso; y++)
    {
        for(ssize_t x = 0; x < xReso; x++)
        {
            Data u_x = x * resolution;
            Data u_y = y * resolution;

            indices.clear();
            // Set the grid node's height according to its nearest neighbors
            
            int numberNeighbors = tree.kSearch(BaseVecT(u_x + xMin,u_y + yMin,findLowestZ<BaseVecT,Data>(u_x + xMin,u_y + yMin,zMin,zMax,resolution/2,tree,points)),
             maxNeighbors, indices, distances);
            if(numberNeighbors == 0)
            {
                workGrid[x][y] = std::numeric_limits<Data>::max();
                ++progressGrid;
                continue;
            }
            averageHeight = 0;
            for(int i = 0; i < numberNeighbors; i++)
            {
                auto p = points[indices[i]];
                averageHeight += p[2];
            }
            averageHeight /= numberNeighbors;
            workGrid[x][y] = averageHeight;
            ++progressGrid;
        }
    } 
    std::cout << std::endl;
    
    // =======================================================================
    // Extraction of Ground Points via Thresholding
    // =======================================================================
    // Three steps that try to filter ground points from non-ground points
    ProgressBar progressPoints(xDim/resolution * yDim/resolution, timestamp.getElapsedTime() + "Checking Points");    
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle> dict;
    
    for (ssize_t y = 0; y < yReso; y++)
    {
        for (ssize_t x = 0; x < xReso; x++)
        {
            if(workGrid[x][y] == std::numeric_limits<Data>::max())
            {
                ++progressPoints;
                continue;
            }
            Data u_x = x * resolution;
            Data u_y = y * resolution;

            // =======================================================================
            // Small Window Thresholding
            // =======================================================================
            // check every pixel and compare with their neighbors --> sort out pixel, if larger than local minimum         
            Data lowestDist = std::numeric_limits<Data>::max();
            int swMax = (smallWindow-1)/2;
            int swMin = 0-swMax;
            for (int xwin = swMin; xwin <= swMax; xwin++)
            {
                for (int ywin = swMin; ywin <= swMax; ywin++)
                {
                    if((xwin + x) > xReso || (xwin + x) < 0)
                    {
                        continue;
                    }
                    if((ywin + y) > yReso || (ywin + y) < 0)
                    {
                        continue;
                    }
                    if(workGrid[xwin + x][ywin + y] == std::numeric_limits<Data>::max())
                    {
                        continue;
                    }
                    if(ywin == 0 && xwin == 0){
                        continue;
                    }
                    
                    if(workGrid[xwin + x][ywin + y] < lowestDist)
                    {
                        lowestDist = workGrid[xwin + x][ywin + y];
                    }
                }                
            }

            // Compare lowest z with points z
            // If height differen biggern then smallWindowHeight 
            // The point does not belong to the surface area
            if(abs(workGrid[x][y] - lowestDist) > smallWindowHeight)
            {
                ++progressPoints;
                continue;
            }

            // =======================================================================
            // Slope Thresholding
            // =======================================================================
            // Calculates the Slope between the observed point and its neighbors
            // If the Slope exerts a threshhold, the point is a non-ground point
            bool slopeGood = true;
            
            for (int xwin = -1; xwin <= 1; xwin++)
            {
                for (int ywin = -1; ywin <= 0; ywin++)
                {
                    //Only use points that we have set before
                    if(ywin == 0 && xwin == 1)
                    {
                        continue;
                    }
                    if((xwin + x) > xReso || (xwin + x) < 0)
                    {
                        continue;
                    }
                    if((ywin + y) > yReso || (ywin + y) < 0)
                    {
                        continue;
                    }
                    if(workGrid[xwin + x][ywin + y] == std::numeric_limits<Data>::max())
                    {
                        continue;
                    }
                    if(ywin == 0 && xwin == 0){
                        continue;
                    }
                    float slope = 0;
            
                    if(workGrid[xwin + x][ywin + y] != workGrid[x][y])
                    {
                        slope = atan(abs(workGrid[xwin + x][ywin + y] 
                         - workGrid[x][y])/sqrt(pow(xwin + x - x,2) + pow(ywin + y - y,2)));
                         slope = slope * 180/M_PI;
                    }                                        
                    
                    if(slope > slopeThreshold)
                    {
                        slopeGood = false;
                        break;
                    }
                }                
            }

            if(!slopeGood)
            {
                ++progressPoints;
                continue;
            }
           
            // Similar too Small Window Thresholding; used to elimnate large Objects like Trees
            lowestDist = std::numeric_limits<Data>::max();
            int lwMax = (largeWindow-1)/2;
            int lwMin = 0-swMax;
            for (int xwin = lwMin; xwin <= lwMax; xwin++)
            {
                for (int ywin = lwMin; ywin <= lwMax; ywin++)
                {
                    if((xwin + x) > xReso || (xwin + x) < 0)
                    {
                        continue;
                    }
                    if((ywin + y) > yReso || (ywin + y) < 0)
                    {
                        continue;
                    }
                    if(workGrid[xwin + x][ywin + y] == std::numeric_limits<Data>::max())
                    {
                        continue;
                    }
                    if(ywin == 0 && xwin == 0){
                        continue;
                    }

                    if(workGrid[xwin + x][ywin + y] < lowestDist)
                    {
                        lowestDist = workGrid[xwin + x][ywin + y];
                    }
                }                
            }
            
            if(abs(workGrid[x][y] - lowestDist) > largeWindowHeight)
            {
                ++progressPoints;
                continue;
            }
            
            // If the node passes through the three tests, it gets recoginised as ground point
            Data v_x = u_x+xMin;
            Data v_y = u_y+yMin;
            Data v_z = workGrid[x][y];
            // Apply transformation matrix
            if(affineMatrix.size() != 0)
            {
                Eigen::Vector4d point(v_x,v_y,v_z,1);
                Eigen::Vector4d solution;
                solution = affineMatrix*point;
                v_x = solution.coeff(0);
                v_y = solution.coeff(1);
                v_z = solution.coeff(2);
            }
            VertexHandle v = mesh.addVertex(VecD(v_x,v_y,v_z));
            dict.emplace(std::make_tuple(x,y),v);
            ++progressPoints;
        }
        
    }
    std::cout << std::endl;
    //Write nodes to mesh structure and create faces
    ProgressBar progressGrid2(dict.size(), timestamp.getElapsedTime() + "Writing Grid");
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle>::iterator pf1;
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle>::iterator pf2;

    for(auto it = dict.begin(); it != dict.end(); )
    {
        tuple t = it->first;
        ssize_t x = std::get<0>(t);
        ssize_t y = std::get<1>(t);  
       
        pf1 = dict.find(make_tuple(x,y+1));
        if(pf1 != dict.end())
        {   
            pf2 = dict.find(make_tuple(x-1,y));
            if(pf2 != dict.end())
            {
                mesh.addFace(it->second,pf1->second,pf2->second);
            }
        }

        pf1 = dict.find(make_tuple(x,y+1));
        if(pf1 != dict.end())
        {   
            pf2 = dict.find(make_tuple(x+1,y+1));
            if(pf2 != dict.end())
            {
                mesh.addFace(pf2->second,pf1->second,it->second);
            }
        }

        it++;
        ++progressGrid2;
    }

    std::cout << std::endl;
}

template <typename BaseVecT, typename Data>
void nearestNeighborMethod(lvr2::HalfEdgeMesh<VecD>& mesh, FloatChannel& points, PointsetSurfacePtr<Vec>& surface,
SearchTreeFlann<BaseVecT>& tree ,int numNeighbors, Data stepSize, Eigen::MatrixXd& affineMatrix)
{
    // =======================================================================
    // Generating Boundingbox and Initialising 
    // =======================================================================
    auto bb = surface->getBoundingBox();
    auto max = bb.getMax();
    auto min = bb.getMin();
    
    ssize_t xMax = (ssize_t)(std::round(max.x));
    ssize_t xMin = (ssize_t)(std::round(min.x));
    ssize_t yMax = (ssize_t)(std::round(max.y));
    ssize_t yMin = (ssize_t)(std::round(min.y));
    ssize_t zMax = (ssize_t)(std::round(max.z));
    ssize_t zMin = (ssize_t)(std::round(min.z));
    ssize_t xDim = abs(xMax) + abs(xMin);
    ssize_t yDim = abs(yMax) + abs(yMin);

    vector<size_t> indices;
    vector<Data> distances;
    Data finalZ = 0;         
    int trustedNeighbors = 0;
    int numberNeighbors = numNeighbors;

    xMin = xMin * (1/stepSize);
    xMax = xMax * (1/stepSize);
    yMin = yMin * (1/stepSize);
    yMax = yMax * (1/stepSize);      
    
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle> dict1;
    
    // =======================================================================
    // Calculate Vertice Height and create Hexagonial Net
    // =======================================================================
    ProgressBar progressVert((xDim / stepSize)*(yDim / stepSize), timestamp.getElapsedTime() + "Calculating Grid");
    for (ssize_t x = xMin; x < xMax; x++)
    {        
        for (ssize_t y = yMin; y < yMax; y++)
        {               
            Data u_x = x * stepSize;
            Data u_y = y * stepSize;
            indices.clear();
            distances.clear();
            finalZ = 0;  
            // Check if there are ground points near the node
            Data closeZ = findLowestZ<BaseVecT,Data>(u_x,u_y,zMin,zMax,stepSize/2,tree,points);
            if(closeZ == std::numeric_limits<Data>::max())
            {
                ++progressVert;
                continue;
            }         
            // Use Nearest Neighbor Search to find the necessary amount of neighbors     
            tree.kSearch(BaseVecT(u_x,u_y,closeZ),numberNeighbors,indices,distances);
            
            trustedNeighbors = numberNeighbors;
            for (int i = 0; i < numberNeighbors; i++)
            {
                if(distances[i] > stepSize)
                {
                    trustedNeighbors--;
                    break;
                }
                else
                {
                    auto index = indices[i];
                    auto nearest = points[index];
                    finalZ = finalZ + nearest[2];
                }
            }

            // If there are not enough points surrounding our node, it is left blank
            // Else, the nodes height value is set to its neighbors aithmetic mean
            if(trustedNeighbors < numberNeighbors)
            {
                ++progressVert;
                continue;
            }  
            else
            {
                finalZ = finalZ/trustedNeighbors;
            }

            //Valid Nodes are saved 
            Data d_x = u_x;
            Data d_y = u_y;
            Data d_z = finalZ;
            //Apply Translation Matrix
            if(affineMatrix.size() != 0)
            {
                Eigen::Vector4d point(u_x,u_y,finalZ,1);

                Eigen::Vector4d solution;
                solution = affineMatrix*point;
                d_x = solution.coeff(0);
                d_y = solution.coeff(1);
                d_z = solution.coeff(2);
            }
            VertexHandle v1 = mesh.addVertex(VecD(d_x,d_y,d_z)); 
            dict1.emplace(std::make_tuple(x,y),v1);
            ++progressVert;
        }

    }
    std::cout << std::endl;
    // Write Nodes to the mesh structure and connect them to faces
    ProgressBar progressGrid(dict1.size(), timestamp.getElapsedTime() + "Writing Grid");
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle>::iterator pf1;
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle>::iterator pf2;

    for(auto it = dict1.begin(); it != dict1.end(); )
    {
        tuple t = it->first;
        ssize_t x = std::get<0>(t);
        ssize_t y = std::get<1>(t);    
       
        pf1 = dict1.find(make_tuple(x,y+1));
        if(pf1 != dict1.end())
        {   
            pf2 = dict1.find(make_tuple(x-1,y));
            if(pf2 != dict1.end())
            {
                mesh.addFace(it->second,pf1->second,pf2->second);
            }
        }

        pf1 = dict1.find(make_tuple(x,y+1));
        if(pf1 != dict1.end())
        {   
            pf2 = dict1.find(make_tuple(x+1,y+1));
            if(pf2 != dict1.end())
            {
                mesh.addFace(pf2->second,pf1->second,it->second);
            }
        }

        it++;
        ++progressGrid;
    }

    std::cout << std::endl;
}

template <typename BaseVecT, typename Data>
void improvedMovingAverage(lvr2::HalfEdgeMesh<VecD>& mesh, FloatChannel& points, PointsetSurfacePtr<Vec>& surface,
SearchTreeFlann<BaseVecT>& tree, float minRadius, float maxRadius, int minNeighbors, int maxNeighbors, int radiusSteps, float stepSize, Eigen::MatrixXd& affineMatrix )
{
    // =======================================================================
    // Generating Boundingbox and Initialising 
    // =======================================================================
    auto bb = surface->getBoundingBox();
    auto max = bb.getMax();
    auto min = bb.getMin();

    ssize_t xMax = (ssize_t)(std::round(max.x));
    ssize_t xMin = (ssize_t)(std::round(min.x));
    ssize_t yMax = (ssize_t)(std::round(max.y));
    ssize_t yMin = (ssize_t)(std::round(min.y));
    ssize_t zMax = (ssize_t)(std::round(max.z));
    ssize_t zMin = (ssize_t)(std::round(min.z));
    ssize_t xDim = abs(xMax) + abs(xMin);
    ssize_t yDim = abs(yMax) + abs(yMin);

    size_t numberNeighbors = 0;

    int found = 0;

    xMin = xMin * (1/stepSize);
    xMax = xMax * (1/stepSize);
    yMin = yMin * (1/stepSize);
    yMax = yMax * (1/stepSize);

    Data finalZ = 0;  
    Data addedDistance = 0;
    vector<size_t> indices;  
    vector<Data> distances;  

    // Calculate the radius step size
    float radiusStepsize = (maxRadius - minRadius)/radiusSteps;
    float radius = 0;

    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle> dict;

    ProgressBar progressVert((xDim/stepSize)*(yDim/stepSize), timestamp.getElapsedTime() + "Calculating height values"); 

    // =======================================================================
    // Calculate Vertice Height and create Hexagonial Net
    // =======================================================================
    for (ssize_t x = xMin; x < xMax; x++)
    {        
        for (ssize_t y = yMin; y < yMax; y++)
        {           
            Data u_x = x * stepSize;
            Data u_y = y * stepSize;

            BaseVecT point;
            found = 0;
            finalZ = 0;  
            addedDistance = 0;
            indices.clear();
            distances.clear(); 

            radius = minRadius;
            // Use lowestZ to find the start of the ground area --> if there is no ground area, the node is skipped
            Data u_z = findLowestZ<BaseVecT,Data>(u_x,u_y,zMin,zMax,stepSize/2,tree,points);
            if(u_z == std::numeric_limits<Data>::max())
            {
                ++progressVert; 
                continue;
            }
            point = BaseVecT(u_x,u_y,u_z);
            // If we don't find enough points in the current radius, we extend the radius
            // If we hit the maximum extension and still find nothing, the node is left blank
            while (found == 0)
            {
                numberNeighbors = tree.radiusSearch(point,maxNeighbors,radius,indices,distances);
                if(numberNeighbors >= minNeighbors)
                {
                    found = 1;
                    break;
                }
                else if(radius <= maxRadius)
                {   
                    radius += radiusStepsize;
                    continue;
                }
                else
                {
                    found = -1;
                    break;
                }                 
            }
            // The nodes height value is calculated by weighting the surrounding points depending on their distance to the node
            if(found == 1)
            {
                for (int i = 0; i < numberNeighbors; i++)
                {
                    size_t pointIdx = indices[i];
                    auto neighbor = points[pointIdx];
                    // When we are exactly on the point, distance is 0 and would divide by 0
                    Data distance = 1;
                    if(distances[i] != 0)
                    {
                        distance = weight<Data>(distances[i]); 
                    }                        
                    
                    finalZ += neighbor[2] * distance;
                    addedDistance += distance;                
                }  
                
                finalZ = finalZ/addedDistance;                                 
                
            }
            else
            {
                ++progressVert; 
                continue;
            }

            // The viable nodes are saved
            Data d_x = u_x;
            Data d_y = u_y;
            Data d_z = finalZ;
            // Apply Translation Matrix
            if(affineMatrix.size() != 0)
            {
                Eigen::Vector4d point(u_x,u_y,finalZ,1);
                Eigen::Vector4d solution;
                solution = affineMatrix*point;
                d_x = solution.coeff(0);
                d_y = solution.coeff(1);
                d_z = solution.coeff(2);
            }
            VertexHandle v1 = mesh.addVertex(VecD(d_x,d_y,d_z)); 
            
            dict.emplace(std::make_tuple(x,y),v1);
            
            ++progressVert;         
        }        
        
    }
    std::cout << std::endl;
    //All nodes are put inside the mesh structure
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle>::iterator pf1;
    std::map<std::tuple<ssize_t, ssize_t>,VertexHandle>::iterator pf2;

    ProgressBar progressGrid(dict.size(), timestamp.getElapsedTime() + "Writing Grid");

    for(auto it = dict.begin(); it != dict.end(); )
    {
        tuple t = it->first;
        ssize_t x = std::get<0>(t);
        ssize_t y = std::get<1>(t);  

        pf1 = dict.find(make_tuple(x,y+1));
        if(pf1 != dict.end())
        {   
            pf2 = dict.find(make_tuple(x-1,y));
            if(pf2 != dict.end())
            {
                mesh.addFace(it->second,pf1->second,pf2->second);
            }
        }

        pf1 = dict.find(make_tuple(x,y+1));
        if(pf1 != dict.end())
        {   
            pf2 = dict.find(make_tuple(x+1,y+1));
            if(pf2 != dict.end())
            {
                mesh.addFace(pf2->second,pf1->second,it->second);
            }
        }

        it++;
        ++progressGrid;
    }

    std::cout << std::endl;
}

int main(int argc, char* argv[])
{  
    // =======================================================================
    // Parse command line parameters
    // =======================================================================
    std::cout << std::fixed;

    ground_level_extractor::Options options(argc,argv);
    options.printLogo();

    if (options.printUsage())
    {
        return 0;
    }
    std::cout << options << std::endl; 

    // =======================================================================
    // Load Pointcloud and create Model + Surface + SearchTree
    // =======================================================================
    lvr2::HalfEdgeMesh<VecD> mesh;
    auto surface = loadPointCloud<Vec>(options.getInputFileName());   
    if(surface == nullptr)
    {
        std::cout << timestamp.getElapsedTime() << "IO Error: Unable to interpret " << options.getInputFileName() << std::endl;
        return 0;
    } 
    PointBufferPtr baseBuffer = surface->pointBuffer();
    auto tree = SearchTreeFlann<VecD> (baseBuffer);

    // Get the pointcloud coordinates from the FloatChannel
    FloatChannel arr =  *(baseBuffer->getFloatChannel("points"));   
    PointsetSurfacePtr<Vec> usedSurface = surface;
    FloatChannel usedArr = arr;       
    float resolution = options.getResolution();
    float texelSize = resolution/2; 

    // Read what mode to use for DTM Creation
    int mode = -1;
    if(options.getExtractionMethod() == "NN")
    {
        mode = 1;
    }
    else if(options.getExtractionMethod() == "IMA")
    {
        mode = 0;
    }
    else if(options.getExtractionMethod() == "THM")
    {
        mode = 2;
    }
    else
    {
        std::cout << timestamp.getElapsedTime() << "IO Error: Unable to interpret " << options.getExtractionMethod() << std::endl;
        return 0;
    }

    // =======================================================================
    // Load Additional Reference Points
    // =======================================================================    
    string currenSystem;
    int numberOfPoints;
    VecD *srcPoints;
    VecD *dstPoints;
     // Read Coordinate System and Reference Points from the provided File
    if(!options.getInputReferencePairs().empty())
    {
        ifstream input;
        VecD s,d;
        char ch;
        string num;
        input.open(options.getInputReferencePairs());
        if(input.fail())
        {
            std::cout << timestamp.getElapsedTime() << "IO Error: Unable to read " << options.getExtractionMethod() << std::endl;
            return 0;
        }
        std::getline(input, currenSystem);        
        std::getline(input,num);
        numberOfPoints = std::stoi(num);        
        srcPoints = new VecD[numberOfPoints];
        dstPoints = new VecD[numberOfPoints];
        for(int i = 0; i < numberOfPoints; i++){
            input >> s.x >> ch >> s.y >> ch >> s.z;
            srcPoints[i] = s;
            input >> d.x >> ch >> d.y >> ch >> d.z;
            dstPoints[i] = d;
        }       
        
    }
    
    // =======================================================================
    // Read GeoTIFF and Warp
    // =======================================================================
    GeoTIFFIO* io = NULL;
    string newGeoTIFFName = options.getOutputFileName()+".tif";

    if(!options.getTargetSystem().empty())
    {
        if(!options.getInputReferencePairs().empty())
        {
            string targetSystem = options.getTargetSystem();
            // Transform reference points into target system
            transformPoints(currenSystem,targetSystem,dstPoints,dstPoints,numberOfPoints);

            if(!options.getInputGeoTIFF().empty())
            {
                GDALDatasetH src = GDALOpen(options.getInputGeoTIFF().c_str(),GA_ReadOnly);
                if(src == NULL)
                {
                    std::cout << timestamp.getElapsedTime() << "IO Error: Unable to read " << options.getInputGeoTIFF() << std::endl;
                }
                else
                {
                    GDALDatasetH dt;
                    // Creates a new GeoTIFF file with the transformed info of the old one
                    warpGeoTIFF(src,dt,targetSystem,newGeoTIFFName);
                    io = new GeoTIFFIO(newGeoTIFFName);
                }
            }
        }
    
    }
    else if(!options.getInputGeoTIFF().empty())
    {
        io = new GeoTIFFIO(options.getInputGeoTIFF());
    }
    
    // =======================================================================
    // Compute Affine Transform Matrix from Transformed Reff Points
    // =======================================================================
    Eigen::MatrixXd affineMatrix, affineTranslation, fullAffineMatrix, checkMatrix;
    bool noTransformation = false;
    if(!options.getInputReferencePairs().empty())
    {    
        // Right now, LVR2 doesn't support Large Coordinates and we can't use the Translation fully
        // In Functions where we use the Matrix we need to exclude the Translation 
        tie(affineMatrix,affineTranslation) = computeAffineGeoRefMatrix(srcPoints,dstPoints,numberOfPoints); 
        fullAffineMatrix = affineTranslation * affineMatrix;

        // Check, if Rotation is supported
        for (int i = 0; i < 16; i++)
        {
            // This is supposed to detect numbers that cannot be represented with float accuracy after Transformation
            // If there is an easier or more accurate way to achieve this, insert it here
            // Optimal Solution would probably be to output vertices as doubles
            if(abs(affineMatrix(i)) != 0)
            {
                if(abs(affineMatrix(i)) < 0.00001 || abs(affineMatrix(i)) > 1000)
                {
                    noTransformation = true;
                    affineMatrix = checkMatrix;
                    break;
                }
            }
        }        
    } 

    // =======================================================================
    // Extract ground from the point cloud
    // =======================================================================
    std::cout << timestamp.getElapsedTime() << "Start" << std::endl;
    if(mode == 0)
    {
        std::cout << "Moving Average" << std::endl;
        improvedMovingAverage<VecD,double>(mesh,usedArr,usedSurface,tree,options.getMinRadius(),options.getMaxRadius(),options.getNumberNeighbors(),options.getNumberNeighbors()+1,
            options.getRadiusSteps(),options.getResolution(),affineMatrix);
    }
    else if(mode == 1)
    {
        std::cout << "Nearest Neighbor" << std::endl;
        nearestNeighborMethod<VecD,double>(mesh,usedArr,usedSurface,tree,options.getNumberNeighbors(),options.getResolution(),affineMatrix);
    }
    else if(mode == 2)
    {
        std::cout << "Threshold Method"<< std::endl;
        thresholdMethod<VecD,double>(mesh,usedArr,usedSurface,options.getResolution(),tree,options.getSWSize(),options.getSWThreshold(),options.getLWSize(),options.getLWThreshold(),
            options.getSlopeThreshold(),affineMatrix);
    }
    std::cout << timestamp.getElapsedTime() << "End" << std::endl;
    
    // =======================================================================
    // Setup LVR_2 Function to allow the export of the Mesh as obj/ply
    // =======================================================================
    // Creating a cluster map made up of one cluster is necessary to use the finalizer and project the texture
    ClusterBiMap<FaceHandle> clusterBiMap;
    MeshHandleIteratorPtr<FaceHandle> iterator = mesh.facesBegin();
    auto newCluster = clusterBiMap.createCluster();
    for (size_t i = 0; i < mesh.numFaces(); i++)
    { 
        clusterBiMap.addToCluster(newCluster,*iterator);

        ++iterator;
    }  
    // Initialise Finalizer with ClusterMap
    TextureFinalizer<VecD> finalize(clusterBiMap);
    // Generate Texture for the OBJ file
    auto matResult = 
    projectTexture<VecD>(mesh,clusterBiMap,*usedSurface,texelSize,affineMatrix,fullAffineMatrix,io,tree,options.getStartingBand(),
    options.getNumberOfBands(),options.getColorScale(), noTransformation);
    // Pass Texture and Texture Coordinate into the Finalizer
    finalize.setMaterializerResult(matResult);  
    // Convert Mesh into Buffer and create Model
    auto buffer = finalize.apply(mesh);
    buffer->addIntAtomic(1, "mesh_save_textures");
    buffer->addIntAtomic(1, "mesh_texture_image_extension");
    std::cout << timestamp.getElapsedTime() << "Setting Model" << std::endl;
    auto m = ModelPtr(new Model(buffer)); 

    // =======================================================================
    // Export Files as PLY and OBJ with a JPEG as Texture
    // =======================================================================    
    std::cout << timestamp.getElapsedTime() << "Saving Model as ply" << std::endl;
    ModelFactory::saveModel(m,options.getOutputFileName() + ".ply");

    std::cout << timestamp.getElapsedTime() << "Saving Model as obj" << std::endl;
    ModelFactory::saveModel(m,options.getOutputFileName() + ".obj");  

    if(!options.getInputReferencePairs().empty())
    {
        ofstream file;
        file.open (options.getOutputFileName() + "_transformmatrix.txt");
        if(!noTransformation)
        {
            file << "Transformation Matrix without Translation\n" << affineMatrix << "\n" << "Translation\n" << affineTranslation;
        }
        else
        {
            std::cout << timestamp.getElapsedTime() << "Transformation cannot be applied without destroying the model. Full Transformation can be found in " << options.getOutputFileName() + "_transformmatrix.txt" << std::endl;
            file << "Full Transformation\n" << fullAffineMatrix;
        }
        file.close();
        delete(srcPoints);
        delete(dstPoints);

        if(!options.getInputGeoTIFF().empty())
        {
            delete(io);
        }
    }
    return 0;
}