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
#include "../vtkBridge/LVRModelBridge.hpp"
#include "../widgets/LVRModelItem.hpp"
#include "../widgets/LVRItemTypes.hpp"
#include "../widgets/LVRTransformationDialog.hpp"
#include "../widgets/LVRPointCloudItem.hpp"
#include "../widgets/LVRMeshItem.hpp"


#include "io/ModelFactory.hpp"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPointPicker.h>

namespace lvr
{

LVRMainWindow::LVRMainWindow()
{
    setupUi(this);
    setupQVTK();


    // Init members
    m_correspondanceDialog = new LVRCorrespondanceDialog(treeWidget);

    // Setup specific properties
    QHeaderView* v = this->treeWidget->header();
    v->resizeSection(0, 175);

    treeWidget->setSelectionMode( QAbstractItemView::SingleSelection);
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_treeContextMenu = new QMenu;
    m_actionShowColorDialog = new QAction("Select base color...", this);
    m_actionDeleteModelItem = new QAction("Delete model", this);

    m_treeContextMenu->addAction(m_actionShowColorDialog);
    m_treeContextMenu->addAction(m_actionDeleteModelItem);


    m_pickingInteractor = new LVRPickingInteractor(m_renderer);
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle( m_pickingInteractor );
    vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
    connectSignalsAndSlots();
}



LVRMainWindow::~LVRMainWindow()
{
    if(m_correspondanceDialog)
    {
        delete m_correspondanceDialog;
    }
}

void LVRMainWindow::renderVtkStuff()
{
    this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::setupQVTK()
{
    // Add new renderer to the render window of the QVTKWidget
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    this->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);
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

void LVRMainWindow::alignPointClouds()
{
    Matrix4f mat = m_correspondanceDialog->getTransformation();
    QString name = m_correspondanceDialog->getModelName();
    cout << name.toStdString() << endl;
    cout << mat << endl;

    QTreeWidgetItemIterator it(treeWidget);
    while (*it)
    {
        if ( (*it)->type() == LVRModelItemType)
        {
            if( (*it)->text(0) == name)
            {
                float pose[6];
                LVRModelItem* item = static_cast<LVRModelItem*>(*it);
                mat.toPostionAngle(pose);
                Pose p;
                p.x = pose[0];
                p.y = pose[1];
                p.z = pose[2];
                p.r = pose[3];
                p.t = pose[4];
                p.p = pose[5];
                item->setPose(p);
            }
        }
        ++it;
    }
}

void LVRMainWindow::connectSignalsAndSlots()
{
    QObject::connect(actionOpen, SIGNAL(activated()), this, SLOT(loadModel()));
    QObject::connect(this->actionICP_using_manual_correspondance, SIGNAL(activated()), this, SLOT(manualICP()));
    QObject::connect(buttonTransformModel, SIGNAL(pressed()), this, SLOT(showTransformationDialog()));
    QObject::connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeContextMenu(const QPoint&)));

    QObject::connect(m_actionShowColorDialog, SIGNAL(activated()), this, SLOT(showColorDialog()));

    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(accepted()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(accepted()), this, SLOT(alignPointClouds()));

    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(rejected()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog, SIGNAL(addArrow(LVRVtkArrow*)), this, SLOT(addArrow(LVRVtkArrow*)));
    QObject::connect(m_correspondanceDialog, SIGNAL(removeArrow(LVRVtkArrow*)), this, SLOT(removeArrow(LVRVtkArrow*)));

    QObject::connect(m_correspondanceDialog, SIGNAL(render()), this, SLOT(renderVtkStuff()));


    QObject::connect(m_pickingInteractor, SIGNAL(firstPointPicked(double*)),m_correspondanceDialog, SLOT(firstPointPicked(double*)));
    QObject::connect(m_pickingInteractor, SIGNAL(secondPointPicked(double*)),m_correspondanceDialog, SLOT(secondPointPicked(double*)));

    QObject::connect(this, SIGNAL(correspondenceDialogOpened()), m_pickingInteractor, SLOT(correspondenceSearchOn()));
    QObject::connect(this, SIGNAL(correspondenceDialogOpened()), m_pickingInteractor, SLOT(correspondenceSearchOn()));

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
			qvtkWidget->GetRenderWindow()->Render();
		}
		else if(item->type() == LVRMeshItemType)
		{
		    LVRMeshItem* mesh_item = static_cast<LVRMeshItem*>(item);
		    mesh_item->setColor(c);
		    qvtkWidget->GetRenderWindow()->Render();
		}
	}
}

void LVRMainWindow::showTreeContextMenu(const QPoint& p)
{
	// Only display context menu for point clounds and meshes
	QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
	if(items.size() > 0)
	{
		QTreeWidgetItem* item = items.first();
		if(item->type() == LVRPointCloudItemType || item->type() == LVRMeshItemType)
		{
			QPoint globalPos = treeWidget->mapToGlobal(p);
			m_treeContextMenu->exec(globalPos);
		}
	}
}

void LVRMainWindow::loadModel()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Open Model"), "", tr("Model Files (*.ply *.obj *.pts *.3d *.txt)"));


    QStringList::Iterator it = filenames.begin();

    while(it != filenames.end())
    {

        // Load model and generate vtk representation
        ModelPtr model = ModelFactory::readModel((*it).toStdString());
        ModelBridgePtr bridge( new LVRModelBridge(model));
        bridge->addActors(m_renderer);

        // Add item for this model to tree widget
        QFileInfo info((*it));
        QString base = info.fileName();
        LVRModelItem* item = new LVRModelItem(bridge, base);
        this->treeWidget->addTopLevelItem(item);
        item->setExpanded(true);
        ++it;
    }

    // Update camera to new scene dimension and force rendering
    m_renderer->ResetCamera();
    this->qvtkWidget->GetRenderWindow()->Render();

}

void LVRMainWindow::manualICP()
{
    m_correspondanceDialog->fillComboBoxes();
    m_correspondanceDialog->m_dialog->show();
    m_correspondanceDialog->m_dialog->raise();
    m_correspondanceDialog->m_dialog->activateWindow();
    Q_EMIT(correspondenceDialogOpened());
}

void LVRMainWindow::showTransformationDialog()
{
    // Setup a message box for unsupported items
    QMessageBox box;
    box.setText("Unsupported Item for Transformation.");
    box.setInformativeText("Only whole models, point clouds or meshes can be transformed.");
    box.setStandardButtons(QMessageBox::Ok);

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
                box.exec();
            }
        }
        else
        {
            box.exec();
        }
    }
}


} /* namespace lvr */
