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
 * LVRPointCloudItem.cpp
 *
 *  @date Feb 7, 2014
 *  @author Thomas Wiemann
 */
#include "LVRPointCloudItem.hpp"
#include "LVRItemTypes.hpp"

namespace lvr
{

LVRPointCloudItem::LVRPointCloudItem(PointBufferBridgePtr& ptr, QTreeWidgetItem* item) :
       QTreeWidgetItem(item, LVRPointCloudItemType), m_parent(item), m_pointBridge(ptr)
{
    // Setup tree widget icon
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/qv_pc_tree_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
    setIcon(0, icon);
    setText(0, "Point Cloud");
    setExpanded(true);

    // Setup Infos
    QTreeWidgetItem* numItem = new QTreeWidgetItem(this);
    QString num;
    numItem->setText(0, "Num Points:");
    numItem->setText(1, num.setNum(ptr->getNumPoints()));
    addChild(numItem);

    QTreeWidgetItem* normalItem = new QTreeWidgetItem(this);
    normalItem->setText(0, "Has Normals:");
    if(ptr->hasNormals())
    {
        normalItem->setText(1, "yes");
    }
    else
    {
        normalItem->setText(1, "no");
    }
    addChild(normalItem);

    QTreeWidgetItem* colorItem = new QTreeWidgetItem(this);
    colorItem->setText(0, "Has Colors:");
    if(ptr->hasColors())
    {
        colorItem->setText(1, "yes");
    }
    else
    {
        colorItem->setText(1, "no");
    }
    addChild(colorItem);

    QTreeWidgetItem* specItem = new QTreeWidgetItem(this);
    specItem->setText(0, "Has Spectraldata:");
    if(ptr->getPointBuffer()->hasPointSpectralChannels())
    {
        specItem->setText(1, "yes");
    }
    else
    {
        specItem->setText(1, "no");
    }
    addChild(specItem);

    // set initial values
    m_opacity = 1;
    m_pointSize = 1;
    m_color = QColor(255,255,255);
    m_visible = true;
}

QColor LVRPointCloudItem::getColor()
{
	return m_color;
}

void LVRPointCloudItem::setColor(QColor &c)
{
    m_pointBridge->setBaseColor(c.redF(), c.greenF(), c.blueF());
    m_color = c;
}

void LVRPointCloudItem::setSelectionColor(QColor& c)
{
    m_pointBridge->setBaseColor(c.redF(), c.greenF(), c.blueF());
}

void LVRPointCloudItem::resetColor()
{
    m_pointBridge->setBaseColor(m_color.redF(), m_color.greenF(), m_color.blueF());
}

int LVRPointCloudItem::getPointSize()
{
	return m_pointSize;
}

void LVRPointCloudItem::setPointSize(int &pointSize)
{
    m_pointBridge->setPointSize(pointSize);
    m_pointSize = pointSize;
}

float LVRPointCloudItem::getOpacity()
{
	return m_opacity;
}

void LVRPointCloudItem::setOpacity(float &opacity)
{
    m_pointBridge->setOpacity(opacity);
    m_opacity = opacity;
}

bool LVRPointCloudItem::getVisibility()
{
	return m_visible;
}

void LVRPointCloudItem::setVisibility(bool &visibility)
{
	m_pointBridge->setVisibility(visibility);
	m_visible = visibility;
}

size_t LVRPointCloudItem::getNumPoints()
{
    return m_pointBridge->getNumPoints();
}

PointBufferPtr LVRPointCloudItem::getPointBuffer()
{
    return m_pointBridge->getPointBuffer();
}

PointBufferBridgePtr LVRPointCloudItem::getPointBufferBridge()
{
    return m_pointBridge;
}

vtkSmartPointer<vtkActor> LVRPointCloudItem::getActor()
{
	return m_pointBridge->getPointCloudActor();
}

LVRPointCloudItem::~LVRPointCloudItem()
{
    // TODO Auto-generated destructor stub
}

} /* namespace lvr */
