#pragma once

#include <lvr2/io/meshio/MeshSchema.hpp>
#include <lvr2/io/meshio/MeshIO.hpp>
#include <lvr2/io/meshio/FeatureBase.hpp>
#include <lvr2/io/scanio/HDF5Kernel.hpp>

namespace lvr2
{
namespace meshio
{

using HDF5IOBase = FeatureBuild<MeshIO>;

class HDF5IO: public HDF5IOBase
{
public:
    HDF5IO(HDF5KernelPtr kernel, MeshSchemaHDF5Ptr schema)
    : HDF5IOBase(kernel, schema)
    {}

    using HDF5IOPtr = std::shared_ptr<HDF5IO>;
};

    
} // namespace meshio
} // namespace lvr2