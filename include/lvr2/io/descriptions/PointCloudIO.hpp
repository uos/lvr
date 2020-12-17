#pragma once

#ifndef LVR2_IO_HDF5_POINTBUFFERIO_HPP
#define LVR2_IO_HDF5_POINTBUFFERIO_HPP

#include <boost/optional.hpp>

#include "lvr2/io/PointBuffer.hpp"
#include "lvr2/registration/ReductionAlgorithm.hpp"

// Dependencies
#include "ChannelIO.hpp"
#include "VariantChannelIO.hpp"

namespace lvr2 {

/**
 * @class PointCloudIO 
 * @brief Hdf5IO Feature for handling PointBuffer related IO
 * 
 * This Feature of the Hdf5IO handles the IO of a PointBuffer object.
 * 
 * Example:
 * @code
 * MyHdf5IO io;
 * PointBufferPtr pointcloud, pointcloud_in;
 * 
 * // writing
 * io.open("test.h5");
 * io.save("apointcloud", pointcloud);
 * 
 * // reading
 * pointcloud_in = io.loadPointCloud("apointcloud");
 * 
 * @endcode
 * 
 * Generates attributes:
 * - IO: PointCloudIO
 * - CLASS: PointBuffer
 * 
 * Dependencies:
 * - VariantChannelIO
 * 
 */
template<typename FeatureBase>
class PointCloudIO 
{
public:
    /**
     * @brief Save a point buffer at the position defined by \ref group and \ref container
     * 
     * @param group             Group with the point cloud data 
     * @param container         Container of the point cloud data
     * @param buffer            Point cloud data
     */
    void savePointCloud(const std::string& group, const std::string& container, const PointBufferPtr& buffer);

    /**
     * @brief  Loads a point cloud
     * 
     * @param group             Group with the point cloud data 
     * @param container         Container of the point cloud data
     * @return PointBufferPtr   A point buffer containing the point 
     *                          cloud data stored at the position 
     *                          defined by \ref group and \ref container
     */
    PointBufferPtr loadPointCloud(const std::string& group, const std::string& container);

    /**
     * @brief Loads a reduced version of a point cloud
     * 
     * @param group             Group with the point cloud data 
     * @param container         Container of the point cloud data
     * @param reduction         A reduction object that is used to generate the reduced data
     * @return PointBufferPtr   A point buffer containing a reduced version of the point 
     *                          cloud data stored at the position 
     *                          defined by \ref group and \ref container
     */
    PointBufferPtr loadPointCloud( const std::string& group, const std::string& container, 
        const ReductionAlgorithm& reduction);
    
protected:

    /// Checks whether the indicated group contains point cloud data
    bool isPointCloud(const std::string& group);

    /// Add access to feature base
    FeatureBase* m_FeatureBase = static_cast<FeatureBase*>(this);

    /// Dependencies
    VariantChannelIO<FeatureBase>* m_vchannel_io = static_cast<VariantChannelIO<FeatureBase>*>(m_file_access);

    /// Class ID
    static constexpr const char* ID = "PointCloudIO";

    /// Object ID
    static constexpr const char* OBJID = "PointBuffer";
};

template<typename FeatureBase>
struct FeatureConstruct<PointCloudIO, FeatureBase> {
    
    // DEPS
    using deps = typename FeatureConstruct<VariantChannelIO, FeatureBase>::type;

    // add actual feature
    using type = typename deps::template add_features<PointCloudIO>::type;
     
};

} // namespace lvr2 

#include "PointCloudIO.tcc"


#endif // LVR2_IO_HDF5_POINTBUFFERIO_HPP