#ifndef LVRMODELTREEWIDGET_HPP
#define LVRMODELTREEWIDGET_HPP
#include <QTreeWidget>
#include "LVRModelItem.hpp"
#include "LVRLabeledScanProjectEditMarkItem.hpp"
#include "lvr2/types/ScanTypes.hpp"
#include "lvr2/types/MatrixTypes.hpp"
class LVRModelTreeWidget : public QTreeWidget
{
public:
    LVRModelTreeWidget(QWidget *parent = nullptr);
    //using QTreeWidget::QTreeWidget;
    void addTopLevelItem(QTreeWidgetItem *item);

    void addModelItem(lvr2::LVRModelItem *item);
    void addLabelScanProjectEditMarkItem(lvr2::LVRLabeledScanProjectEditMarkItem *item);

    //std::vector<> get**ModelItems();
};

#endif //LVRMODELTREEWIDGET_HPP
