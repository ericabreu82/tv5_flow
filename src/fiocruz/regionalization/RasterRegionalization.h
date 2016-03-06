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
\file fiocruz/src/fiocruz/RasterRegionalization.h

\brief This file defines the algorithm for raster regionalization
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_RASTER_REGIONALIZATION_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_RASTER_REGIONALIZATION_H

#include "../Config.h"

#include "Regionalization.h"

#include <map>
#include <string>
#include <vector>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        class RasterRegionalizationOutputParams
        {
        public:

          RasterRegionalizationOutputParams()
          {
          }

          virtual ~RasterRegionalizationOutputParams()
          {

          }

        public:

          std::string               m_path;
          std::string               m_baseName;
        };


        /*!
        \class RasterRegionalization

        \brief This is an class that defines a algorithm for raster regionalization

        */
        class RasterRegionalization
        {

        public:

          RasterRegionalization();

          virtual ~RasterRegionalization();

            void setInputParameters(RegionalizationInputParams* inParams);

            void setOutputParameters(RasterRegionalizationOutputParams* outParams);

            bool generate(std::vector<std::string>& rastersPath, std::vector<te::rst::Raster*>& rasters);

          protected:

            std::auto_ptr<RegionalizationInputParams> m_inputParams;          //!< Regionalization input parameters.

            std::auto_ptr<RasterRegionalizationOutputParams> m_outputParams;        //!< Regionalization output parameters.
        };
      }
    }
  }
}

#endif //__FIOCRUZ_INTERNAL_REGIONALIZATION_RASTER_REGIONALIZATION_H
