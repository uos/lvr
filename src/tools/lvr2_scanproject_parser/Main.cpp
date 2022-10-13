#include "Options.hpp"
#include "lvr2/types/ScanTypes.hpp"
#include "lvr2/io/baseio/BaseIO.hpp"
#include "lvr2/io/kernels/DirectoryKernel.hpp"
#include "lvr2/io/schema/ScanProjectSchemaRaw.hpp"
#include "lvr2/io/scanio/ScanProjectIO.hpp"
#include "lvr2/io/scanio/DirectoryIO.hpp"
#include "lvr2/io/schema/ScanProjectSchemaRdbx.hpp"

#include "lvr2/util/ScanProjectUtils.hpp"

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

    std::cout << inputProject << std::endl;

    // Save to target in new format
    saveScanProject(inputProject, options.getOutputSchema(), options.getOutputSource());

    return 0;
}
