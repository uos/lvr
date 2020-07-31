#include "LVRScanPositionItem.hpp"
#include "LVRPointCloudItem.hpp"
#include "LVRTextureMeshItem.hpp"
#include "LVRItemTypes.hpp"
#include "LVRModelItem.hpp"
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <sstream>

namespace lvr2
{

LVRScanPositionItem::LVRScanPositionItem(ScanPositionBridgePtr bridge, QString name) :
    QTreeWidgetItem(LVRScanPositionItemType), m_scanPositionBridge(bridge), m_name(name)
{
    for(int i = 0; i < bridge->getScanPosition()->scans.size(); i++)
    {
        std::stringstream pos;
        pos << "" << std::setfill('0') << std::setw(8) << i;
        std::string posName = pos.str();
        std::vector<ModelBridgePtr> models;
        models = bridge->getModels();
        LVRModelItem* modelItem = new LVRModelItem(models[i], QString::fromStdString(posName));
        addChild(modelItem);
    }
    
    LVRPoseItem* posItem = new LVRPoseItem(bridge->getPose());
    addChild(posItem);

    // Setup item properties
    setText(0, name);
    setCheckState(0, Qt::Checked);

}

LVRScanPositionItem::LVRScanPositionItem(const LVRScanPositionItem& item)
{
    m_scanPositionBridge   = item.m_scanPositionBridge;
    m_name          = item.m_name;
}


QString LVRScanPositionItem::getName()
{
    return m_name;
}

void LVRScanPositionItem::setName(QString name)
{
    m_name = name;
    setText(0, m_name);
}

ScanPositionBridgePtr LVRScanPositionItem::getScanPositionBridge()
{
	return m_scanPositionBridge;
}

bool LVRScanPositionItem::isEnabled()
{
    return this->checkState(0);
}

void LVRScanPositionItem::setVisibility(bool visible)
{
    std::cout << "Im here " <<std::endl;
    for (auto model : m_scanPositionBridge->getModels())
    {
        model->setVisibility(visible);
    }
}
void LVRScanPositionItem::setModelVisibility(int column, bool globalValue)
{
    std::cout << "Im here " <<std::endl;
    if(checkState(column) == globalValue || globalValue == true)
    {
        setVisibility(checkState(column));
    }
}
LVRScanPositionItem::~LVRScanPositionItem()
{
    // TODO Auto-generated destructor stub
}

}
