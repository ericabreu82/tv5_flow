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
\file fiocruz/src/fiocruz/Utils.h

\brief This file defines common functions for regionalization plugin
*/

#ifndef __FIOCRUZ_INTERNAL_UTILS_REGIONALIZATION_H
#define __FIOCRUZ_INTERNAL_UTILS_REGIONALIZATION_H

#include "../Config.h"
#include "Regionalization.h"

// TerraLib
#include <terralib/color/ColorBar.h>
#include <terralib/geometry/Envelope.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/maptools/GroupingItem.h>
#include <terralib/raster/Raster.h>

// STL
#include <memory>
#include <string>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        te::rst::Raster* CreateRaster(const std::string& fileName, te::gm::Envelope* envelope, double resX, double resY, int srid, int type = te::dt::DOUBLE_TYPE);

        std::vector<te::map::AbstractLayerPtr> CreateRasterIndividualMaps(std::vector<std::string> rastersPath, std::auto_ptr<te::color::ColorBar> cb, int slices = 20, int prec = 15);

        std::vector<te::map::AbstractLayerPtr> CreateRasterDominanceMaps(std::vector<std::string> rastersPath, std::map<std::string, te::map::GroupingItem*> legMap);

        std::vector<te::map::AbstractLayerPtr> CreateVecDominanceMaps(std::string dsId, std::vector<te::qt::plugins::fiocruz::DominanceParams> dpVec, std::map<std::string, te::map::GroupingItem*> legMap);

        std::vector<te::map::AbstractLayerPtr> CreateVecIndividualMaps(std::string dsId, std::vector<std::string> propNames, std::auto_ptr<te::color::ColorBar> cb, int slices, int prec = 1, int attrType = te::dt::INT32_TYPE);
      }
    }
  }
}
#endif //#ifndef __FIOCRUZ_INTERNAL_UTILS_REGIONALIZATION_H