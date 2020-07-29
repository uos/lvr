#include "LVRModelTreeWidget.hpp"
#include <iostream>
#include "LVRItemTypes.hpp"

LVRModelTreeWidget::LVRModelTreeWidget(QWidget *parent)
{
}

void LVRModelTreeWidget::addTopLevelItem(QTreeWidgetItem *item)
{
    /*
    if(item->type() == lvr2::LVRModelItemType)
    {
        lvr2::LVRModelItem *modelItem = static_cast<lvr2::LVRModelItem *>(item);
    }else if(item->type() == lvr2::LVRLabeledScanProjectEditMarkItemType)
    {
        lvr2::LVRLabeledScanProjectEditMarkItem *modelItem = static_cast<lvr2::LVRLabeledScanProjectEditMarkItem *>(item);
    }
    else
    {*/
        QTreeWidget::addTopLevelItem(item);
    //}
}

void LVRModelTreeWidget::addScanProject(lvr2::ScanProjectPtr scanProject)
{
    lvr2::ScanProjectBridgePtr scanBridgePtr;
    scanBridgePtr = lvr2::ScanProjectBridgePtr(new lvr2::LVRScanProjectBridge(scanProject));
    lvr2::LVRScanProjectItem * item = new lvr2::LVRScanProjectItem(scanBridgePtr, "");
    QTreeWidget::addTopLevelItem(item);
}

void LVRModelTreeWidget::addLabeledScanProjectEditMark(lvr2::LabeledScanProjectEditMarkPtr labeledScanProject)
{
    lvr2::LabeledScanProjectEditMarkBridgePtr labelScanBridgePtr;
    labelScanBridgePtr = lvr2::LabeledScanProjectEditMarkBridgePtr(new lvr2::LVRLabeledScanProjectEditMarkBridge(labeledScanProject));
    lvr2::LVRLabeledScanProjectEditMarkItem *item = new lvr2::LVRLabeledScanProjectEditMarkItem(labelScanBridgePtr, "Root");
    QTreeWidget::addTopLevelItem(item);

}
