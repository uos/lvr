#ifndef __SCANTYPES_HPP__
#define __SCANTYPES_HPP__

#include "lvr2/io/PointBuffer.hpp"
#include "lvr2/geometry/BoundingBox.hpp"
#include "lvr2/types/MatrixTypes.hpp"
#include "lvr2/registration/CameraModels.hpp"

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include <opencv2/core.hpp>

#include <memory>
#include <vector>

namespace lvr2
{

/*****************************************************************************
 * @brief Class to represent a scan within a scan project
 ****************************************************************************/
struct Scan
{
    Scan() :
        points(nullptr),
        registration(Transformd::Identity()),
        poseEstimation(Transformd::Identity()),
        thetaMin(0), thetaMax(0),
        phiMin(0), phiMax(0),
        hResolution(0),
        vResolution(0),
        pointsLoaded(false),
        positionNumber(0),
        numPoints(0),
        scanRoot(boost::filesystem::path("./"))
    {}

    ~Scan() {};

    std::string                     sensorType = "Scan";

    /// Point buffer containing the scan points
    PointBufferPtr                  points;

    /// Registration of this scan in project coordinates
    Transformd                      registration;

    /// Pose estimation of this scan in project coordinates
    Transformd                      poseEstimation;

    /// Axis aligned bounding box of this scan
    BoundingBox<BaseVector<float> > boundingBox;

    /// Min horizontal scan angle
    double                           thetaMin;

    /// Max horizontal scan angle
    double                           thetaMax;

    /// Min vertical scan angle
    double                           phiMin;

    /// Max vertical scan angle
    double                           phiMax;

    /// Horizontal resolution of used laser scanner
    double                           hResolution;

    /// Vertical resolution of used laser scanner
    double                           vResolution;

    /// Start timestamp 
    double                           startTime;

    /// End timestamp     
    double                           endTime;

    /// Indicates if all points ware loaded from the initial
    /// input file
    bool                            pointsLoaded;

    /// Scan position number of this scan in the current scan project
    int                             positionNumber;

    /// Path to root dir of this scan
    boost::filesystem::path         scanRoot;

    /// Name of the file containing the scan data
    boost::filesystem::path         scanFile;

    /// Number of points in scan
    size_t                          numPoints;
};

/// Shared pointer to scans
using ScanPtr = std::shared_ptr<Scan>;
using ScanOptional = boost::optional<Scan>;

/*****************************************************************************
 * @brief   Struct to hold a camera image together with intrinsic 
 *          and extrinsic camera parameters
 * 
 *****************************************************************************/

struct ScanImage
{
    /// Sensor type flag
    std::string                     sensorType = "ScanImage";

    /// Extrinsics 
    Extrinsicsd                     extrinsics;

    /// Extrinsics estimate
    Extrinsicsd                     extrinsicsEstimate;

    /// Path to stored image
    boost::filesystem::path         imageFile;

    /// OpenCV representation
    cv::Mat                         image;
};



using ScanImagePtr = std::shared_ptr<ScanImage>;
using ScanImageOptional = boost::optional<ScanImage>;


/*****************************************************************************
 * @brief   Represents a camera that was used at a specific scan
 *          position. The intrinsic calibration is stored in the
 *          camera's camera field. Each image has its owen orientation
 *          (extrinsic matrix) with respect to the laser scanner
 * 
 ****************************************************************************/
struct ScanCamera 
{
    /// Description of the sensor model
    std::string                     sensorType = "ScanCamera";

    /// Individual name of the camera
    std::string                     sensorName = "Camera";

    /// Pinhole camera model
    PinholeModeld                   camera;

    /// Pointer to a set of images taken at a scan position
    std::vector<ScanImagePtr>       images;
};

using ScanCameraPtr = std::shared_ptr<ScanCamera>;


/**********************************string*******************************************
 * @brief   Represents a scan position consisting of a scan and
 *          images taken at this position
 * 
 ****************************************************************************/
struct ScanPosition
{
    std::string                     sensorType = "ScanPosition";

    /// Vector of scan data. The scan position can contain several 
    /// scans. The scan with the best resolition should be stored in
    /// scans[0]. Scans can be empty
    std::vector<ScanPtr>            scans;

    /// Image data (optional, empty vector of no images were taken) 
    std::vector<ScanCameraPtr>      cams;

    /// Latitude (optional)
    double                          latitude;

    /// Longitude (optional)        
    double                          longitude;

    /// Estimated pose
    Transformd                      poseEstimate;

    /// Final registered position in project coordinates
    Transformd                      registration;

    /// Timestamp when this position was created
    double                          timestamp;

};

using ScanPositionPtr = std::shared_ptr<ScanPosition>;

/*****************************************************************************
 * @brief   Struct to represent a scan project consisting
 *          of a set of scan position. Each scan position 
 *          can consist of a laser scan and an set of acquired
 *          images. All scan position are numbered incrementally.
 *          If an optional for a scan position returns false,
 *          the corresponding data is not available for this 
 *          scan position number.
 *****************************************************************************/
struct ScanProject
{
    /// Type of used laser scanner
    std::string                     sensorType = "ScanProject";

    /// Individual name of used laser scanner
    std::string                     sensorName;

    /// Position of this scan project in world coordinates.
    /// It is assumed that all stored scan positions are in 
    /// project coordinates
    Transformd                      pose;

    /// Vector of scan positions for this project
    std::vector<ScanPositionPtr>    positions;

    /// Description (tag) of the internally used coordinate
    /// system. It is assumed that all coordinate systems 
    /// loaded with this software are right-handed
    std::string                     coordinateSystem;
};

using ScanProjectPtr = std::shared_ptr<ScanProject>;

/*****************************************************************************
 * @brief   Struct to represent a scan project with marker showing if a scan
 *          pose has been changed
 *****************************************************************************/
struct ScanProjectEditMark : ScanProject
{
    /// True if scan pose has been changed, one bool for each scan position
    std::vector<bool>    changed;
};

using ScanProjectEditMarkPtr = std::shared_ptr<ScanProjectEditMark>;

} // namespace lvr2

#endif