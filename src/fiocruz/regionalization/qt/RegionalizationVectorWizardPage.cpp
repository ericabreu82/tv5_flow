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
\file fiocruz/src/fiocruz/regionalization/RegionalizationVectorWizardPage.cpp

\brief This file defines the Regionalization Vector Wizard Page class
*/

#include "RegionalizationVectorWizardPage.h"
#include "ui_RegionalizationVectorWizardPageForm.h"

// Qt
#include <QMessageBox>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::RegionalizationVectorWizardPage::RegionalizationVectorWizardPage(QWidget* parent)
: QWizardPage(parent),
m_ui(new Ui::RegionalizationVectorWizardPageForm)
{
  //setup controls
  m_ui->setupUi(this);

  m_ui->m_targetDatasourceToolButton->setIcon(QIcon::fromTheme("datasource"));

  //configure page
  this->setTitle(tr("Regionalization Vector"));
  this->setSubTitle(tr("Reginalization Maps using vectorial representation."));
}

te::qt::plugins::fiocruz::RegionalizationVectorWizardPage::~RegionalizationVectorWizardPage()
{

}

bool te::qt::plugins::fiocruz::RegionalizationVectorWizardPage::isComplete() const
{
  if (m_ui->m_repositoryLineEdit->text().isEmpty())
    return false;

  if (m_ui->m_newLayerNameLineEdit->text().isEmpty())
    return false;

  return true;
}

void te::qt::plugins::fiocruz::RegionalizationVectorWizardPage::setList(std::list<te::map::AbstractLayerPtr>& layerList)
{
}
