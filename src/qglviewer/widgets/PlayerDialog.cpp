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
 * PlayerDialog.cpp
 *
 *  @date 19.12.2011
 *  @author Thomas Wiemann
 */

#include "PlayerDialog.hpp"
#include "VideoExportDialog.hpp"

#include <iostream>
using std::cout;
using std::endl;

namespace lssr
{

PlayerDialog::PlayerDialog(Viewer* parent)
    : m_parent(parent), m_item(0)
{
    m_ui = new PlayerDialogUI;

    m_dialog = new QDialog(parent);
    m_ui->setupUi(m_dialog);

    connectEvents();
}

void PlayerDialog::show()
{
    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void PlayerDialog::connectEvents()
{
    connect(m_ui->buttonLast       , SIGNAL(clicked()),this, SLOT(selectLast()));
    connect(m_ui->buttonFirst      , SIGNAL(clicked()),this, SLOT(selectFirst()));
    connect(m_ui->buttonAddFrame   , SIGNAL(clicked()),this, SLOT(addItem()));
    connect(m_ui->buttonDeleteFrame, SIGNAL(clicked()),this, SLOT(removeItem()));
    connect(m_ui->buttonNext        ,SIGNAL(clicked()),this, SLOT(selectNext()));
    connect(m_ui->buttonPrev        ,SIGNAL(clicked()),this, SLOT(selectPrev()));
    connect(m_ui->buttonAnimate     ,SIGNAL(clicked()),this, SLOT(play()));

    connect(m_ui->buttonCreateVideo, SIGNAL(clicked()), this, SLOT(createVideo()));

    connect(m_ui->spinBoxCurrentTime,SIGNAL(valueChanged(double)),this, SLOT(updateTimes(double)));
    connect(m_ui->listWidget        ,SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateSelectedItem(QListWidgetItem*)));

}

void PlayerDialog::updateSelectedItem(QListWidgetItem* item)
{
    m_parent->m_kfi->stopInterpolation();
    if(item)
    {
        if(item->type() == PlayListItem)
        {
            m_item = static_cast<AnimationListItem*>(item);
            m_ui->spinBoxStartTime->setValue(m_item->time());
            m_ui->spinBoxCurrentTime->setValue(m_item->duration());
            m_ui->spinBoxLastTime->setValue(m_item->time() + m_item->duration());
        }

        m_parent->camera()->interpolateTo(m_item->frame(), 0.5);
    }

}


void PlayerDialog::addItem()
{
    m_parent->m_kfi->stopInterpolation();
    // Default duration is 1 second
    float duration = 1.0;

    // Get current camera frame
    qglviewer::Frame* frame = m_parent->camera()->frame();

    if(m_item)
    {
        AnimationListItem* item = new AnimationListItem(frame, m_ui->listWidget, duration);
        m_ui->listWidget->insertItem(m_ui->listWidget->row(m_item) + 1, item);
    }
    else
    {
        AnimationListItem* item = new AnimationListItem(frame, m_ui->listWidget, duration);
        m_ui->listWidget->insertItem(0, item);
        m_item = item;

    }

}

void PlayerDialog::removeItem()
{
    m_parent->m_kfi->stopInterpolation();
    if(m_item)
    {
        m_item->updateFollowingTimes(-m_item->duration());

        // Get next item
        AnimationListItem* next = m_item->getNext();
        AnimationListItem* prev = m_item->getPrev();

        // Remove current item item
        m_ui->listWidget->removeItemWidget(m_item);
        delete m_item;

        // Set current to previous
        m_item = next;

        // Check if we delete the last item in the list.
        // If we did, the nex item is the previous
        // one
        if(m_item == 0)
        {
            m_item = prev;
        }
    }
}

void PlayerDialog::updateTimes(double d)
{
    m_parent->m_kfi->stopInterpolation();
    if(m_item)
    {
        float oldValue = m_item->duration();
        float value = m_ui->spinBoxCurrentTime->value();

        m_item->setDuration(value);
        m_item->updateLabel();
        m_item->updateFollowingTimes(value - oldValue);
    }
}

void PlayerDialog::selectNext()
{
    m_parent->m_kfi->stopInterpolation();
    QListWidget* list = m_ui->listWidget;
    int next =  list->row(m_item) + 1;
    if(next > list->count() - 1)
    {
        next = list->count() - 1;
    }

    AnimationListItem* item = static_cast<AnimationListItem*>(list->item(next));
    list->setCurrentItem(item);
    m_item = item;
    m_parent->camera()->interpolateTo(m_item->frame(), 0.5);
}

void PlayerDialog::selectPrev()
{
    m_parent->m_kfi->stopInterpolation();
    QListWidget* list = m_ui->listWidget;
    int prev =  list->row(m_item) - 1;
    if( !(prev >= 0 && list->count()) )
    {
        prev = 0;
    }

    AnimationListItem* item = static_cast<AnimationListItem*>(list->item(prev));
    list->setCurrentItem(item);
    m_item = item;
    m_parent->camera()->interpolateTo(m_item->frame(), 0.5);
}
void PlayerDialog::selectLast()
{
    // Get first item from list
    m_parent->m_kfi->stopInterpolation();
    QListWidgetItem* item = m_ui->listWidget->item(m_ui->listWidget->count() - 1);
    m_ui->listWidget->setCurrentItem(item);
    m_item = static_cast<AnimationListItem*>(item);
    m_parent->camera()->interpolateTo(m_item->frame(), 0.5);
}

void PlayerDialog::selectFirst()
{
    // Get last item
    m_parent->m_kfi->stopInterpolation();
    QListWidgetItem* item = m_ui->listWidget->item(0);
    m_ui->listWidget->setCurrentItem(item);
    m_item = static_cast<AnimationListItem*>(item);
    m_parent->camera()->interpolateTo(m_item->frame(), 0.5);
}

void PlayerDialog::play()
{
    if(!m_parent->m_kfi->interpolationIsStarted())
    {
        m_parent->m_kfi->deletePath();

        QListWidget *list = m_ui->listWidget;
        for(int i = 0; i < list->count(); i++)
        {
            AnimationListItem *item = static_cast<AnimationListItem*>(list->item(i));
            m_parent->m_kfi->addKeyFrame(item->frame(), item->time());
        }
        m_parent->m_kfi->setLoopInterpolation(false);
        m_parent->m_kfi->startInterpolation();
    }
    else
    {
        m_parent->m_kfi->stopInterpolation();
    }


}

void PlayerDialog::createVideo()
{
    // Create a dialog to select output directory
    QFileDialog dialog(m_parent);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle("Select Output Directory");

    QStringList fileNames;
    if(dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }
    else
    {
        return;
    }

    // Get directory and save
    string outputDir = fileNames.constBegin()->toStdString() + "frame";
    m_parent->setSnapshotFileName(QString(outputDir.c_str()));

    // Open settingsdialog
    if(m_parent->openSnapshotFormatDialog())
    {
        m_parent->setSnapshotQuality(100);
        m_parent->setSnapshotCounter(0);
    }
    else
    {
        return;
    }

    // Start export
    play();


}


} /* namespace lssr */
