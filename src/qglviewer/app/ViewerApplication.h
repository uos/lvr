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
 * ViewerApplication.h
 *
 *  Created on: 15.09.2010
 *      Author: Thomas Wiemannn
 */

#ifndef VIEWERAPPLICATION_H_
#define VIEWERAPPLICATION_H_

#include <QtGui>

#include "MainWindow.h"
#include "FogDensityDialog.h"
#include "SceneDockWidgetUI.h"
#include "ActionDockWidgetUI.h"
#include "MeshingOptionsDialogUI.h"
#include "RenderingDialogUI.h"

#include "../data/VisualizerFactory.hpp"
#include "../data/SignalingMeshGenerator.hpp"

#include "../viewers/Viewer.h"
#include "../viewers/PerspectiveViewer.h"
#include "../viewers/ViewerManager.h"

#include "../widgets/CustomTreeWidgetItem.h"
#include "../widgets/ClusterTreeWidgetItem.h"
#include "../widgets/PointCloudTreeWidgetItem.h"
#include "../widgets/TriangleMeshTreeWidgetItem.h"
#include "../widgets/TransformationDialog.h"
#include "../widgets/DebugOutputDialog.hpp"
#include "../widgets/AnimationDialog.hpp"

#include "display/StaticMesh.hpp"
#include "geometry/HalfEdgeMesh.hpp"

#include "reconstruction/AdaptiveKSearchSurface.hpp"
#include "reconstruction/FastReconstruction.hpp"

#include "io/Model.hpp"
#include "io/ModelFactory.hpp"

using Ui::MainWindow;
using Ui::Fogsettings;
using Ui::SceneDockWidgetUI;
using Ui::ActionDockWidgetUI;
using Ui::RenderingDialogUI;

class EventManager;

class ViewerApplication : public QObject
{
	Q_OBJECT

public:
	ViewerApplication(int argc, char** argv);
	virtual ~ViewerApplication();

public Q_SLOTS:
	void setViewerModePerspective();
	void setViewerModeOrthoXY();
	void setViewerModeOrthoXZ();
	void setViewerModeOrthoYZ();
	void toggleFog();
	void displayFogSettingsDialog();
	void fogDensityChanged(int i);
	void fogLinear();
	void fogExp2();
	void fogExp();

	void displayRenderingSettings();

	void dataCollectorAdded(Visualizer* d);
	void treeItemClicked(QTreeWidgetItem* item, int n);
	void treeItemChanged(QTreeWidgetItem*, int);
	void treeSelectionChanged();
	void treeContextMenuRequested(const QPoint &);

	void saveSelectedObject();
	void changeSelectedName();

	void transformObject();
	void createAnimation();
	void deleteObject();

	void openFile();

	void meshRenderModeChanged();
	void pointRenderModeChanged();
	void createMeshFromPointcloud();
	void centerOnSelection();

	void zoomChanged();

private:

	void updateToolbarActions(CustomTreeWidgetItem* item);
	void updateActionDock(CustomTreeWidgetItem* item);
	void connectEvents();
	void openFile(string filename);

	MainWindow*					m_mainWindowUi;
	QMainWindow*				m_qMainWindow;
	QDialog*					m_fogSettingsDialog;

	SceneDockWidgetUI*			m_sceneDockWidgetUi;
	ActionDockWidgetUI*         m_actionDockWidgetUi;

	QDockWidget*				m_sceneDockWidget;
	QDockWidget*                m_actionDockWidget;

	QDoubleSpinBox*             m_zoomSpinBox;
	QAction*                    m_zoomAction;
	QAction*                    m_zoomBoxAction;

	Fogsettings*				m_fogSettingsUI;
	ViewerManager*				m_viewerManager;
	VisualizerFactory*       	m_factory;

	AnimationDialog*            m_playerDialog;


public:
    Viewer*                     m_viewer;
};

#endif /* VIEWERAPPLICATION_H_ */
