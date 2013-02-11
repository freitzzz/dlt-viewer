/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file filterdialog.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "filterdialog.h"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterDialog)
{
    ui->setupUi(this);

    connect(ui->buttonSelectColor, SIGNAL(pressed()), this, SLOT(on_buttonSelectColor_clicked()));

    ui->buttonSelectColor->setEnabled(false);
    ui->labelSelectedColor->setVisible(false);
}

FilterDialog::~FilterDialog()
{
    delete ui;
}


void FilterDialog::setType(int value)
{
    ui->comboBoxType->setCurrentIndex(value);
}

int FilterDialog::getType()
{
    return ui->comboBoxType->currentIndex();
}

void FilterDialog::setName(QString name)
{
    ui->lineEditName->setText(name);
}

QString FilterDialog::getName()
{
    return ui->lineEditName->text();
}

void FilterDialog::setEcuId(QString id)
{
    ui->lineEditEcuId->setText(id);
}

QString FilterDialog::getEcuId()
{
    return ui->lineEditEcuId->text();
}

void FilterDialog::setEnableEcuId(bool state)
{
    ui->checkBoxEcuId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableEcuId()
{
    return (ui->checkBoxEcuId->checkState() == Qt::Checked);
}

void FilterDialog::setApplicationId(QString id)
{
    ui->lineEditApplicationId->setText(id);
}

QString FilterDialog::getApplicationId()
{
    return ui->lineEditApplicationId->text();
}

void FilterDialog::setEnableApplicationId(bool state)
{
    ui->checkBoxApplicationId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableApplicationId()
{
    return (ui->checkBoxApplicationId->checkState() == Qt::Checked);
}

void FilterDialog::setContextId(QString id)
{
    ui->lineEditContextId->setText(id);
}

QString FilterDialog::getContextId()
{
    return ui->lineEditContextId->text();
}

void FilterDialog::setEnableContextId(bool state)
{
    ui->checkBoxContextId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableContextId()
{
    return (ui->checkBoxContextId->checkState() == Qt::Checked);
}

void FilterDialog::setHeaderText(QString id)
{
    ui->lineEditHeaderText->setText(id);
}

QString FilterDialog::getHeaderText()
{
    return ui->lineEditHeaderText->text();
}

void FilterDialog::setEnableHeaderText(bool state)
{
    ui->checkBoxHeaderText->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableHeaderText()
{
    return (ui->checkBoxHeaderText->checkState() == Qt::Checked);
}

void FilterDialog::setPayloadText(QString id)
{
    ui->lineEditPayloadText->setText(id);
}

QString FilterDialog::getPayloadText()
{
    return ui->lineEditPayloadText->text();
}

void FilterDialog::setEnablePayloadText(bool state)
{
    ui->checkBoxPayloadText->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnablePayloadText()
{
    return (ui->checkBoxPayloadText->checkState() == Qt::Checked);
}

void FilterDialog::setFilterColour(QColor color)
{
   QPalette palette = ui->labelSelectedColor->palette();
   palette.setColor(QPalette::Background,color);
   ui->labelSelectedColor->setPalette(palette);

}

QColor FilterDialog::getFilterColour()
{
    return ui->labelSelectedColor->palette().background().color();
}

void FilterDialog::setLogLevelMax(int value)
{
    ui->comboBoxLogLevelMax->setCurrentIndex(value);
}

int FilterDialog::getLogLevelMax()
{
    return ui->comboBoxLogLevelMax->currentIndex();
}

void FilterDialog::setEnableLogLevelMax(bool state)
{
    ui->checkBoxLogLevelMax->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableLogLevelMax()
{
    return (ui->checkBoxLogLevelMax->checkState() == Qt::Checked);
}

void FilterDialog::setLogLevelMin(int value)
{
    ui->comboBoxLogLevelMin->setCurrentIndex(value);
}

int FilterDialog::getLogLevelMin()
{
    return ui->comboBoxLogLevelMin->currentIndex();
}

void FilterDialog::setEnableLogLevelMin(bool state)
{
    ui->checkBoxLogLevelMin->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableLogLevelMin()
{
    return (ui->checkBoxLogLevelMin->checkState() == Qt::Checked);
}

void FilterDialog::setEnableCtrlMsgs(bool state)
{
    ui->checkBoxCtrlMsgs->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableCtrlMsgs()
{
    return (ui->checkBoxCtrlMsgs->checkState() == Qt::Checked);
}

void FilterDialog::setActive(bool state){
    ui->checkBoxActive->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableActive(){
    return (ui->checkBoxActive->checkState() == Qt::Checked);
}

void FilterDialog::on_buttonSelectColor_clicked()
{
    QColor selectedBackgroundColor = QColorDialog::getColor();
    if(selectedBackgroundColor.isValid())
    {
        QPalette palette = ui->labelSelectedColor->palette();
        palette.setColor(QPalette::Background,selectedBackgroundColor);
        ui->labelSelectedColor->setPalette(palette);

    }
}

void FilterDialog::on_comboBoxType_currentIndexChanged(int index){
    switch(index){
    case 0:
    case 1:
            ui->buttonSelectColor->setEnabled(false);
            ui->labelSelectedColor->setVisible(false);

            break;
    case 2:
            ui->buttonSelectColor->setEnabled(true);
            ui->labelSelectedColor->setVisible(true);
            break;
    }
}
/*
void FilterDialog::on_lineEditEcuId_editingFinished()
{
  //ui->checkBoxHeaderText->checkStateSet();
  //ui->checkBoxHeaderText->setCheckState(Qt::Checked);
  if (ui->lineEditEcuId->text().length())
    ui->checkBoxEcuId->setCheckState(Qt::Checked);
  else
    ui->checkBoxEcuId->setCheckState(Qt::Unchecked);
//    ui->FilterDialog::setEnableHeaderText(bool state)
}
*/
void FilterDialog::on_lineEditApplicationId_textEdited(const QString &arg1)
{
  if (ui->lineEditApplicationId->text().length())
    ui->checkBoxApplicationId->setCheckState(Qt::Checked);
  else
    ui->checkBoxApplicationId->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditEcuId_textEdited(const QString &arg1)
{
  if (ui->lineEditEcuId->text().length())
    ui->checkBoxEcuId->setCheckState(Qt::Checked);
  else
    ui->checkBoxEcuId->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditContextId_textEdited(const QString &arg1)
{
  if (ui->lineEditContextId->text().length())
    ui->checkBoxContextId->setCheckState(Qt::Checked);
  else
    ui->checkBoxContextId->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditHeaderText_textEdited(const QString &arg1)
{
  if (ui->lineEditHeaderText->text().length())
    ui->checkBoxHeaderText->setCheckState(Qt::Checked);
  else
    ui->checkBoxHeaderText->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_lineEditPayloadText_textEdited(const QString &arg1)
{
  if (ui->lineEditPayloadText->text().length())
    ui->checkBoxPayloadText->setCheckState(Qt::Checked);
  else
    ui->checkBoxPayloadText->setCheckState(Qt::Unchecked);
}

void FilterDialog::on_comboBoxLogLevelMax_currentIndexChanged(int index)
{
    ui->checkBoxLogLevelMax->setCheckState(Qt::Checked);
}

void FilterDialog::on_comboBoxLogLevelMin_currentIndexChanged(int index)
{
    ui->checkBoxLogLevelMin->setCheckState(Qt::Checked);
}
