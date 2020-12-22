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

/**
 * LVRScanCamBridge.cpp
 *
 *  @date Dec 10, 2020
 *  @author Arthur Schreiber
 */
#include "LVRScanCamBridge.hpp"

#include "lvr2/geometry/Matrix4.hpp"

#include <vtkTransform.h>
#include <vtkActor.h>
#include <vtkProperty.h>

namespace lvr2
{

class LVRMeshBufferBridge;

LVRScanCamBridge::LVRScanCamBridge(ScanCameraPtr camera)
{
    cam = camera;
}

LVRScanCamBridge::LVRScanCamBridge(const LVRScanCamBridge& b)
{

}


void LVRScanCamBridge::addActors(vtkSmartPointer<vtkRenderer> renderer)
{
    //TODO IMPLEMENT ME
    std::cout << "ScanCam addActors()" << std::endl;
}

void LVRScanCamBridge::removeActors(vtkSmartPointer<vtkRenderer> renderer)
{
    //TODO IMPLEMENT ME
    std::cout << "ScanCam removeActors()" << std::endl;
}

void LVRScanCamBridge::setVisibility(bool visible)
{
    //TODO IMPLEMENT ME
    std::cout << "ScanCam setVisibility()" << std::endl;
}


LVRScanCamBridge::~LVRScanCamBridge()
{
    // TODO Auto-generated destructor stub
}

} /* namespace lvr2 */
