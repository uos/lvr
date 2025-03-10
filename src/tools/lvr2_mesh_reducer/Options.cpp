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

#include "lvr2/config/lvropenmp.hpp"

#include <fstream>

namespace meshreduce
{

using namespace boost::program_options;

Options::Options(int argc, char** argv) : BaseOption(argc, argv)
{
    // Create option descriptions
    m_descr.add_options()("help", "Produce help message")(
        "inputFile",
            value<vector<string>>(),
            "Input file name. Supported formats are .obj and .ply")(
        "reductionRatio,r",
            value<float>(&m_edgeCollapseReductionRatio)->default_value(0.0),
            "Percentage of faces to remove via edge-collapse (0.0 means no reduction, 1.0 means to "
            "remove all faces which can be removed)")(
        "hem,m",
            value<string>(&m_hemImplementation)->default_value("pmp"),
            "Half edge mesh (HEM) implementation. Default: pmp. Availaible: pmp, lvr");
    setup();
}

string Options::getInputFileName() const
{
    return (m_variables["inputFile"].as<vector<string>>())[0];
}

string Options::getHemImplementation() const
{
    return (m_variables["hem"].as<string>());
}

float Options::getEdgeCollapseReductionRatio() const
{
    return (m_variables["reductionRatio"].as<float>());
}

bool Options::printUsage() const
{
    if (m_variables.count("help"))
    {
        cout << endl;
        cout << m_descr << endl;
        return true;
    }
    else if (!m_variables.count("inputFile"))
    {
        cout << "Error: You must specify an input file." << endl;
        cout << endl;
        cout << m_descr << endl;
        return true;
    }
    return false;
}

Options::~Options()
{
    // TODO Auto-generated destructor stub
}

} // namespace meshreduce
