/*  Copyright (C) 2011-2012 National Institute For Space Research (INPE) - Brazil.

This file is part of the TerraLib - a Framework for building GIS enabled applications.

TerraLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

TerraLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with TerraLib. See COPYING. If not, write to
TerraLib Team at <terralib-team@terralib.org>.
*/

/*!
\file fiocruz/src/fiocruz/flow/qt/FlowNetworkDialog.cpp

\brief This file defines the Flow Network dialog class
*/

// TerraLib
#include "FlowNetworkDialog.h"
#include "ui_FlowNetworkDialogForm.h"

// Qt
#include <QMessageBox>


Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::FlowNetworkDialog::FlowNetworkDialog(QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f),
m_ui(new Ui::FlowNetworkDialogForm)
{
  // add controls
  m_ui->setupUi(this);

}

te::qt::plugins::fiocruz::FlowNetworkDialog::~FlowNetworkDialog()
{

}

void te::qt::plugins::fiocruz::FlowNetworkDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{

}
