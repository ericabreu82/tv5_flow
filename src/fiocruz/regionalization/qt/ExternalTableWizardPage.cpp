/*  Copyright (C) 2008 National Institute For Space Research (INPE) - Brazil.

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
\file fiocruz/src/fiocruz/regionalization/ExternalTableWizardPage.h

\brief This file defines the Regionalization External Table Wizard Page class
*/

#include "ExternalTableWizardPage.h"
#include "ui_ExternalTableWizardPageForm.h"

// Qt
#include <QMessageBox>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::ExternalTableWizardPage::ExternalTableWizardPage(QWidget* parent)
  : QWizardPage(parent),
  m_ui(new Ui::ExternalTableWizardPageForm)
{
  //setup controls
  m_ui->setupUi(this);

  //configure page
  this->setTitle(tr("External Table"));
  this->setSubTitle(tr("Selects the external table to link."));
}

te::qt::plugins::fiocruz::ExternalTableWizardPage::~ExternalTableWizardPage()
{

}

bool te::qt::plugins::fiocruz::ExternalTableWizardPage::isComplete() const
{
  return true;
}

void te::qt::plugins::fiocruz::ExternalTableWizardPage::setList(std::list<te::map::AbstractLayerPtr>& layerList)
{
  
}
