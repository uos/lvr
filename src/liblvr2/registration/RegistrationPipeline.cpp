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

#include "lvr2/registration/RegistrationPipeline.hpp"
#include "lvr2/registration/SLAMAlign.hpp"
#include "lvr2/registration/SLAMScanWrapper.hpp"

#include "vector"

using namespace lvr2;

float getDifference(Transformd a, Transformd b)
{

    float sum = 0;
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            sum += std::abs(a(i,j) - b(i,j));
        }
    }
    return sum;
}  

RegistrationPipeline::RegistrationPipeline(const SLAMOptions* options, ScanProjectEditMarkPtr scans)
{
    m_options = options;
    m_scans = scans;
}


void RegistrationPipeline::doRegistration()
{
    SLAMAlign align(*m_options);
    
    std::vector<Transformd> oldTreansformd(m_scans->positions.size());
    for (size_t i = 0; i < m_scans->positions.size(); i++)
    {
        ScanOptional opt = m_scans->positions.at(i)->scan;
        if (opt)
        {
            Transformd g;
            for (size_t i_1 = 0; i < 4; i++)
            {
                for (size_t i_2 = 0; i_2 < 4; i_2++)
                {
                    g(i_1,i_2) = opt->m_registration(i_1, i_2);
                }
            }
            oldTreansformd.at(i) = g;


            ScanPtr scptr = std::make_shared<Scan>(*opt);
            align.addScan(scptr);
        }
    }

    align.finish();

    oldTreansformd.at(3)(1,2) = 1000;
    for (int i = 0; i < m_scans->positions.size(); i++)
    {
        // check if the new pos different to old pos
        // ToDo: make num to option
        cout << "Diff: " << getDifference(m_scans->positions.at(i)->scan->m_registration, oldTreansformd.at(i)) << endl;
        if ((!m_scans->changed.at(i)) && (getDifference(m_scans->positions.at(i)->scan->m_registration, oldTreansformd.at(i)) > 5.0))
        {
            m_scans->changed.at(i) = true;
        }
        ScanPositionPtr posPtr = m_scans->positions.at(i);
        posPtr->scan->m_registration = align.scan(i)->pose();
        cout << "Pose Scan Nummer " << i << endl << posPtr->scan->m_registration << endl;
    }

}