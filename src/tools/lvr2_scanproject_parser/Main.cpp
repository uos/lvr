#include "lvr2/util/ScanProjectUtils.hpp"
#include "lvr2/types/ScanTypes.hpp"
#include "lvr2/io/baseio/BaseIO.hpp"
#include "lvr2/io/kernels/DirectoryKernel.hpp"
#include "lvr2/io/schema/ScanProjectSchemaRaw.hpp"
#include "lvr2/io/scanio/ScanProjectIO.hpp"
#include "lvr2/io/scanio/DirectoryIO.hpp"
#include "lvr2/io/schema/ScanProjectSchemaRdbx.hpp"

#include "Options.hpp"

#include <boost/filesystem.hpp>

#include <iostream>

using namespace lvr2;
using namespace lvr2::scanio;


int main(int argc, char** argv)
{
    // Parse options
    scanproject_parser::Options options(argc, argv);
    options.printLogo();

    // Load scan project (without fetching data)
    ScanProjectPtr inputProject = loadScanProject(options.getInputSchema(), options.getInputSource());

    if (options.printStructure())
    {
        printScanProjectStructure(inputProject);
    }

    // Pointer to the scan project we are actually working on.
    // Helpful, if only a partial project is used, i.e., for 
    // normal estimation or plain eport
    ScanProjectPtr workProject = inputProject;

    if(options.scanPositions().size())
    {
        workProject = getSubProject(inputProject, options.scanPositions());
    }

    if(options.computeNormals())
    {
        estimateProjectNormals(workProject, options.kn(), options.ki());
    }

    if(options.convert())
    {
        saveScanProject(workProject, options.getOutputSchema(), options.getOutputSource());
    }

    return 0;
}
