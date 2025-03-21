/**
 * Copyright (c) 2018, University Osnabrück
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University Osnabrück nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL University Osnabrück BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Options.cpp
 *
 *  Created on: Nov 21, 2010
 *      Author: Thomas Wiemann
 */

#include "Options.hpp"

#include <sstream>

namespace scanproject_parser
{

std::string Options::getSupportedSchemasHelpString()
{
    std::stringstream ss;
    ss << "Supported HDF5 schemas are: ";

    for(auto s : lvr2::implementedHDF5Schemas)
    {
        ss << s << " ";
    }

    ss << std::endl;
    ss << "Supported directory schemas are: ";
    for(auto s : lvr2::implementedDirectorySchemas)
    {
        ss << s << " ";
    }

    return ss.str();
}

Options::Options(int argc, char** argv) : lvr2::BaseOption(argc, argv), m_descr("Scanproject tool options.\n" + getSupportedSchemasHelpString())
{

    // Create option descriptions

    m_descr.add_options()
        ("help", "Produce help message")
        ("inputSource", value<string>()->default_value(""), "Source of the input data (directory or HDF5 file)")
        ("outputSource", value<string>()->default_value(""), "Target source of converted project data (directory or HDF5 file).")
        ("inputSchema", value<string>()->default_value(""), "Schema of the input data. Has to fit the structure of the input source")
        ("outputSchema", value<string>()->default_value(""), "Schema of the output data. Has to fit the structure of the input source")
        ("plyFileName", value<string>()->default_value(""), "If defined, all scans will be exported into a single .ply file")
        ("scanpositions",  value<std::vector<size_t>>()->multitoken(), "List of scan positions to load from a scan project")
        ("kn", value<size_t>()->default_value(100), "Number of nearest neighbors for normal estimation")
        ("ki", value<size_t>()->default_value(100), "Number of nearest neighbors for normal interpolation")
        ("printStructure,p", "Print structure of the loaded scan project")
        ("computeNormals,n", "Compute normals for each scan position in the project")
        ("convert,c", "Convert and save structure in a new schema defined by outputSchema and outputStructure")
        ("reduction", value<string>()->default_value(""), "Reduce the point cloud data with given reduction method. Possible methods are OCTREE_RANDOM, OCTREE_NEAREST, OCTREE_CENTER")
        ("minPointsInVoxel", value<size_t>()->default_value(10), "Minimum number of points per voxel in voxel-based reduction.")
        ("voxelSize", value<float>()->default_value(10), "Voxel size for reduction." );
    
    
    // Parse command line and generate variables map
    positional_options_description p;
    p.add("inputDir", 1);
    store(command_line_parser(argc, argv).options(m_descr).positional(p).run(), m_variables);
    notify(m_variables);

    if (m_variables.count("help"))
    {
        ::std::cout << m_descr << ::std::endl;
        exit(-1);
    }
   
}

Options::~Options()
{
    // TODO Auto-generated destructor stub
}

} // namespace scanproject_parser
