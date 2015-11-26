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
\file fiocruz/src/fiocruz/RegionalizationMap.h

\brief This file defines the representation of a Regionalization Map
*/

#include "terralib/dataaccess/dataset/DataSet.h"

#include <map>
#include <string>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*!
        \class RegionalizationMap

        \brief This class defines the representation of a Regionalization Map

        */
        class RegionalizationMap
        {
          typedef std::map<std::string, std::size_t> DestinyMap;
          typedef std::map<std::string, DestinyMap> OriginMap;

        public:

          RegionalizationMap();

          virtual ~RegionalizationMap();

          bool init(te::da::DataSetPtr dataSet, const std::string& columnOrigin, const std::string& columnDestiny, const std::vector<std::string>& vecFilterDestinyIds);

          std::string getDominanceId(const std::string& originId, int minLevel, int maxLevel) const;

          size_t getOccurrenciesCount(const std::string& originId, const std::string& destinyId) const;

          std::vector<std::string> getOriginIds() const;

        protected:

          OriginMap m_originMap;

        };
      }
    }
  }
}
