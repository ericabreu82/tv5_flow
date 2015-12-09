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
\file fiocruz/src/fiocruz/regionalization/LegendWizardPage.h

\brief This file defines the Regionalization Legend Wizard Page class
*/

#include "LegendWizardPage.h"
#include "ui_LegendWizardPageForm.h"

// TerraLib
#include <terralib/common/Globals.h>
#include <terralib/common/STLUtils.h>
#include <terralib/color/ColorBar.h>
#include <terralib/color/RGBAColor.h>
#include <terralib/maptools/Grouping.h>
#include <terralib/maptools/GroupingAlgorithms.h>
#include <terralib/maptools/GroupingItem.h>
#include <terralib/qt/widgets/colorbar/ColorBar.h>
#include <terralib/qt/widgets/colorbar/ColorCatalogWidget.h>
#include <terralib/qt/widgets/se/SymbologyPreview.h>
#include <terralib/se/Utils.h>

// Qt
#include <QMessageBox>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

#define NO_TITLE "No Value"

te::qt::plugins::fiocruz::LegendWizardPage::LegendWizardPage(QWidget* parent)
: QWizardPage(parent),
m_ui(new Ui::LegendWizardPageForm)
{
  //setup controls
  m_ui->setupUi(this);

  //add color catalog widget
  QGridLayout* l = new QGridLayout(m_ui->m_widget);
  l->setContentsMargins(0, 0, 0, 0);
  m_colorBar = new  te::qt::widgets::ColorCatalogWidget(m_ui->m_widget);
  l->addWidget(m_colorBar);

  m_colorBar->getColorBar()->setHeight(20);
  m_colorBar->getColorBar()->setScaleVisible(false);

  //configure page
  this->setTitle(tr("Legend"));
  this->setSubTitle(tr("Defines legend for each objects used in regionalization."));

  //connects
  connect(m_ui->m_applyPushButton, SIGNAL(released()), this, SLOT(onApplyPushButtonReleased()));
}

te::qt::plugins::fiocruz::LegendWizardPage::~LegendWizardPage()
{

}

bool te::qt::plugins::fiocruz::LegendWizardPage::isComplete() const
{
  if (m_legend.empty())
    return false;

  return true;
}

void te::qt::plugins::fiocruz::LegendWizardPage::setList(te::qt::plugins::fiocruz::VecStringPair objects)
{
  m_ui->m_listWidget->clear();

  for (std::size_t t = 0; t < objects.size(); ++t)
  {
    m_ui->m_listWidget->addItem(objects[t].first.c_str());
  }
}

std::map<std::string, te::map::GroupingItem*> te::qt::plugins::fiocruz::LegendWizardPage::getLegendMap()
{
  createNullLegend();

  std::map<std::string, te::map::GroupingItem*> map;

  for (std::size_t t = 0; t < m_legend.size(); ++t)
  {
    te::map::GroupingItem* gi = new te::map::GroupingItem(*m_legend[t]);

    map.insert(std::map<std::string, te::map::GroupingItem*>::value_type(gi->getTitle(), gi));
  }

  return map;
}

void te::qt::plugins::fiocruz::LegendWizardPage::onApplyPushButtonReleased()
{
  std::vector<std::string> objects = getObjects();

  buildLegend(objects);

  buildSymbolizer();

  updateLegend();

  emit completeChanged();
}

std::vector<std::string> te::qt::plugins::fiocruz::LegendWizardPage::getObjects()
{
  std::vector<std::string> values;

  QList<QListWidgetItem*> list = m_ui->m_listWidget->selectedItems();

  for (int i = 0; i < list.size(); ++i)
    values.push_back(list[i]->text().toStdString());

  return values;
}

void te::qt::plugins::fiocruz::LegendWizardPage::buildLegend(std::vector<std::string>& objects)
{
  te::common::FreeContents(m_legend);
  m_legend.clear();

  for (std::size_t t = 0; t < objects.size(); ++t)
  {
    te::map::GroupingItem* legendItem = new te::map::GroupingItem;
    legendItem->setValue(objects[t]);
    legendItem->setTitle(objects[t]);
    legendItem->setCount(0);

    std::vector<te::se::Symbolizer*> symbVec;
    te::se::Symbolizer* s = te::se::CreateSymbolizer(te::gm::PolygonType, "#dddddd");
    symbVec.push_back(s);
    legendItem->setSymbolizers(symbVec);

    m_legend.push_back(legendItem);
  }
}

void te::qt::plugins::fiocruz::LegendWizardPage::buildSymbolizer()
{
  if (m_legend.empty())
    return;

  std::auto_ptr<te::color::ColorBar> cb(m_colorBar->getColorBar()->getColorBar());

  int legendSize = m_legend.size();

  std::vector<te::color::RGBAColor> colorVec;

  colorVec = cb->getSlices(legendSize);

  if (colorVec.size() != m_legend.size())
    return;

  for (size_t t = 0; t < colorVec.size(); ++t)
  {
    std::vector<te::se::Symbolizer*> symbVec;

    te::se::Symbolizer* s = te::se::CreateSymbolizer(te::gm::PolygonType, colorVec[t].getColor());

    symbVec.push_back(s);

    m_legend[t]->setSymbolizers(symbVec);
  }
}

void te::qt::plugins::fiocruz::LegendWizardPage::createNullLegend()
{
  //create null group item
  te::map::GroupingItem* legendItem = new te::map::GroupingItem;
  legendItem->setValue(te::common::Globals::sm_nanStr);
  legendItem->setTitle("No Dominance");
  legendItem->setCount(0);

  std::vector<te::se::Symbolizer*> symbVec;
  te::se::Symbolizer* s = te::se::CreateSymbolizer(te::gm::PolygonType, "#dddddd");
  symbVec.push_back(s);
  legendItem->setSymbolizers(symbVec);

  m_legend.push_back(legendItem);
}

void te::qt::plugins::fiocruz::LegendWizardPage::updateLegend()
{
  m_ui->m_tableWidget->setRowCount(0);

  for (std::size_t t = 0; t < m_legend.size(); ++t)
  {
    te::map::GroupingItem* gi = m_legend[t];

    int newrow = m_ui->m_tableWidget->rowCount();
    m_ui->m_tableWidget->insertRow(newrow);

    //symbol
    {
      const std::vector<te::se::Symbolizer*>& ss = gi->getSymbolizers();
      QPixmap pix = te::qt::widgets::SymbologyPreview::build(ss, QSize(24, 24));
      QIcon icon(pix);
      QTableWidgetItem* item = new QTableWidgetItem(icon, "");
      item->setFlags(Qt::ItemIsEnabled);
      item->setTextAlignment(Qt::AlignCenter);
      m_ui->m_tableWidget->setItem(newrow, 0, item);
    }

    //title
    {
      QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(gi->getTitle()));
      item->setFlags(Qt::ItemIsEnabled);
      m_ui->m_tableWidget->setItem(newrow, 1, item);
    }
  }

  m_ui->m_tableWidget->resizeColumnsToContents();
  m_ui->m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}
