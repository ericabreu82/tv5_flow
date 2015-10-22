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
\file fiocruz/src/fiocruz/AbstractAction.h

\brief This file defines the abstract class AbstractAction
*/

#include "../Config.h"
#include "terralib/dataaccess/dataset/DataSet.h"
#include "terralib/dataaccess/datasource/DataSource.h"
#include "RegionalizationMap.h"

#include <map>
#include <string>

namespace te
{
  namespace mem
  {
    class DataSet;
  }

  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {

        class RegionalizationMapParams
        {
          public:

            te::mem::DataSet*       m_dataSet; //!< The dataSet containing the vector information
            te::da::DataSetType*    m_dataSetType; //!< The dataSet type of the vector dataSet
            std::string             m_originColumn; //!< The name of the origin column
            RegionalizationMap      m_regMap; //!< The regionalization map
        };

        class DominanceParams
        {
          public:
            int m_minLevel;
            int m_maxLevel;
            std::string m_propertyName;
        };

        class DataSetParams
        {
          public:
            te::mem::DataSet*       m_dataSet; //!< The dataSet
            te::da::DataSetType*    m_dataSetType; //!< The dataSet type of the dataSet
        };


        /*!
        \class AbstractAction

        \brief This is an abstract class used to register actions into fiocruz pluging.

        */
        class Regionalization
        {
          typedef std::map<std::string, std::size_t> OriginMap;
          typedef std::map<std::string, OriginMap> MercadoMap;


        public:

          Regionalization();

          virtual ~Regionalization();

          te::da::DataSetPtr readFile(const std::string& fileName);

          te::da::DataSetPtr createMercadoDataSet(const std::string& originColumn, const std::string& destinyColumn, MercadoMap& mercadoMap);

          bool getDistinctObjects(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& columnName, std::vector<std::string>& vecIds);

          DataSetParams cloneDataSet(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& outputDataSetName);

          bool addDominanceProperty(const RegionalizationMapParams& params, const DominanceParams& dominanceParams);

          bool addOcurrenciesProperty(const RegionalizationMapParams& params, const std::string& destinyId, const std::string& newPropertyName);

          bool generate();
        };
      }
    }
  }
}
