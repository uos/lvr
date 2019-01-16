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
 * LVRPickingInteractor.cpp
 *
 *  @date Feb 19, 2014
 *  @author Thomas Wiemann
 */
#include "LVRPickingInteractor.hpp"

#include <vtkObjectFactory.h>
#include <vtkTextProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAbstractPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkPointPicker.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>

namespace lvr2
{

LVRPickingInteractor::LVRPickingInteractor(vtkSmartPointer<vtkRenderer> renderer) :
        m_renderer(renderer), m_motionFactor(50), m_rotationFactor(20), m_interactorMode(TRACKBALL)
{
    m_pickMode = None;
    m_correspondenceMode = false;
    vtkSmartPointer<vtkTextProperty> p = vtkSmartPointer<vtkTextProperty>::New();
    p->SetColor(1.0, 1.0, 0.0);
    p->SetBold(1);
    p->SetShadow(0);

    m_textActor = vtkSmartPointer<vtkTextActor>::New();
    m_textActor->SetDisplayPosition(100, 10);
    m_textActor->SetTextProperty(p);
    m_textActor->SetInput("Pick a point...");
    m_textActor->VisibilityOff();
    m_renderer->AddActor(m_textActor);

}

void LVRPickingInteractor::setMotionFactor(double factor)
{
    m_motionFactor = factor;
}

void LVRPickingInteractor::setRotationFactor(double factor)
{
    m_rotationFactor = factor;
}

LVRPickingInteractor::~LVRPickingInteractor()
{
    // TODO Auto-generated destructor stub
}


void LVRPickingInteractor::Dolly()
{
    switch(m_interactorMode)
    {
        case TRACKBALL:
            dollyTrackball();
            break;
        default:
            dollyTrackball();
    }
}

void LVRPickingInteractor::Dolly(double speed)
{
    switch(m_interactorMode)
    {
        case TRACKBALL:
            dollyTrackball(speed);
            break;
        default:
            dollyTrackball();

    }
}

void LVRPickingInteractor::Pan()
{
    switch(m_interactorMode)
    {
        case TRACKBALL:
            panTrackball();
            break;
        default:
            panTrackball();

    }
}

void LVRPickingInteractor::Spin()
{
    switch(m_interactorMode)
    {
        case TRACKBALL:
            spinTrackball();
            break;
        default:
            spinTrackball();
    }
}

void LVRPickingInteractor::Rotate()
{
    switch(m_interactorMode)
    {
        case TRACKBALL:
            rotateTrackball();
            break;
        default:
            rotateTrackball();
    }
}

void LVRPickingInteractor::Zoom()
{
    switch(m_interactorMode)
    {
        case TRACKBALL:
            zoomTrackball();
            break;
        default:
            zoomTrackball();
    }
}

 void LVRPickingInteractor::OnLeftButtonDown()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onLeftButtonDownTrackball();
             break;
         default:
             onLeftButtonDownTrackball();
     }
 }

 void LVRPickingInteractor::OnLeftButtonUp()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onLeftButtonUpTrackball();
             break;
         default:
             onLeftButtonUpTrackball();
     }
 }

 void LVRPickingInteractor::OnMouseMove()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onMouseMoveTrackball();
             break;
         default:
             onMouseMoveTrackball();
     }
 }

 void LVRPickingInteractor::OnMiddleButtonUp()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onMiddleButtonUpTrackball();
             break;
         default:
             onMiddleButtonUpTrackball();
     }
 }

 void LVRPickingInteractor::OnMiddleButtonDown()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onMiddleButtonDownTrackball();
             break;
         default:
             onMiddleButtonDownTrackball();
     }
 }

 void LVRPickingInteractor::OnRightButtonUp()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onRightButtonUpTrackball();
             break;
         default:
             onRightButtonUpTrackball();
     }
 }

 void LVRPickingInteractor::OnRightButtonDown()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onRightButtonDownTrackball();
             break;
         default:
             onRightButtonDownTrackball();
     }
 }

 void LVRPickingInteractor::OnMouseWheelBackward()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onMouseWheelBackwardTrackball();
             break;
         default:
             onMouseWheelBackwardTrackball();
     }
 }

 void LVRPickingInteractor::OnMouseWheelForward()
 {
     switch(m_interactorMode)
     {
         case TRACKBALL:
             onMouseWheelForwardTrackball();
             break;
         default:
             onMouseWheelForwardTrackball();
     }
 }

void LVRPickingInteractor::dollyTrackball(double factor)
{
    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
    if (camera->GetParallelProjection())
    {
        camera->SetParallelScale(camera->GetParallelScale() / factor);
    }
    else
    {
        double position[3];
        double direction[3];

        camera->GetPosition(position);
        camera->GetDirectionOfProjection(direction);

        // Compute desired position
        camera->SetPosition(
                position[0] - factor * direction[0],
                position[1] - factor * direction[1],
                position[2] - factor * direction[2]);

        if (this->AutoAdjustCameraClippingRange)
        {
            this->CurrentRenderer->ResetCameraClippingRange();
        }
    }

    if (this->Interactor->GetLightFollowCamera())
    {
        this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

    this->Interactor->Render();
}

void LVRPickingInteractor::dollyTrackball()
{
    if (this->CurrentRenderer == nullptr)
    {
      return;
    }

    vtkRenderWindowInteractor *rwi = this->Interactor;
    double *center = this->CurrentRenderer->GetCenter();
    int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
    double dyf = 1 * dy / center[1];

    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
    if (camera->GetParallelProjection())
    {
        camera->SetParallelScale(camera->GetParallelScale() / dyf);
    }
    else
    {
        camera->Dolly(dyf);
        if (this->AutoAdjustCameraClippingRange)
        {
            this->CurrentRenderer->ResetCameraClippingRange();
        }
    }

    if (this->Interactor->GetLightFollowCamera())
    {
        this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

    this->Interactor->Render();
}

void LVRPickingInteractor::panTrackball()
{
    if (this->CurrentRenderer == nullptr)
     {
       return;
     }

     vtkRenderWindowInteractor *rwi = this->Interactor;

     double viewFocus[4], focalDepth, viewPoint[3];
     double newPickPoint[4], oldPickPoint[4], motionVector[3];

     // Calculate the focal depth since we'll be using it a lot

     vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
     camera->GetFocalPoint(viewFocus);
     this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2],
                                 viewFocus);
     focalDepth = viewFocus[2];

     this->ComputeDisplayToWorld(rwi->GetEventPosition()[0],
                                 rwi->GetEventPosition()[1],
                                 focalDepth,
                                 newPickPoint);

     // Has to recalc old mouse point since the viewport has moved,
     // so can't move it outside the loop

     this->ComputeDisplayToWorld(rwi->GetLastEventPosition()[0],
                                 rwi->GetLastEventPosition()[1],
                                 focalDepth,
                                 oldPickPoint);

     // Camera motion is reversed

     motionVector[0] = oldPickPoint[0] - newPickPoint[0];
     motionVector[1] = oldPickPoint[1] - newPickPoint[1];
     motionVector[2] = oldPickPoint[2] - newPickPoint[2];

     camera->GetFocalPoint(viewFocus);
     camera->GetPosition(viewPoint);
     camera->SetFocalPoint(motionVector[0] + viewFocus[0],
                           motionVector[1] + viewFocus[1],
                           motionVector[2] + viewFocus[2]);

     camera->SetPosition(motionVector[0] + viewPoint[0],
                         motionVector[1] + viewPoint[1],
                         motionVector[2] + viewPoint[2]);

     if (rwi->GetLightFollowCamera())
     {
       this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
     }

   rwi->Render();
}

void LVRPickingInteractor::spinTrackball()
{
    if ( this->CurrentRenderer == nullptr )
    {
        return;
    }

    vtkRenderWindowInteractor *rwi = this->Interactor;

    double *center = this->CurrentRenderer->GetCenter();

    double newAngle =
            vtkMath::DegreesFromRadians( atan2( rwi->GetEventPosition()[1] - center[1],
            rwi->GetEventPosition()[0] - center[0] ) );

    double oldAngle =
            vtkMath::DegreesFromRadians( atan2( rwi->GetLastEventPosition()[1] - center[1],
            rwi->GetLastEventPosition()[0] - center[0] ) );

    vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
    camera->Roll( newAngle - oldAngle );
    camera->OrthogonalizeViewUp();

    rwi->Render();
}

void LVRPickingInteractor::zoomTrackball()
{
    //vtkInteractorStyleTrackballCamera::Zoom();
}

void LVRPickingInteractor::rotateTrackball()
{
    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    vtkRenderWindowInteractor *rwi = this->Interactor;

    int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
    int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

    int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();

    double delta_elevation = -20.0 / size[1];
    double delta_azimuth = -20.0 / size[0];

    double rxf = dx * delta_azimuth * m_motionFactor;
    double ryf = dy * delta_elevation * m_motionFactor;

    vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
    camera->Azimuth(rxf);
    camera->Elevation(ryf);
    camera->OrthogonalizeViewUp();

    if (this->AutoAdjustCameraClippingRange)
    {
        this->CurrentRenderer->ResetCameraClippingRange();
    }

    if (rwi->GetLightFollowCamera())
    {
        this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

    rwi->Render();
}

void LVRPickingInteractor::correspondenceSearchOn()
{
    vtkRenderWindowInteractor *rwi = this->Interactor;
    m_correspondenceMode = true;
    m_textActor->SetInput("Pick first correspondence point...");
    m_textActor->VisibilityOn();
    rwi->Render();
    m_pickMode = PickFirst;
}

void LVRPickingInteractor::correspondenceSearchOff()
{
    vtkRenderWindowInteractor *rwi = this->Interactor;
    m_correspondenceMode = false;
    m_pickMode = None;
    m_textActor->VisibilityOff();
    rwi->Render();
}

void LVRPickingInteractor::onLeftButtonDownTrackball()
{
    vtkPointPicker* picker = (vtkPointPicker*)this->Interactor->GetPicker();

    if(m_pickMode == None)
    {
        this->m_numberOfClicks++;
        //std::cout << "m_numberOfClicks = " << this->m_numberOfClicks << std::endl;
        int pickPosition[2];
        this->GetInteractor()->GetEventPosition(pickPosition);

        int xdist = pickPosition[0] - this->m_previousPosition[0];
        int ydist = pickPosition[1] - this->m_previousPosition[1];

        this->m_previousPosition[0] = pickPosition[0];
        this->m_previousPosition[1] = pickPosition[1];

        int moveDistance = (int)sqrt((double)(xdist*xdist + ydist*ydist));

        // Reset numClicks - If mouse moved further than resetPixelDistance
        if(moveDistance > 5)
        { 
            this->m_numberOfClicks = 1;
        }

        if(this->m_numberOfClicks == 2)
        {
            this->m_numberOfClicks = 0;
            double* picked = new double[3];
            picker->Pick(pickPosition[0],
                                    pickPosition[1],
                                    0,  // always zero.
                                    this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
            vtkActor* actor = picker->GetActor();
            int point = picker->GetPointId();
            Q_EMIT(pointSelected(actor, point));
        }
    }
    else
    {
        int* pickPos = this->Interactor->GetEventPosition();
        double* picked = new double[3];
        picker->Pick(this->Interactor->GetEventPosition()[0],
                                this->Interactor->GetEventPosition()[1],
                                0,  // always zero.
                                this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
        picker->GetPickPosition(picked);

        if(m_pickMode == PickFirst)
        {
            Q_EMIT(firstPointPicked(picked));
        }
        else if(m_pickMode == PickSecond)
        {
            Q_EMIT(secondPointPicked(picked));
        }
    }

    handleLeftButtonDownTrackball();
}

void LVRPickingInteractor::onLeftButtonUpTrackball()
{
    switch (this->State)
    {
    case VTKIS_DOLLY:
        this->EndDolly();
        break;

    case VTKIS_PAN:
        this->EndPan();
        break;

    case VTKIS_SPIN:
        this->EndSpin();
        break;

    case VTKIS_ROTATE:
        this->EndRotate();
        break;
    }

    if ( this->Interactor )
    {
        this->ReleaseFocus();
    }
}

void LVRPickingInteractor::onMouseMoveTrackball()
{
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    switch (this->State)
    {
    case VTKIS_ROTATE:
        this->FindPokedRenderer(x, y);
        this->Rotate();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
        break;

    case VTKIS_PAN:
        this->FindPokedRenderer(x, y);
        this->Pan();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
        break;

    case VTKIS_DOLLY:
        this->FindPokedRenderer(x, y);
        this->Dolly();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
        break;

    case VTKIS_SPIN:
        this->FindPokedRenderer(x, y);
        this->Spin();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
        break;
    }
}

void LVRPickingInteractor::onMiddleButtonUpTrackball()
{
    switch (this->State)
    {
    case VTKIS_PAN:
        this->EndPan();
        if ( this->Interactor )
        {
            this->ReleaseFocus();
        }
        break;
    }
}

void LVRPickingInteractor::onMouseWheelForwardTrackball()
{
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
            this->Interactor->GetEventPosition()[1]);
    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    this->StartDolly();
    double factor = m_motionFactor * this->MouseWheelMotionFactor;
    this->Dolly(-pow(1.1, factor));
    this->EndDolly();
    this->ReleaseFocus();
}

void LVRPickingInteractor::onMouseWheelBackwardTrackball()
{
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
            this->Interactor->GetEventPosition()[1]);
    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    this->StartDolly();
    double factor = m_motionFactor * this->MouseWheelMotionFactor;
    this->Dolly(pow(1.1, factor));
    this->EndDolly();
    this->ReleaseFocus();
}

void LVRPickingInteractor::onMiddleButtonDownTrackball()
{
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
            this->Interactor->GetEventPosition()[1]);
    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    this->StartPan();
}

void LVRPickingInteractor::onRightButtonUpTrackball()
{
    switch (this->State)
    {
    case VTKIS_DOLLY:
        this->EndDolly();

        if ( this->Interactor )
        {
            this->ReleaseFocus();
        }
        break;
    }
}

void LVRPickingInteractor::onRightButtonDownTrackball()
{
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
            this->Interactor->GetEventPosition()[1]);
    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    this->StartDolly();
}


void LVRPickingInteractor::handleLeftButtonDownTrackball()
{
    // Code taken from vtkInteractorStyleTrackballCamera
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
            this->Interactor->GetEventPosition()[1]);
    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    GrabFocus(this->EventCallbackCommand, nullptr);
    if (this->Interactor->GetShiftKey())
    {
        if (this->Interactor->GetControlKey())
        {
            this->StartDolly();
        }
        else
        {
            this->StartPan();
        }
    }
    else
    {
        if (this->Interactor->GetControlKey())
        {
            this->StartSpin();
        }
        else
        {
            this->StartRotate();
        }
    }
}

void LVRPickingInteractor::OnKeyPress()
{

}

void LVRPickingInteractor::OnKeyDown()
{
    vtkRenderWindowInteractor *rwi = this->Interactor;
    std::string key = rwi->GetKeySym();

    if(key == "a" && m_correspondenceMode)
    {
        m_textActor->SetInput("Pick first correspondence point...");
        m_textActor->VisibilityOn();
        rwi->Render();
        m_pickMode = PickFirst;
    }

    if(key == "y" && m_correspondenceMode)
    {
        m_textActor->SetInput("Pick second correspondence point...");
        m_textActor->VisibilityOn();
        rwi->Render();
        m_pickMode = PickSecond;
    }

    if(key == "q")
    {
        m_textActor->VisibilityOff();
        m_pickMode = None;
        rwi->Render();
    }

}

void LVRPickingInteractor::OnKeyRelease()
{

}

} /* namespace lvr2 */
