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
  \file fiocruz/src/fiocruz/regionalization/RegionalizationVectorAction.cpp

  \brief This file defines the Regionalization Vector Action class
*/

// Fiocruz
#include "qt/RegionalizationWizard.h"
#include "RegionalizationVectorAction.h"

// Terralib
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

te::qt::plugins::fiocruz::RegionalizationVectorAction::RegionalizationVectorAction(QMenu* menu) :te::qt::plugins::fiocruz::AbstractAction(menu)
{
  createAction(tr("Vector Map...").toStdString(), "");
}

te::qt::plugins::fiocruz::RegionalizationVectorAction::~RegionalizationVectorAction()
{
}

void te::qt::plugins::fiocruz::RegionalizationVectorAction::onActionActivated(bool checked)
{
  //get input layers
  std::list<te::map::AbstractLayerPtr> list = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  //show interface
  te::qt::plugins::fiocruz::RegionalizationWizard dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow(), te::qt::plugins::fiocruz::Vector_Regionalization);

  dlg.setList(list);

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    std::vector<te::map::AbstractLayerPtr> layersVec = dlg.getOutputLayers();

    for (std::size_t t = 0; t < layersVec.size(); ++t)
    {
      addNewLayer(layersVec[t]);
    }
  }
}
