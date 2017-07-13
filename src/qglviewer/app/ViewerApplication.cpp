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


 /*
 * ViewerApplication.cpp
 *
 *  Created on: 15.09.2010
 *      Author: Thomas Wiemann
 */

#include "ViewerApplication.h"

#define RC_PCM_TYPE lvr::ColorVertex<float, unsigned char>, lvr::Normal<float>

ViewerApplication::ViewerApplication( int argc, char ** argv )
{
    // Setup main window
    m_qMainWindow = new QMainWindow;
    m_mainWindowUi = new MainWindow;


    m_mainWindowUi->setupUi(m_qMainWindow);

    // Add a spinbox for scene zoom to toolbar (cool ;-)
    QLabel* label = new QLabel(m_mainWindowUi->toolBar);
    label->setText("Zoom Scene: ");
    QAction* label_action = m_mainWindowUi->toolBar->addWidget(label);
    label_action->setVisible(true);

    QDoubleSpinBox* m_zoomSpinBox = new QDoubleSpinBox(m_mainWindowUi->toolBar);
    m_zoomSpinBox->setMinimum(0.01);
    m_zoomSpinBox->setMaximum(10000);
    m_zoomSpinBox->setSingleStep(0.5);
    m_zoomSpinBox->setDecimals(2);
    m_zoomSpinBox->setValue(1.00);

    m_zoomBoxAction = m_mainWindowUi->toolBar->addWidget(m_zoomSpinBox);
    m_zoomBoxAction->setVisible(true);

    m_zoomAction = new QAction(m_qMainWindow);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/qv_zoom.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_zoomAction->setIcon(icon);
    m_zoomAction->setObjectName(QString::fromUtf8("actionZoom"));
    m_zoomAction->setCheckable(false);
    m_zoomAction->setEnabled(true);
    m_mainWindowUi->toolBar->addAction(m_zoomAction);

    // Add dock widget for currently active objects in viewer
    m_sceneDockWidget = new QDockWidget(m_qMainWindow);
    m_sceneDockWidgetUi = new SceneDockWidgetUI;
    m_sceneDockWidgetUi->setupUi(m_sceneDockWidget);
    m_qMainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_sceneDockWidget);

    // Add tool box widget to dock area
    m_actionDockWidget = new QDockWidget(m_qMainWindow);
    m_actionDockWidgetUi = new ActionDockWidgetUI;
    m_actionDockWidgetUi->setupUi(m_actionDockWidget);
    m_qMainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_actionDockWidget);

    // Setup event manager objects
    m_viewerManager = new ViewerManager(m_qMainWindow);
    m_viewer = m_viewerManager->current();

    m_factory = new VisualizerFactory;


    // Show window
    m_qMainWindow->show();


    connectEvents();

    /* Load files given as command line arguments. */
    int i;
    for ( i = 1; i < argc; i++ ) {
        printf( "Loading »%s«…\n", argv[i] );
        openFile(string(argv[i]));
    }

    // Call a resize to fit viewers to their parent widgets
    m_viewer->setGeometry(m_qMainWindow->centralWidget()->geometry());
    m_viewer->setBackgroundColor(QColor(255, 255, 255));
    m_qMainWindow->setCentralWidget(m_viewer);

    // Initalize dialogs
    m_fogSettingsUI = 0;
    m_fogSettingsDialog = 0;

    m_playerDialog = new AnimationDialog(m_viewer);


}

void ViewerApplication::connectEvents()
{
    QTreeWidget* treeWidget = m_sceneDockWidgetUi->treeWidget;

    // File operations
    QObject::connect(m_mainWindowUi->action_Open , SIGNAL(triggered()), this, SLOT(openFile()));

    // Scene views
    connect(m_mainWindowUi->actionShow_entire_scene, SIGNAL(triggered()), m_viewer, SLOT(resetCamera()));
    connect(m_mainWindowUi->actionShowSelection, SIGNAL(triggered()), this, SLOT(centerOnSelection()));

    // Projections
    connect(m_mainWindowUi->actionXZ_ortho_projection,    SIGNAL(triggered()), this, SLOT(setViewerModeOrthoXZ()));
    connect(m_mainWindowUi->actionXY_ortho_projection,    SIGNAL(triggered()), this, SLOT(setViewerModeOrthoXY()));
    connect(m_mainWindowUi->actionYZ_ortho_projection,    SIGNAL(triggered()), this, SLOT(setViewerModeOrthoYZ()));
    connect(m_mainWindowUi->actionPerspective_projection, SIGNAL(triggered()), this, SLOT(setViewerModePerspective()));

    // Render Modes
    connect(m_mainWindowUi->actionVertexView,     SIGNAL(triggered()), this, SLOT(meshRenderModeChanged()));
    connect(m_mainWindowUi->actionSurfaceView,    SIGNAL(triggered()), this, SLOT(meshRenderModeChanged()));
    connect(m_mainWindowUi->actionWireframeView,  SIGNAL(triggered()), this, SLOT(meshRenderModeChanged()));

    connect(m_mainWindowUi->actionPointCloudView, SIGNAL(triggered()), this, SLOT(pointRenderModeChanged()));
    connect(m_mainWindowUi->actionPointNormalView, SIGNAL(triggered()), this, SLOT(pointRenderModeChanged()));

    connect(m_mainWindowUi->actionRenderingSettings, SIGNAL(triggered()), this, SLOT(displayRenderingSettings()));

    // Fog settings
    connect(m_mainWindowUi->actionToggle_fog, SIGNAL(triggered()),  this, SLOT(toggleFog()));
    connect(m_mainWindowUi->actionFog_settings, SIGNAL(triggered()),this, SLOT(displayFogSettingsDialog()));

    // Communication between the manager objects
    connect(m_factory, SIGNAL(visualizerCreated(Visualizer*)), m_viewerManager, SLOT(addDataCollector(Visualizer*)));
    connect(m_factory, SIGNAL(visualizerCreated(Visualizer*)), this,            SLOT(dataCollectorAdded(Visualizer*)));

    // Communication between tree widget items
    connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),         this, SLOT(treeItemClicked(QTreeWidgetItem*, int)));
    connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)),         this, SLOT(treeItemChanged(QTreeWidgetItem*, int)));
    connect(treeWidget, SIGNAL(itemSelectionChanged()),                     this, SLOT(treeSelectionChanged()));
    connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeContextMenuRequested(const QPoint &)));

    // Actions
    connect(m_sceneDockWidgetUi->actionExport, SIGNAL(triggered()), this, SLOT(saveSelectedObject()));
    connect(m_sceneDockWidgetUi->actionChangeName, SIGNAL(triggered()), this, SLOT(changeSelectedName()));


    connect(m_mainWindowUi->actionGenerateMesh,               SIGNAL(triggered()), this, SLOT(createMeshFromPointcloud()));

    // Action dock functions
    connect(m_actionDockWidgetUi->buttonCreateMesh, SIGNAL(clicked()), this, SLOT(createMeshFromPointcloud()));
    connect(m_actionDockWidgetUi->buttonTransform,  SIGNAL(clicked()), this, SLOT(transformObject()));
    connect(m_actionDockWidgetUi->buttonDelete,     SIGNAL(clicked()), this, SLOT(deleteObject()));
    connect(m_actionDockWidgetUi->buttonExport,     SIGNAL(clicked()), this, SLOT(saveSelectedObject()));
    connect(m_actionDockWidgetUi->buttonAnimation,  SIGNAL(clicked()), this, SLOT(createAnimation()));

    connect(m_zoomAction, SIGNAL(triggered()), this, SLOT(zoomChanged()));
}

void ViewerApplication::zoomChanged()
{
    QDoubleSpinBox* s = static_cast<QDoubleSpinBox*>(m_mainWindowUi->toolBar->widgetForAction(m_zoomBoxAction));
    m_viewer->zoomChanged(s->value());
}

void ViewerApplication::displayRenderingSettings()
{
    // Create dialog ui
    RenderingDialogUI*  render_ui = new RenderingDialogUI;
    QDialog dialog;
    render_ui->setupUi(&dialog);

    // Get selected object
    QTreeWidgetItem * t_item = m_sceneDockWidgetUi->treeWidget->currentItem();

    if(t_item)
    {
        if(t_item->type() >= PointCloudItem)
        {
            // Convert to custom item
            CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(t_item);

            // Get relevant values from renderable and set them in ui
            Renderable* renderable = item->renderable();
            render_ui->spinBoxLineWidth->setValue(renderable->lineWidth());
            render_ui->spinBoxPointSize->setValue(renderable->pointSize());

            // Execute dialog and set new values
            if(dialog.exec() == QDialog::Accepted)
            {
                renderable->setPointSize(render_ui->spinBoxPointSize->value());
                renderable->setLineWidth(render_ui->spinBoxLineWidth->value());
            }
        }
    }


}

void ViewerApplication::createMeshFromPointcloud()
{
//    // Some usings
//    using namespace lvr;
//
//    // Display mesh generation dialog
//    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
//    if(item)
//    {
//        if(item->type() == PointCloudItem || item->type() == MultiPointCloudItem)
//        {
//            CustomTreeWidgetItem* c_item = static_cast<CustomTreeWidgetItem*>(item);
//
//            // Create a dialog to parse options
//            QDialog* mesh_dialog = new QDialog(m_qMainWindow);
//            Ui::MeshingOptionsDialogUI* mesh_ui = new Ui::MeshingOptionsDialogUI;
//            mesh_ui->setupUi(mesh_dialog);
//            int result = mesh_dialog->exec();
//
//            // Check dialog result and create mesh
//            if(result == QDialog::Accepted)
//            {
//                // Get point cloud data
//                lvr::PointCloud* pc = static_cast<lvr::PointCloud*>(c_item->renderable());
//                PointBufferPtr loader = pc->model()->m_pointCloud;
//
//                if(loader)
//                {
//                    // Create a point cloud manager object
//                    lvr::AdaptiveKSearchSurface< RC_PCM_TYPE >::Ptr pcm;
////                    PointCloudManager<ColorVertex<float, unsigned char>, Normal<float> >* pcm;
//                    std::string pcm_name = mesh_ui->comboBoxPCM->currentText().toAscii().data();
//
//                    pcm = AdaptiveKSearchSurface< RC_PCM_TYPE >::Ptr(
//                          new AdaptiveKSearchSurface<ColorVertex<float, unsigned char>, Normal<float> > ( loader, pcm_name ));
//
//                    // Set pcm parameters
//                    pcm->setKD(mesh_ui->spinBoxKd->value());
//                    pcm->setKI(mesh_ui->spinBoxKi->value());
//                    pcm->setKN(mesh_ui->spinBoxKn->value());
//                    pcm->calculateSurfaceNormals();
//
//                    // Create an empty mesh
//                    HalfEdgeMesh<ColorVertex<float, unsigned char>, Normal<float> > mesh(pcm);
//
//                    // Get reconstruction mesh
//                    float voxelsize = mesh_ui->spinBoxVoxelsize->value();
//
//                    FastReconstruction<ColorVertex<float, unsigned char>, Normal<float> > reconstruction(pcm, voxelsize, true);
//                    reconstruction.getMesh(mesh);
//
//                    // Get optimization parameters
//                    bool optimize_planes = mesh_ui->checkBoxOptimizePlanes->isChecked();
//                    bool fill_holes      = mesh_ui->checkBoxFillHoles->isChecked();
//                    bool rda             = mesh_ui->checkBoxRDA->isChecked();
//                    bool small_regions   = mesh_ui->checkBoxRemoveRegions->isChecked();
//                    bool retesselate     = mesh_ui->checkBoxRetesselate->isChecked();
//                    bool texture         = mesh_ui->checkBoxGenerateTextures->isChecked();
//                    bool color_regions   = mesh_ui->checkBoxColorRegions->isChecked();
//
//                    int  num_plane_its   = mesh_ui->spinBoxPlaneIterations->value();
//                    int  num_rda         = mesh_ui->spinBoxRDA->value();
//                    int  num_rm_regions  = mesh_ui->spinBoxRemoveRegions->value();
//
//                    float min_plane_size = mesh_ui->spinBoxMinPlaneSize->value();
//                    float normal_thresh  = mesh_ui->spinBoxNormalThr->value();
//                    float max_hole_size  = mesh_ui->spinBoxHoleSize->value();
//
//                    if(rda)
//                    {
//                        mesh.removeDanglingArtifacts(num_rda);
//                    }
//
//                    if(!small_regions)
//                    {
//                        num_rm_regions = 0;
//                    }
//
//                    // Perform optimizations
//                    if(optimize_planes)
//                    {
//                        if(color_regions)
//                        {
//                            mesh.enableRegionColoring();
//                        }
//
//                        mesh.optimizePlanes(num_plane_its,
//                                normal_thresh,
//                                min_plane_size,
//                                num_rm_regions,
//                                true);
//
//                        mesh.fillHoles(max_hole_size);
//                        mesh.optimizePlaneIntersections();
//                        mesh.restorePlanes(min_plane_size);
//                    }
//
//                    if(retesselate)
//                    {
//                        mesh.finalizeAndRetesselate(texture);
//                    }
//                    else
//                    {
//                        mesh.finalize();
//                    }
//
//
//                    // Create and add mesh to loaded objects
//                    MeshBufferPtr l = mesh.meshBuffer();
//
//                    lvr::StaticMesh* static_mesh = new lvr::StaticMesh(l);
//                    TriangleMeshTreeWidgetItem* mesh_item = new TriangleMeshTreeWidgetItem(TriangleMeshItem);
//
//                    int modes = 0;
//                    modes |= Mesh;
//
//                    string name = "Mesh: " + c_item->name();
//
//                    cout << static_mesh->getNumberOfFaces() << endl;
//                    cout << static_mesh->getNumberOfVertices() << endl;
//
//                    if(static_mesh->getNormals())
//                    {
//                        modes |= VertexNormals;
//                    }
//                    mesh_item->setSupportedRenderModes(modes);
//                    mesh_item->setViewCentering(false);
//                    mesh_item->setName(name);
//                    mesh_item->setRenderable(static_mesh);
//                    mesh_item->setNumFaces(static_mesh->getNumberOfFaces());
//
//                    Static3DDataCollector* dc = new Static3DDataCollector(static_mesh, name, mesh_item);
//
//                    dataCollectorAdded(dc);
//                    m_viewerManager->addDataCollector(dc);
//                }
//                else
//                {
//                    QMessageBox msgBox;
//                    msgBox.setText("Action not supported.");
//                    msgBox.setStandardButtons(QMessageBox::Ok  );
//                    int ret = msgBox.exec();
//                }
//
//            }
//        }
//    }

}

void ViewerApplication::openFile()
{
    QFileDialog file_dialog;
    QStringList file_names;
    QStringList file_types;

    file_types
      << "OBJ Files + Material (*.obj)"
      << "PLY Models (*.ply)"
      << "Point Clouds (*.pts)"
      // << "Points and Normals (*.nor)"
      // << "Polygonal Meshes (*.bor)"
      << "All Files (*.*)";

    //Set Title
    file_dialog.setWindowTitle("Open File");
    file_dialog.setFileMode(QFileDialog::ExistingFile);
    file_dialog.setNameFilters(file_types);

    if(file_dialog.exec()){
        file_names = file_dialog.selectedFiles();
    } else {
        return;
    }

    //Get filename from list
    string file_name = file_names.constBegin()->toStdString();
    m_factory->create(file_name);
}

void ViewerApplication::meshRenderModeChanged()
{
    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
    if(item)
    {
        if(item->type() == TriangleMeshItem)
        {
            TriangleMeshTreeWidgetItem* t_item = static_cast<TriangleMeshTreeWidgetItem*>(item);

            // Setup new render mode
            lvr::StaticMesh* mesh = static_cast<lvr::StaticMesh*>(t_item->renderable());

            int renderMode = 0;

            // Check states of buttons
            if(m_mainWindowUi->actionSurfaceView->isChecked()) renderMode |= lvr::RenderSurfaces;
            if(m_mainWindowUi->actionWireframeView->isChecked()) renderMode |= lvr::RenderTriangles;

            // Set proper render mode and forbid nothing selected
            if(renderMode != 0)
            {
                mesh->setRenderMode(renderMode);
            }

            // Avoid inconsistencies in button toggle states
            m_mainWindowUi->actionSurfaceView->setChecked(mesh->getRenderMode() & lvr::RenderSurfaces);
            m_mainWindowUi->actionWireframeView->setChecked(mesh->getRenderMode() & lvr::RenderTriangles);

            // Force redisplay
            m_viewer->updateGL();
        }
    }
}

void ViewerApplication::pointRenderModeChanged()
{
    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
    if(item)
    {
        if(item->type() == PointCloudItem)
        {
            PointCloudTreeWidgetItem* t_item = static_cast<PointCloudTreeWidgetItem*>(item);
            lvr::PointCloud* pc = static_cast<lvr::PointCloud*>(t_item->renderable());

            int renderMode = 0;
            renderMode |= lvr::RenderPoints;
            if(m_mainWindowUi->actionPointNormalView->isChecked())
            {
                renderMode |= lvr::RenderNormals;
            }
            pc->setRenderMode(renderMode);
            m_viewer->updateGL();
        }
    }
}

void ViewerApplication::openFile(string filename)
{
    m_factory->create(filename);
}

void ViewerApplication::transformObject()
{
    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
    if(item)
    {
        if(item->type() > 1000)
        {
            CustomTreeWidgetItem* c_item = static_cast<CustomTreeWidgetItem*>(item);
            TransformationDialog* d = new TransformationDialog(m_viewer, c_item->renderable());
        }
    }
}

void ViewerApplication::createAnimation()
{
    m_playerDialog->show();
}

void ViewerApplication::deleteObject()
{
    // Ask in Microsoft stype ;-)
    QMessageBox msgBox;
    msgBox.setText("Remove selected object?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel );
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    // Only delete objects when uses says "Yeah man, ok! Do It!!"
    if(ret == QMessageBox::Ok)
    {
        QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
        if(item)
        {
            if(item->type() > 1000)
            {
                // Remove item from tree widget
                CustomTreeWidgetItem* c_item = static_cast<CustomTreeWidgetItem*>(item);
                int i = m_sceneDockWidgetUi->treeWidget->indexOfTopLevelItem(item);
                m_sceneDockWidgetUi->treeWidget->takeTopLevelItem(i);

                // Delete  data collector
                m_viewerManager->current()->removeDataObject(c_item);

            }
        }
    }
}

void ViewerApplication::treeContextMenuRequested(const QPoint &position)
{
    // Create suitable actions for clicked widget
    QList<QAction *> actions;
    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->itemAt(position);

    // Check if item is valid and parse supported actions
    if(item)
    {
        if(item->type() == MultiPointCloudItem)
        {
            QAction* mesh_action = m_mainWindowUi->actionGenerateMesh;
            actions.append(mesh_action);
        }

        if(item->type() == PointCloudItem)
        {
            QAction* mesh_action = m_mainWindowUi->actionGenerateMesh;
            actions.append(mesh_action);
        }


        // Add standard action to context menu
        actions.append(m_mainWindowUi->actionShowSelection);
        actions.append(m_sceneDockWidgetUi->actionExport);
        actions.append(m_sceneDockWidgetUi->actionChangeName);
    }

    // Display menu if actions are present
    if (actions.count() > 0)
    {
       QMenu::exec(actions, m_sceneDockWidgetUi->treeWidget->mapToGlobal(position));
    }

}

void ViewerApplication::saveSelectedObject()
{
    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
    if(item)
    {

        QFileDialog file_dialog;
        QStringList file_names;
        QStringList file_types;

        // Parse extensions by file type
        if(item->type() == PointCloudItem || item->type() == MultiPointCloudItem)
        {
            file_types
          << "PLY Models (*.ply)"
          << "Point Clouds (*.pts)"
          << "All Files (*.*)";
        }
        else if (item->type() == TriangleMeshItem)
        {
            file_types
          << "OBJ Models (*.obj)"
          << "PLY Models (*.ply)"
          << "All Files (*.*)";
        }
        else if (item->type() == ClusterItem)
        {
            cout << "CLUSTER" << endl;
            ClusterTreeWidgetItem* c_item = static_cast<ClusterTreeWidgetItem*>(item);
            c_item->saveCluster("export.clu");
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Object type not supported");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        //Set Title
        file_dialog.setWindowTitle("Save selected object");
        file_dialog.setNameFilters(file_types);

        if(file_dialog.exec()){
            file_names = file_dialog.selectedFiles();
        } else {
            return;
        }

        string file_name = file_names.constBegin()->toStdString();

        // Cast to custom item
        if(item->type() > 1000)
        {
            CustomTreeWidgetItem* c_item = static_cast<CustomTreeWidgetItem*>(item);

            lvr::ModelPtr m = c_item->renderable()->model();
            lvr::ModelFactory::saveModel( m, file_name );

        }

    }
}


void ViewerApplication::changeSelectedName()
{
    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
    if(item)
    {
        // Test for custom item
        if(item->type() > 1000)
        {
            CustomTreeWidgetItem* c_item = static_cast<CustomTreeWidgetItem*>(item);
            QString new_name = QInputDialog::getText(
                    0,
                    QString("Input new label"),
                    QString("Name:"),
                    QLineEdit::Normal,
                    QString(c_item->name().c_str()), 0, 0);
            c_item->setName(new_name.toStdString());
            c_item->renderable()->setName(new_name.toStdString());

            m_viewer->updateGL();
        }

    }
}



void ViewerApplication::dataCollectorAdded(Visualizer* d)
{
    if(d->treeItem())
    {
        m_sceneDockWidgetUi->treeWidget->addTopLevelItem(d->treeItem());
        updateToolbarActions(d->treeItem());
        updateActionDock(d->treeItem());
    }
}

void ViewerApplication::treeItemClicked(QTreeWidgetItem* item, int d)
{
    // Center view on selected item if enabled
    if(item->type() > 1000)
    {
        CustomTreeWidgetItem* custom_item = static_cast<CustomTreeWidgetItem*>(item);
        if(custom_item->centerOnClick())
        {
            m_viewer->centerViewOnObject(custom_item->renderable());
            updateToolbarActions(custom_item);
            updateActionDock(custom_item);
        }
    }

    // Parse special operations of different items
}

void ViewerApplication::treeItemChanged(QTreeWidgetItem* item, int d)
{
    if(item->type() > 1000)
    {
        CustomTreeWidgetItem* custom_item = static_cast<CustomTreeWidgetItem*>(item);
        custom_item->renderable()->setActive(custom_item->checkState(d) == Qt::Checked);
        m_viewer->updateGL();
    }
}


void ViewerApplication::treeSelectionChanged()
{

    // Unselect all custom items
    QTreeWidgetItemIterator w_it( m_sceneDockWidgetUi->treeWidget);
    while (*w_it)
    {
        if( (*w_it)->type() >= ServerItem)
        {
            CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(*w_it);
            item->renderable()->setSelected(false);
        }
        ++w_it;
    }

    QList<QTreeWidgetItem *> list = m_sceneDockWidgetUi->treeWidget->selectedItems();
    QList<QTreeWidgetItem *>::iterator it = list.begin();

    for(it = list.begin(); it != list.end(); it++)
    {
        if( (*it)->type() >= ServerItem)
        {
            // Get selected item
            CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(*it);
            item->renderable()->setSelected(true);

            // Update render modes in tool bar
            updateToolbarActions(item);
            updateActionDock(item);

        }
    }

    m_viewer->updateGL();
}

void ViewerApplication::updateToolbarActions(CustomTreeWidgetItem* item)
{
    bool point_support = item->supportsMode(Points);
    bool pn_support = item->supportsMode(PointNormals);
    //    bool vn_support = item->supportsMode(VertexNormals);
    bool mesh_support = item->supportsMode(Mesh);


    m_mainWindowUi->actionVertexView->setEnabled(false);
    m_mainWindowUi->actionWireframeView->setEnabled(false);
    m_mainWindowUi->actionSurfaceView->setEnabled(false);
    m_mainWindowUi->actionPointCloudView->setEnabled(false);
    m_mainWindowUi->actionGenerateMesh->setEnabled(false);
    m_mainWindowUi->actionPointNormalView->setEnabled(false);

    if(mesh_support)
    {
        m_mainWindowUi->actionVertexView->setEnabled(true);
        m_mainWindowUi->actionWireframeView->setEnabled(true);
        m_mainWindowUi->actionSurfaceView->setEnabled(true);
    }

    if(point_support)
    {
        m_mainWindowUi->actionPointCloudView->setEnabled(true);
    }

    if(pn_support)
    {
        m_mainWindowUi->actionPointNormalView->setEnabled(true);
    }


}

void ViewerApplication::updateActionDock(CustomTreeWidgetItem* item)
{
       // Enable meshing for point clouds
       m_actionDockWidgetUi->buttonCreateMesh->setEnabled(item->supportsMode(Points));

       // Don't allow deletion of sub point clouds from multi
       // point cloud items
       m_actionDockWidgetUi->buttonDelete->setEnabled(true);
       if(item->type() == PointCloudItem)
       {
           if(item->parent())
           {
               if(item->parent()->type() == MultiPointCloudItem)
               {
                   m_actionDockWidgetUi->buttonDelete->setEnabled(false);
               }
           }
       }
}

void ViewerApplication::toggleFog()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->toggleFog();
    }
}

void ViewerApplication::displayFogSettingsDialog()
{
    if(!m_fogSettingsUI)
    {
        m_fogSettingsUI = new Fogsettings;
        m_fogSettingsDialog = new QDialog(m_qMainWindow);
        m_fogSettingsUI->setupUi(m_fogSettingsDialog);

//      QObject::connect(m_fogSettingsUI->sliderDensity, SIGNAL(valueChanged(int)),
//                      this, SLOT(fogDensityChanged(int)));
        QObject::connect(m_fogSettingsUI->sliderDensity, SIGNAL(sliderMoved(int)),
                        this, SLOT(fogDensityChanged(int)));
        QObject::connect(m_fogSettingsUI->radioButtonLinear , SIGNAL(clicked()),
                        this, SLOT(fogLinear()));
        QObject::connect(m_fogSettingsUI->radioButtonExp , SIGNAL(clicked()),
                        this, SLOT(fogExp()));
        QObject::connect(m_fogSettingsUI->radioButtonExp2 , SIGNAL(clicked()),
                        this, SLOT(fogExp2()));

    }

    m_fogSettingsDialog->show();
    m_fogSettingsDialog->raise();
    m_fogSettingsDialog->activateWindow();

}




void ViewerApplication::fogDensityChanged(int i)
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setFogDensity(1.0f * i / 2000.0f);
    }
}

void ViewerApplication::fogLinear()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setFogType(FOG_LINEAR);
    }
}

void ViewerApplication::fogExp2()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setFogType(FOG_EXP2);
    }
}

void ViewerApplication::fogExp()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setFogType(FOG_EXP);
    }
}

void ViewerApplication::setViewerModePerspective()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setProjectionMode(PERSPECTIVE);
    }
}

void ViewerApplication::setViewerModeOrthoXY()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setProjectionMode(ORTHOXY);
    }
}

void ViewerApplication::setViewerModeOrthoXZ()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setProjectionMode(ORTHOXZ);
    }
}

void ViewerApplication::setViewerModeOrthoYZ()
{
    if(m_viewer->type() == PERSPECTIVE_VIEWER)
    {
        (static_cast<PerspectiveViewer*>(m_viewer))->setProjectionMode(ORTHOYZ);
    }
}

void ViewerApplication::centerOnSelection()
{
    QTreeWidgetItem* item = m_sceneDockWidgetUi->treeWidget->currentItem();
    if(item)
    {
        if(item->type() > 1000)
        {
            CustomTreeWidgetItem* c_item = static_cast<CustomTreeWidgetItem*>(item);
            m_viewer->centerViewOnObject(c_item->renderable());
        }
    }
}

ViewerApplication::~ViewerApplication()
{
    //if(m_qMainWindow != 0) delete m_qMainWindow;
    //if(m_mainWindowUI != 0) delete m_mainWindowUI;
    if(m_viewer != 0) delete m_viewer;
}
