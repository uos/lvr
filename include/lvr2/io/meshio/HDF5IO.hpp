#pragma once

#include <lvr2/io/schema/MeshSchemaHDF5.hpp>
#include <lvr2/io/meshio/MeshIO.hpp>
#include <lvr2/io/baseio/BaseIO.hpp>
#include <lvr2/io/kernels/HDF5Kernel.hpp>

namespace lvr2
{
namespace meshio
{

using HDF5IOBase = lvr2::baseio::FeatureBuild<MeshIO, MeshSchemaHDF5Ptr>;

class HDF5IO: public HDF5IOBase
{
public:
    HDF5IO(HDF5KernelPtr kernel, MeshSchemaHDF5Ptr schema)
    : HDF5IOBase(kernel, schema)
    {}
};

using HDF5IOPtr = std::shared_ptr<HDF5IO>;
    
} // namespace meshio
} // namespace lvr2