/* Copyright (C) 2011 Uni Osnabrück
 * This file is part of the LAS VEGAS Reconstruction Toolkit,
 *
 * LAS VEGAS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LAS VEGAS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/**
 * MainWindow.cpp
 *
 *  @date Jan 31, 2014
 *  @author Thomas Wiemann
 */

#include <QFileInfo>
#include <QAbstractItemView>
#include <QtGui>

#include "LVRMainWindow.hpp"

#include <lvr/io/ModelFactory.hpp>
#include <lvr/io/DataStruct.hpp>

#include <lvr/registration/ICPPointAlign.hpp>

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPointPicker.h>
#include <vtkCamera.h>

namespace lvr
{

LVRMainWindow::LVRMainWindow()
{
    setupUi(this);
    setupQVTK();

    // Init members
    m_correspondanceDialog = new LVRCorrespondanceDialog(treeWidget);
    m_incompatibilityBox = new QMessageBox();
    m_aboutDialog = new QDialog();
    Ui::AboutDialog aboutDialog;
    aboutDialog.setupUi(m_aboutDialog);

    m_tooltipDialog = new QDialog();
    Ui::TooltipDialog tooltipDialog;
    tooltipDialog.setupUi(m_tooltipDialog);

    m_spectralDialog = nullptr;
    m_spectralColorGradientDialog = nullptr;
    m_pointInfoDialog = nullptr;
    m_histogram=nullptr;

    // Setup specific properties
    QHeaderView* v = this->treeWidget->header();
    v->resizeSection(0, 175);

    treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_treeWidgetHelper = new LVRTreeWidgetHelper(treeWidget);

    m_treeParentItemContextMenu = new QMenu;
    m_treeChildItemContextMenu = new QMenu;
    m_actionRenameModelItem = new QAction("Rename item", this);
    m_actionDeleteModelItem = new QAction("Delete item", this);
    m_actionExportModelTransformed = new QAction("Export item with transformation", this);
    m_actionShowColorDialog = new QAction("Select base color...", this);

    m_treeParentItemContextMenu->addAction(m_actionRenameModelItem);
    m_treeParentItemContextMenu->addAction(m_actionDeleteModelItem);
    m_treeChildItemContextMenu->addAction(m_actionExportModelTransformed);
    m_treeChildItemContextMenu->addAction(m_actionShowColorDialog);
    m_treeChildItemContextMenu->addAction(m_actionDeleteModelItem);

    this->dockWidgetSpectralSliderSettings->close();
    this->dockWidgetSpectralColorGradientSettings->close();
    this->dockWidgetPointPreview->close();

    // Toolbar item "File"
    m_actionOpen = this->actionOpen;
    m_actionExport = this->actionExport;
    m_actionQuit = this->actionQuit;
    // Toolbar item "Views"
    m_actionReset_Camera = this->actionReset_Camera;
    m_actionStore_Current_View = this->actionStore_Current_View;
    m_actionRecall_Stored_View = this->actionRecall_Stored_View;
    m_actionCameraPathTool = this->actionCameraPathTool;
    // Toolbar item "Reconstruction"
    m_actionEstimate_Normals = this->actionEstimate_Normals; // TODO: fix normal estimation
    m_actionMarching_Cubes = this->actionMarching_Cubes;
    m_actionPlanar_Marching_Cubes = this->actionPlanar_Marching_Cubes;
    m_actionExtended_Marching_Cubes = this->actionExtended_Marching_Cubes;
    m_actionCompute_Textures = this->actionCompute_Textures; // TODO: Compute textures
    m_actionMatch_Textures_from_Package = this->actionMatch_Textures_from_Package; // TODO: Match textures from package
    m_actionExtract_and_Rematch_Patterns = this->actionExtract_and_Rematch_Patterns; // TODO: Extract and rematch patterns
    // Toolbar item "Mesh Optimization"
    m_actionPlanar_Optimization = this->actionPlanar_Optimization;
    m_actionRemove_Artifacts = this->actionRemove_Artifacts;
    // Toolbar item "Filtering"
    m_actionRemove_Outliers = this->actionRemove_Outliers;
    m_actionMLS_Projection = this->actionMLS_Projection;
    // Toolbar item "Registration"
    m_actionICP_Using_Manual_Correspondance = this->actionICP_Using_Manual_Correspondance;
    m_actionICP_Using_Pose_Estimations = this->actionICP_Using_Pose_Estimations; // TODO: implement ICP registration
    m_actionGlobal_Relaxation = this->actionGlobal_Relaxation; // TODO: implement global relaxation
    // Toolbar item "Classification"
    m_actionSimple_Plane_Classification = this->actionSimple_Plane_Classification;
    m_actionFurniture_Recognition = this->actionFurniture_Recognition;
    // Toolbar item "About"
    // TODO: Replace "About"-QMenu with "About"-QAction
    m_menuAbout = this->menuAbout;
    // QToolbar below toolbar
    m_actionShow_Points = this->actionShow_Points;
    m_actionShow_Normals = this->actionShow_Normals;
    m_actionShow_Mesh = this->actionShow_Mesh;
    m_actionShow_Wireframe = this->actionShow_Wireframe;
    m_actionShowBackgroundSettings = this->actionShowBackgroundSettings;
    m_actionShowSpectralSlider = this->actionShow_SpectralSlider;
    m_actionShowSpectralColorGradient = this->actionShow_SpectralColorGradient;
    m_actionShowSpectralPointPreview = this->actionShow_SpectralPointPreview;
    m_actionShowSpectralHistogram = this->actionShow_SpectralHistogram;

    // Slider below tree widget
    m_horizontalSliderPointSize = this->horizontalSliderPointSize;
    m_horizontalSliderTransparency = this->horizontalSliderTransparency;
    // Combo boxes
    m_comboBoxGradient = this->comboBoxGradient; // TODO: implement gradients
    m_comboBoxShading = this->comboBoxShading; // TODO: fix shading
    // Buttons below combo boxes
    m_buttonCameraPathTool = this->buttonCameraPathTool;
    m_buttonCreateMesh = this->buttonCreateMesh;
    m_buttonExportData = this->buttonExportData;
    m_buttonTransformModel = this->buttonTransformModel;

    m_pickingInteractor = new LVRPickingInteractor(m_renderer);
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle( m_pickingInteractor );
    vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);


   // Widget to display the coordinate system
     m_axes = vtkSmartPointer<vtkAxesActor>::New();

     m_axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
     m_axesWidget->SetOutlineColor( 0.9300, 0.5700, 0.1300 );
     m_axesWidget->SetOrientationMarker( m_axes );
     m_axesWidget->SetInteractor( m_renderer->GetRenderWindow()->GetInteractor() );
     m_axesWidget->SetDefaultRenderer(m_renderer);
     m_axesWidget->SetViewport( 0.0, 0.0, 0.3, 0.3 );
     m_axesWidget->SetEnabled( 1 );
     m_axesWidget->InteractiveOff();

    connectSignalsAndSlots();
}

LVRMainWindow::~LVRMainWindow()
{
    if(m_correspondanceDialog)
    {
        delete m_correspondanceDialog;
    }
    if (m_spectralDialog)
    {
        delete m_spectralDialog;
    }
    if (m_spectralColorGradientDialog)
    {
        delete m_spectralColorGradientDialog;
    }
    delete m_incompatibilityBox;
}

void LVRMainWindow::connectSignalsAndSlots()
{
    QObject::connect(m_actionOpen, SIGNAL(triggered()), this, SLOT(loadModel()));
    QObject::connect(m_actionExport, SIGNAL(triggered()), this, SLOT(exportSelectedModel()));
    QObject::connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeContextMenu(const QPoint&)));
    QObject::connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(restoreSliders(QTreeWidgetItem*, int)));
    QObject::connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(setModelVisibility(QTreeWidgetItem*, int)));

    QObject::connect(m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QObject::connect(m_actionShowColorDialog, SIGNAL(triggered()), this, SLOT(showColorDialog()));
    QObject::connect(m_actionRenameModelItem, SIGNAL(triggered()), this, SLOT(renameModelItem()));
    QObject::connect(m_actionDeleteModelItem, SIGNAL(triggered()), this, SLOT(deleteModelItem()));
    QObject::connect(m_actionExportModelTransformed, SIGNAL(triggered()), this, SLOT(exportSelectedModel()));

    QObject::connect(m_actionReset_Camera, SIGNAL(triggered()), this, SLOT(updateView()));
    QObject::connect(m_actionStore_Current_View, SIGNAL(triggered()), this, SLOT(saveCamera()));
    QObject::connect(m_actionRecall_Stored_View, SIGNAL(triggered()), this, SLOT(loadCamera()));
    QObject::connect(m_actionCameraPathTool, SIGNAL(triggered()), this, SLOT(openCameraPathTool()));

    QObject::connect(m_actionEstimate_Normals, SIGNAL(triggered()), this, SLOT(estimateNormals()));
    QObject::connect(m_actionMarching_Cubes, SIGNAL(triggered()), this, SLOT(reconstructUsingMarchingCubes()));
    QObject::connect(m_actionPlanar_Marching_Cubes, SIGNAL(triggered()), this, SLOT(reconstructUsingPlanarMarchingCubes()));
    QObject::connect(m_actionExtended_Marching_Cubes, SIGNAL(triggered()), this, SLOT(reconstructUsingExtendedMarchingCubes()));

    QObject::connect(m_actionPlanar_Optimization, SIGNAL(triggered()), this, SLOT(optimizePlanes()));
    QObject::connect(m_actionRemove_Artifacts, SIGNAL(triggered()), this, SLOT(removeArtifacts()));

    QObject::connect(m_actionRemove_Outliers, SIGNAL(triggered()), this, SLOT(removeOutliers()));
    QObject::connect(m_actionMLS_Projection, SIGNAL(triggered()), this, SLOT(applyMLSProjection()));

    QObject::connect(m_actionICP_Using_Manual_Correspondance, SIGNAL(triggered()), this, SLOT(manualICP()));

    QObject::connect(m_menuAbout, SIGNAL(triggered(QAction*)), this, SLOT(showAboutDialog(QAction*)));

    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(accepted()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(accepted()), this, SLOT(alignPointClouds()));
    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(rejected()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog, SIGNAL(addArrow(LVRVtkArrow*)), this, SLOT(addArrow(LVRVtkArrow*)));
    QObject::connect(m_correspondanceDialog, SIGNAL(removeArrow(LVRVtkArrow*)), this, SLOT(removeArrow(LVRVtkArrow*)));
    QObject::connect(m_correspondanceDialog, SIGNAL(disableCorrespondenceSearch()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog, SIGNAL(enableCorrespondenceSearch()), m_pickingInteractor, SLOT(correspondenceSearchOn()));

    QObject::connect(m_actionShow_Points, SIGNAL(toggled(bool)), this, SLOT(togglePoints(bool)));
    QObject::connect(m_actionShow_Normals, SIGNAL(toggled(bool)), this, SLOT(toggleNormals(bool)));
    QObject::connect(m_actionShow_Mesh, SIGNAL(toggled(bool)), this, SLOT(toggleMeshes(bool)));
    QObject::connect(m_actionShow_Wireframe, SIGNAL(toggled(bool)), this, SLOT(toggleWireframe(bool)));
    QObject::connect(m_actionShowBackgroundSettings, SIGNAL(triggered()), this, SLOT(showBackgroundDialog()));
    QObject::connect(m_actionShowSpectralSlider, SIGNAL(triggered()), this, SLOT(showSpectralSliderDialog()));
    QObject::connect(m_actionShowSpectralColorGradient, SIGNAL(triggered()), this, SLOT(showSpectralColorGradientDialog()));
    QObject::connect(m_actionShowSpectralPointPreview, SIGNAL(triggered()), this, SLOT(showSpectralPointPreviewDialog()));
    QObject::connect(m_actionShowSpectralHistogram, SIGNAL(triggered()), this, SLOT(showHistogram()));

    QObject::connect(m_horizontalSliderPointSize, SIGNAL(valueChanged(int)), this, SLOT(changePointSize(int)));
    QObject::connect(m_horizontalSliderTransparency, SIGNAL(valueChanged(int)), this, SLOT(changeTransparency(int)));

    QObject::connect(m_comboBoxShading, SIGNAL(currentIndexChanged(int)), this, SLOT(changeShading(int)));

    QObject::connect(m_buttonCameraPathTool, SIGNAL(pressed()), this, SLOT(openCameraPathTool()));
    QObject::connect(m_buttonCreateMesh, SIGNAL(pressed()), this, SLOT(reconstructUsingMarchingCubes()));
    QObject::connect(m_buttonExportData, SIGNAL(pressed()), this, SLOT(exportSelectedModel()));
    QObject::connect(m_buttonTransformModel, SIGNAL(pressed()), this, SLOT(showTransformationDialog()));

    QObject::connect(m_pickingInteractor, SIGNAL(firstPointPicked(double*)),m_correspondanceDialog, SLOT(firstPointPicked(double*)));
    QObject::connect(m_pickingInteractor, SIGNAL(secondPointPicked(double*)),m_correspondanceDialog, SLOT(secondPointPicked(double*)));

    QObject::connect(m_pickingInteractor, SIGNAL(pointSelected(vtkActor*, int)), this, SLOT(showPointInfoDialog(vtkActor*, int)));

    QObject::connect(this, SIGNAL(correspondenceDialogOpened()), m_pickingInteractor, SLOT(correspondenceSearchOn()));
}

void LVRMainWindow::showBackgroundDialog()
{
    LVRBackgroundDialog dialog(qvtkWidget->GetRenderWindow());
    if(dialog.exec() == QDialog::Accepted)
    {
        if(dialog.renderGradient())
        {
            float r1, r2, g1, g2, b1, b2;
            dialog.getColor1(r1, g1, b1);
            dialog.getColor2(r2, g2, b2);
            m_renderer->GradientBackgroundOn();
            m_renderer->SetBackground(r1, g1, b1);
            m_renderer->SetBackground2(r2, g2, b2);
        }
        else
        {
            float r, g, b;
            dialog.getColor1(r, g, b);
            m_renderer->GradientBackgroundOff();
            m_renderer->SetBackground(r, g, b);
        }
        this->qvtkWidget->GetRenderWindow()->Render();
    }
}

void LVRMainWindow::setupQVTK()
{
    // Grab relevant entities from the qvtk widget
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = this->qvtkWidget->GetRenderWindow();

    m_renderWindowInteractor = this->qvtkWidget->GetInteractor();
    m_renderWindowInteractor->Initialize();


    // Camera that saves a position that can be loaded
    m_camera = vtkSmartPointer<vtkCamera>::New();

    // Custom interactor to handle picking actions
    m_pickingInteractor = new LVRPickingInteractor(m_renderer);
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle( m_pickingInteractor );

    vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);

    // Camera and camera interpolator to be used for camera paths
    m_pathCamera = vtkSmartPointer<vtkCameraRepresentation>::New();
    vtkSmartPointer<vtkCameraInterpolator> cameraInterpolator = vtkSmartPointer<vtkCameraInterpolator>::New();
    cameraInterpolator->SetInterpolationTypeToSpline();
    m_pathCamera->SetInterpolator(cameraInterpolator);
    m_pathCamera->SetCamera(m_renderer->GetActiveCamera());

    // Finalize QVTK setup by adding the renderer to the window
    renderWindow->AddRenderer(m_renderer);
}

void LVRMainWindow::updateView()
{
    m_renderer->ResetCamera();
    m_renderer->ResetCameraClippingRange();
    this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::refreshView()
{
    this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::saveCamera()
{
    m_camera->DeepCopy(m_renderer->GetActiveCamera());
}

void LVRMainWindow::loadCamera()
{
    m_renderer->GetActiveCamera()->DeepCopy(m_camera);
    refreshView();
}

void LVRMainWindow::openCameraPathTool()
{
    new LVRAnimationDialog(m_renderWindowInteractor, m_pathCamera, treeWidget);
}

void LVRMainWindow::addArrow(LVRVtkArrow* a)
{
    if(a)
    {
        m_renderer->AddActor(a->getArrowActor());
        m_renderer->AddActor(a->getStartActor());
        m_renderer->AddActor(a->getEndActor());
    }
    this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::removeArrow(LVRVtkArrow* a)
{
    if(a)
    {
        m_renderer->RemoveActor(a->getArrowActor());
        m_renderer->RemoveActor(a->getStartActor());
        m_renderer->RemoveActor(a->getEndActor());
    }
    this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::restoreSliders(QTreeWidgetItem* treeWidgetItem, int column)
{
    if(treeWidgetItem->type() == LVRModelItemType)
    {
        QTreeWidgetItemIterator it(treeWidgetItem);

        while(*it)
        {
            QTreeWidgetItem* child_item = *it;

            if(child_item->type() == LVRPointCloudItemType && child_item->parent()->isSelected())
            {
                LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(child_item);
                m_horizontalSliderPointSize->setEnabled(true);
                m_horizontalSliderPointSize->setValue(model_item->getPointSize());
                int transparency = ((float)1 - model_item->getOpacity()) * 100;
                m_horizontalSliderTransparency->setEnabled(true);
                m_horizontalSliderTransparency->setValue(transparency);
            }
            else if(child_item->type() == LVRMeshItemType && child_item->parent()->isSelected())
            {
                LVRMeshItem* model_item = static_cast<LVRMeshItem*>(child_item);
                m_horizontalSliderPointSize->setEnabled(false);
                m_horizontalSliderPointSize->setValue(1);
                int transparency = ((float)1 - model_item->getOpacity()) * 100;
                m_horizontalSliderTransparency->setEnabled(true);
                m_horizontalSliderTransparency->setValue(transparency);
            }

            ++it;
        }
    }
    else if(treeWidgetItem->type() == LVRPointCloudItemType)
    {
        LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(treeWidgetItem);
        m_horizontalSliderPointSize->setEnabled(true);
        m_horizontalSliderPointSize->setValue(model_item->getPointSize());
        int transparency = ((float)1 - model_item->getOpacity()) * 100;
        m_horizontalSliderTransparency->setEnabled(true);
        m_horizontalSliderTransparency->setValue(transparency);
    }
    else if(treeWidgetItem->type() == LVRMeshItemType)
    {
        LVRMeshItem* model_item = static_cast<LVRMeshItem*>(treeWidgetItem);
        m_horizontalSliderPointSize->setEnabled(false);
        m_horizontalSliderPointSize->setValue(1);
        int transparency = ((float)1 - model_item->getOpacity()) * 100;
        m_horizontalSliderTransparency->setEnabled(true);
        m_horizontalSliderTransparency->setValue(transparency);
    }
    else
    {
        m_horizontalSliderPointSize->setEnabled(false);
        m_horizontalSliderPointSize->setValue(1);
        m_horizontalSliderTransparency->setEnabled(false);
        m_horizontalSliderTransparency->setValue(0);
    }
}

void LVRMainWindow::exportSelectedModel()
{
    // Get selected point cloud
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        if(item->type() == LVRPointCloudItemType)
        {
            if(item->parent() && item->parent()->type() == LVRModelItemType)
            {
                QString qFileName = QFileDialog::getSaveFileName(this, tr("Export Point Cloud As..."), "", tr("Point cloud Files(*.ply *.3d)"));

                LVRModelItem* model_item = static_cast<LVRModelItem*>(item->parent());
                LVRPointCloudItem* pc_item = static_cast<LVRPointCloudItem*>(item);
                PointBufferPtr points = pc_item->getPointBuffer();

                // Get transformation matrix
                Pose p = model_item->getPose();
                Matrix4f mat(Vertexf(p.x, p.y, p.z), Vertexf(p.r, p.t, p.p));

                // Allocate target buffer and insert transformed points
                size_t n;
                floatArr transformedPoints(new float[3 * points->getNumPoints()]);
                floatArr pointArray = points->getPointArray(n);
                for(size_t i = 0; i < points->getNumPoints(); i++)
                {
                    Vertexf v(pointArray[3 * i], pointArray[3 * i + 1], pointArray[3 * i + 2]);
                    Vertexf vt = mat * v;

                    transformedPoints[3 * i    ] = vt[0];
                    transformedPoints[3 * i + 1] = vt[1];
                    transformedPoints[3 * i + 2] = vt[2];
                }

                // Save transformed points
                PointBufferPtr trans(new PointBuffer);
                trans->setPointArray(transformedPoints, n);
                ModelPtr model(new Model(trans));
                ModelFactory::saveModel(model, qFileName.toStdString());
            }
        }
    }
}

void LVRMainWindow::alignPointClouds()
{
    Matrix4f mat = m_correspondanceDialog->getTransformation();
    QString name = m_correspondanceDialog->getDataName();
    QString modelName = m_correspondanceDialog->getModelName();

    PointBufferPtr modelBuffer = m_treeWidgetHelper->getPointBuffer(modelName);
    PointBufferPtr dataBuffer  = m_treeWidgetHelper->getPointBuffer(name);

    float pose[6];
    LVRModelItem* item = m_treeWidgetHelper->getModelItem(name);

    if(item)
    {
        mat.toPostionAngle(pose);

        // Pose ist in radians, so we need to convert p to degrees
        // to achieve consistency
        Pose p;
        p.x = pose[0];
        p.y = pose[1];
        p.z = pose[2];
        p.r = pose[3]  * 57.295779513;
        p.t = pose[4]  * 57.295779513;
        p.p = pose[5]  * 57.295779513;
        item->setPose(p);
    }

    updateView();
    // Refine pose via ICP
    if(m_correspondanceDialog->doICP() && modelBuffer && dataBuffer)
    {
        ICPPointAlign icp(modelBuffer, dataBuffer, mat);
        icp.setEpsilon(m_correspondanceDialog->getEpsilon());
        icp.setMaxIterations(m_correspondanceDialog->getMaxIterations());
        icp.setMaxMatchDistance(m_correspondanceDialog->getMaxDistance());
        Matrix4f refinedTransform = icp.match();

        cout << "Initial: " << mat << endl;

        // Apply correction to initial estimation
        //refinedTransform = mat * refinedTransform;
        refinedTransform.toPostionAngle(pose);

        cout << "Refined: " << refinedTransform << endl;

        Pose p;
        p.x = pose[0];
        p.y = pose[1];
        p.z = pose[2];
        p.r = pose[3]  * 57.295779513;
        p.t = pose[4]  * 57.295779513;
        p.p = pose[5]  * 57.295779513;
        item->setPose(p);
    }
    m_correspondanceDialog->clearAllItems();
    updateView();
}

void LVRMainWindow::showTreeContextMenu(const QPoint& p)
{
    // Only display context menu for point clounds and meshes
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        if(item->type() == LVRModelItemType)
        {
            QPoint globalPos = treeWidget->mapToGlobal(p);
            m_treeParentItemContextMenu->exec(globalPos);
        }
        if(item->type() == LVRPointCloudItemType || item->type() == LVRMeshItemType)
        {
            QPoint globalPos = treeWidget->mapToGlobal(p);
            m_treeChildItemContextMenu->exec(globalPos);
        }
    }
}

void LVRMainWindow::renameModelItem()
{
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        LVRModelItem* model_item = getModelItem(item);
        if(model_item != NULL) new LVRRenameDialog(model_item, treeWidget);
    }
}

void LVRMainWindow::loadModel()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Open Model"), "", tr("Model Files (*.ply *.obj *.pts *.3d *.txt *.h5)"));

    if(filenames.size() > 0)
    {
        QStringList::Iterator it = filenames.begin();
        while(it != filenames.end())
        {
            // Load model and generate vtk representation
            ModelPtr model = ModelFactory::readModel((*it).toStdString());
            ModelBridgePtr bridge(new LVRModelBridge(model));
            bridge->addActors(m_renderer);

            // Add item for this model to tree widget
            QFileInfo info((*it));
            QString base = info.fileName();
            LVRModelItem* item = new LVRModelItem(bridge, base);
            this->treeWidget->addTopLevelItem(item);
            item->setExpanded(true);
            for(QTreeWidgetItem* selected : treeWidget->selectedItems())
            {
                selected->setSelected(false);
            }
            item->setSelected(true);
            ++it;
        }

        assertToggles();
        updateView();
    }
}

void LVRMainWindow::deleteModelItem()
{
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();

        if(item->type() == LVRModelItemType)
        {
            QTreeWidgetItemIterator it(item);

            while(*it)
            {
                QTreeWidgetItem* child_item = *it;
                if(child_item->type() == LVRPointCloudItemType && child_item->parent() == item)
                {
                    LVRPointCloudItem* pc_item = getPointCloudItem(item);
                    if(pc_item != NULL) m_renderer->RemoveActor(pc_item->getActor());
                }
                else if(child_item->type() == LVRMeshItemType && child_item->parent() == item)
                {
                    LVRMeshItem* mesh_item = getMeshItem(item);
                    if(mesh_item != NULL)
                    {
                        m_renderer->RemoveActor(mesh_item->getWireframeActor());
                        m_renderer->RemoveActor(mesh_item->getActor());
                    }
                }

                ++it;
            }
        }
        else
        {
            // Remove model from view
            LVRPointCloudItem* pc_item = getPointCloudItem(item);
            if(pc_item != NULL) m_renderer->RemoveActor(pc_item->getActor());

            LVRMeshItem* mesh_item = getMeshItem(item);
            if(mesh_item != NULL) m_renderer->RemoveActor(mesh_item->getActor());
        }

        // Remove list item (safe according to http://stackoverflow.com/a/9399167)
        delete item;

        refreshView();
    }
}

LVRModelItem* LVRMainWindow::getModelItem(QTreeWidgetItem* item)
{
    if(item->type() == LVRModelItemType) return static_cast<LVRModelItem*>(item);
    if(item->parent()->type() == LVRModelItemType) return static_cast<LVRModelItem*>(item->parent());
    return NULL;
}

LVRPointCloudItem* LVRMainWindow::getPointCloudItem(QTreeWidgetItem* item)
{
    if(item->type() == LVRPointCloudItemType) return static_cast<LVRPointCloudItem*>(item);
    if(item->type() == LVRModelItemType)
    {
        QTreeWidgetItemIterator it(item);

        while(*it)
        {
            QTreeWidgetItem* child_item = *it;
            if(child_item->type() == LVRPointCloudItemType && child_item->parent() == item)
            {
                return static_cast<LVRPointCloudItem*>(child_item);
            }
            ++it;
        }
    }
    return NULL;
}

LVRMeshItem* LVRMainWindow::getMeshItem(QTreeWidgetItem* item)
{
    if(item->type() == LVRMeshItemType) return static_cast<LVRMeshItem*>(item);
    if(item->type() == LVRModelItemType)
    {
        QTreeWidgetItemIterator it(item);

        while(*it)
        {
            QTreeWidgetItem* child_item = *it;
            if(child_item->type() == LVRMeshItemType && child_item->parent() == item)
            {
                return static_cast<LVRMeshItem*>(child_item);
            }
            ++it;
        }
    }
    return NULL;
}

void LVRMainWindow::assertToggles()
{
    togglePoints(m_actionShow_Points->isChecked());
    toggleNormals(m_actionShow_Normals->isChecked());
    toggleMeshes(m_actionShow_Mesh->isChecked());
    toggleWireframe(m_actionShow_Wireframe->isChecked());
}

void LVRMainWindow::setModelVisibility(QTreeWidgetItem* treeWidgetItem, int column)
{
    if(treeWidgetItem->type() == LVRModelItemType)
    {
        QTreeWidgetItemIterator it(treeWidgetItem);

        while(*it)
        {
            QTreeWidgetItem* child_item = *it;
            if(child_item->type() == LVRPointCloudItemType)
            {
                LVRModelItem* model_item = static_cast<LVRModelItem*>(treeWidgetItem);
                model_item->setModelVisibility(column, m_actionShow_Points->isChecked());
            }
            if(child_item->type() == LVRMeshItemType)
            {
                LVRModelItem* model_item = static_cast<LVRModelItem*>(treeWidgetItem);
                model_item->setModelVisibility(column, m_actionShow_Mesh->isChecked());
            }
            ++it;
        }

        refreshView();
    }
}

void LVRMainWindow::changePointSize(int pointSize)
{
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();

        if(item->type() == LVRModelItemType)
        {
            QTreeWidgetItemIterator it(item);

            while(*it)
            {
                QTreeWidgetItem* child_item = *it;
                if(child_item->type() == LVRPointCloudItemType && child_item->parent()->isSelected())
                {
                    LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(child_item);
                    model_item->setPointSize(pointSize);
                }
                ++it;
            }
        }
        else if(item->type() == LVRPointCloudItemType)
        {
            LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(item);
            model_item->setPointSize(pointSize);
        }

        refreshView();
    }
}

void LVRMainWindow::changeTransparency(int transparencyValue)
{
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        float opacityValue = 1 - ((float)transparencyValue / (float)100);

        if(item->type() == LVRModelItemType)
        {
            QTreeWidgetItemIterator it(item);

            while(*it)
            {
                QTreeWidgetItem* child_item = *it;
                if(child_item->type() == LVRPointCloudItemType && child_item->parent()->isSelected())
                {
                    LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(child_item);
                    model_item->setOpacity(opacityValue);
                }
                else if(child_item->type() == LVRMeshItemType && child_item->parent()->isSelected())
                {
                    LVRMeshItem* model_item = static_cast<LVRMeshItem*>(child_item);
                    model_item->setOpacity(opacityValue);
                }
                ++it;
            }
        }
        else if(item->type() == LVRPointCloudItemType)
        {
            LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(item);
            model_item->setOpacity(opacityValue);
        }
        else if(item->type() == LVRMeshItemType)
        {
            LVRMeshItem* model_item = static_cast<LVRMeshItem*>(item);
            model_item->setOpacity(opacityValue);
        }

        refreshView();
    }
}

void LVRMainWindow::changeShading(int shader)
{
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();

        if(item->type() == LVRMeshItemType)
        {
            LVRMeshItem* model_item = static_cast<LVRMeshItem*>(item);
            model_item->setShading(shader);
            refreshView();
        }
    }
}

void LVRMainWindow::togglePoints(bool checkboxState)
{
    QTreeWidgetItemIterator it(treeWidget);

    while(*it)
    {
        QTreeWidgetItem* item = *it;
        if(item->type() == LVRPointCloudItemType)
        {
            LVRModelItem* model_item = static_cast<LVRModelItem*>(item->parent());
            if(model_item->isEnabled()) model_item->setVisibility(checkboxState);
        }
        ++it;
    }

    refreshView();
}

void LVRMainWindow::toggleNormals(bool checkboxState)
{
    QTreeWidgetItemIterator it(treeWidget);

    while(*it)
    {
        QTreeWidgetItem* item = *it;
        if(item->type() == LVRPointCloudItemType)
        {
            LVRModelItem* model_item = static_cast<LVRModelItem*>(item->parent());
            if(model_item->isEnabled()) return; // TODO: functions to show/hide normals
        }
        ++it;
    }

    refreshView();
}

void LVRMainWindow::toggleMeshes(bool checkboxState)
{
    QTreeWidgetItemIterator it(treeWidget);

    while(*it)
    {
        QTreeWidgetItem* item = *it;
        if(item->type() == LVRMeshItemType)
        {
            LVRModelItem* model_item = static_cast<LVRModelItem*>(item->parent());
            if(model_item->isEnabled()) model_item->setVisibility(checkboxState);
        }
        ++it;
    }

    refreshView();
}

void LVRMainWindow::toggleWireframe(bool checkboxState)
{
    if(m_actionShow_Mesh)
    {
        QTreeWidgetItemIterator it(treeWidget);

        while(*it)
        {
            QTreeWidgetItem* item = *it;
            if(item->type() == LVRMeshItemType)
            {
                LVRMeshItem* mesh_item = static_cast<LVRMeshItem*>(item);
                if(checkboxState)
                {
                    m_renderer->AddActor(mesh_item->getWireframeActor());
                }
                else
                {
                    m_renderer->RemoveActor(mesh_item->getWireframeActor());
                }
                refreshView();
            }
            ++it;
        }

        refreshView();
    }
}

void LVRMainWindow::parseCommandLine(int argc, char** argv)
{
    for(int i = 1; i < argc; i++)
    {
        // Load model and generate vtk representation
        ModelPtr model = ModelFactory::readModel(string(argv[i]));
        ModelBridgePtr bridge(new LVRModelBridge(model));
        bridge->addActors(m_renderer);

        // Add item for this model to tree widget
        QString s(argv[i]);
        QFileInfo info(s);
        QString base = info.fileName();
        LVRModelItem* item = new LVRModelItem(bridge, base);
        this->treeWidget->addTopLevelItem(item);
        item->setExpanded(true);
        for(QTreeWidgetItem* selected : treeWidget->selectedItems())
        {
            selected->setSelected(false);
        }
        item->setSelected(true);
    }
    updateView();
    assertToggles();

}

void LVRMainWindow::manualICP()
{
    m_correspondanceDialog->fillComboBoxes();
    m_correspondanceDialog->m_dialog->show();
    m_correspondanceDialog->m_dialog->raise();
    m_correspondanceDialog->m_dialog->activateWindow();
    Q_EMIT(correspondenceDialogOpened());
}

void LVRMainWindow::showColorDialog()
{
    QColor c = QColorDialog::getColor();
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        if(item->type() == LVRPointCloudItemType)
        {
            LVRPointCloudItem* pc_item = static_cast<LVRPointCloudItem*>(item);
            pc_item->setColor(c);
        }
        else if(item->type() == LVRMeshItemType)
        {
            LVRMeshItem* mesh_item = static_cast<LVRMeshItem*>(item);
            mesh_item->setColor(c);
        }
        else {
            return;
        }

        refreshView();
    }
}

void LVRMainWindow::showTransformationDialog()
{
    buildIncompatibilityBox(string("transformation"), POINTCLOUDS_AND_MESHES_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        if(item->type() == LVRModelItemType)
        {
            LVRModelItem* item = static_cast<LVRModelItem*>(items.first());
            LVRTransformationDialog* dialog = new LVRTransformationDialog(item, qvtkWidget->GetRenderWindow());
        }
        else if(item->type() == LVRPointCloudItemType || item->type() == LVRMeshItemType)
        {
            if(item->parent()->type() == LVRModelItemType)
            {
                LVRModelItem* l_item = static_cast<LVRModelItem*>(item->parent());
                LVRTransformationDialog* dialog = new LVRTransformationDialog(l_item, qvtkWidget->GetRenderWindow());
            }
            else
            {
                m_incompatibilityBox->exec();
            }
        }
        else
        {
            m_incompatibilityBox->exec();
        }
    }
}

void LVRMainWindow::estimateNormals()
{
    buildIncompatibilityBox(string("normal estimation"), POINTCLOUDS_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRPointCloudItem* pc_item = getPointCloudItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(pc_item != NULL)
        {
            LVREstimateNormalsDialog* dialog = new LVREstimateNormalsDialog(pc_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::reconstructUsingMarchingCubes()
{
    buildIncompatibilityBox(string("reconstruction"), POINTCLOUDS_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRPointCloudItem* pc_item = getPointCloudItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(pc_item != NULL)
        {
            LVRReconstructViaMarchingCubesDialog* dialog = new LVRReconstructViaMarchingCubesDialog("MC", pc_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::reconstructUsingPlanarMarchingCubes()
{
    buildIncompatibilityBox(string("reconstruction"), POINTCLOUDS_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRPointCloudItem* pc_item = getPointCloudItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(pc_item != NULL)
        {
            LVRReconstructViaMarchingCubesDialog* dialog = new LVRReconstructViaMarchingCubesDialog("PMC", pc_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::reconstructUsingExtendedMarchingCubes()
{
    buildIncompatibilityBox(string("reconstruction"), POINTCLOUDS_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRPointCloudItem* pc_item = getPointCloudItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(pc_item != NULL)
        {
            LVRReconstructViaExtendedMarchingCubesDialog* dialog = new LVRReconstructViaExtendedMarchingCubesDialog("SF", pc_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::optimizePlanes()
{
    buildIncompatibilityBox(string("planar optimization"), MESHES_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRMeshItem* mesh_item = getMeshItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(mesh_item != NULL)
        {
            LVRPlanarOptimizationDialog* dialog = new LVRPlanarOptimizationDialog(mesh_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::removeArtifacts()
{
    buildIncompatibilityBox(string("artifact removal"), MESHES_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRMeshItem* mesh_item = getMeshItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(mesh_item != NULL)
        {
            LVRRemoveArtifactsDialog* dialog = new LVRRemoveArtifactsDialog(mesh_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::applyMLSProjection()
{
    buildIncompatibilityBox(string("MLS projection"), POINTCLOUDS_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRPointCloudItem* pc_item = getPointCloudItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(pc_item != NULL)
        {
            LVRMLSProjectionDialog* dialog = new LVRMLSProjectionDialog(pc_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::removeOutliers()
{
    buildIncompatibilityBox(string("outlier removal"), POINTCLOUDS_AND_PARENT_ONLY);
    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        LVRPointCloudItem* pc_item = getPointCloudItem(items.first());
        LVRModelItem* parent_item = getModelItem(items.first());
        if(pc_item != NULL)
        {
            LVRRemoveOutliersDialog* dialog = new LVRRemoveOutliersDialog(pc_item, parent_item, treeWidget, qvtkWidget->GetRenderWindow());
            return;
        }
    }
    m_incompatibilityBox->exec();
}

void LVRMainWindow::buildIncompatibilityBox(string actionName, unsigned char allowedTypes)
{
    // Setup a message box for unsupported items
    string titleString = str(boost::format("Unsupported Item for %1%.") % actionName);
    QString title = QString::fromStdString(titleString);
    string bodyString = "Only %2% are applicable to %1%.";
    QString body;

    if(allowedTypes == MODELITEMS_ONLY)
        bodyString = str(boost::format(bodyString) % actionName % "whole models");
    else if(allowedTypes == POINTCLOUDS_ONLY)
        bodyString = str(boost::format(bodyString) % actionName % "point clouds");
    else if(allowedTypes == MESHES_ONLY)
        bodyString = str(boost::format(bodyString) % actionName % "meshes");
    else if(allowedTypes == POINTCLOUDS_AND_PARENT_ONLY)
        bodyString = str(boost::format(bodyString) % actionName % "point clouds and model items containing point clouds");
    else if(allowedTypes == MESHES_AND_PARENT_ONLY)
        bodyString = str(boost::format(bodyString) % actionName % "meshes and model items containing meshes");
    else if(allowedTypes == POINTCLOUDS_AND_MESHES_AND_PARENT_ONLY)
        bodyString = str(boost::format(bodyString) % actionName % "point clouds, meshes and whole models");

    body = QString::fromStdString(bodyString);

    m_incompatibilityBox->setText(title);
    m_incompatibilityBox->setInformativeText(body);
    m_incompatibilityBox->setStandardButtons(QMessageBox::Ok);
}

void LVRMainWindow::showAboutDialog(QAction*)
{
    m_aboutDialog->show();
}

void LVRMainWindow::showTooltipDialog()
{
    m_tooltipDialog->show();
    m_tooltipDialog->raise();
}

void LVRMainWindow::showSpectralSliderDialog()
{
    this->dockWidgetSpectralSliderSettings->show();
}

void LVRMainWindow::showSpectralColorGradientDialog()
{
    this->dockWidgetSpectralColorGradientSettings->show();
}

void LVRMainWindow::showSpectralPointPreviewDialog()
{
    this->dockWidgetPointPreview->show();
}

void LVRMainWindow::showHistogram()
{
  
     if (m_spectralColorGradientDialog)
    {
        m_spectralColorGradientDialog->exitDialog();    
    }
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        LVRModelItem* model_item = getModelItem(item);
        if(model_item != NULL)
        {
            PointBufferBridgePtr points = model_item->getModelBridge()->getPointBridge();
            if(points->getNumPoints() && points->getPointBuffer()->hasPointSpectralChannels())
            {
                m_histogram=new LVRHistogram();
                m_histogram->setPointBuffer(points->getPointBuffer());
                m_histogram->sethistogram();   
            }
            else
            {
                showTooltipDialog();
            }
        }
    }
    else
    {
       showTooltipDialog(); 
    }
}

void LVRMainWindow::showPointInfoDialog(vtkActor* actor, int point)
{
    if (actor == nullptr || point < 0)
    {
        return;
    }
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    LVRPointBufferBridge* pointBridge = nullptr;
    for(int i = 0; i < treeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = treeWidget->topLevelItem(i);
        LVRPointBufferBridge* bridge = getModelItem(item)->getModelBridge()->getPointBridge().get();
        if (bridge->getPointCloudActor() == actor)
        {
            pointBridge = bridge;
            break;
        }
    }
    if (pointBridge == nullptr)
    {
        return;
    }
    if (!m_pointInfoDialog)
    {
        m_pointInfoDialog = new LVRPointInfo(treeWidget);
    }
    m_pointInfoDialog->setPointBuffer(pointBridge->getPointBuffer());
    m_pointInfoDialog->setPoint(point);
}


} /* namespace lvr */
