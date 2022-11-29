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
#include <thread>

using namespace lvr2;
using namespace lvr2::scanio;

#include "lvr2/util/Logging.hpp"

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

    if (options.scanPositions().size())
    {
        workProject = loadScanPositionsExplicitly(
            options.getInputSchema(),
            options.getInputSource(),
            options.scanPositions());
    }

    if(options.computeNormals())
    {
        estimateProjectNormals(workProject, options.kn(), options.ki());
    }

    if(options.convert())
    {
        saveScanProject(workProject, options.getOutputSchema(), options.getOutputSource());
    }

    if(options.getPLYFileName() != "")
    {
        writeScanProjectToPLY(workProject, options.getPLYFileName());
    }

    // for(size_t i = 0; i < 5; i++)
    // {
    //     lvr2::log << lvr2::error << "Error " << i << lvr2::endl;
    //     lvr2::log << lvr2::warning << "Warning " << i << lvr2::endl;
    //     lvr2::log << lvr2::debug << "Debug " << i << lvr2::endl;
    //     lvr2::log << lvr2::info << "Info " << i << lvr2::endl;
    //     lvr2::log << lvr2::trace << "Trace " << i << lvr2::endl;
    // }

    // lvr2::LVR2Monitor monitor( spdlog::level::debug, "Prefix text", 100);

    // for(size_t i = 0; i < 100; i++)
    // {
    //     usleep(1000000);
    //     ++monitor;
    // }

    return 0;
}
