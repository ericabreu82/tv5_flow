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

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATION_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATION_H

#include "../Config.h"
#include "terralib/dataaccess/dataset/DataSet.h"
#include "terralib/dataaccess/dataset/DataSetAdapter.h"
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
        typedef std::pair<std::string, std::string> StringPair;
        typedef std::vector<StringPair> VecStringPair;

        class SimpleMemDataSet;

        class DataSetParams
        {
          public:
            te::da::DataSourcePtr   m_dataSource; //The dataSource
            std::string             m_dataSetName;
        };

        class RegionalizationMapParams
        {
          public:

            SimpleMemDataSet*      m_simpleDataSet; //!< The simple memory dataSet
            std::string            m_originColumn; //!< The name of the origin column
            RegionalizationMap     m_regMap; //!< The regionalization map
        };

        class DominanceParams
        {
          public:
            int m_minLevel;
            int m_maxLevel;
            std::string m_propertyName;
        };

        class RegionalizationInputParams
        {
          public:

            RegionalizationInputParams()
            {
              m_iVectorDataSetName = "";
              m_iVectorColumnOriginId = "";
              m_iTabularDataSetName = "";
              m_iTabularColumnOriginId = "";
              m_iTabularColumnDestinyId = "";
              m_iTabularColumnDestinyAlias = "";
            }

            ~RegionalizationInputParams()
            {

            }

          public:

            //input vector data
            te::da::DataSourcePtr     m_iVectorDataSource;
            te::da::DataSetPtr        m_iVectorDataSet;
            std::string               m_iVectorDataSetName;

            std::string               m_iVectorColumnOriginId;

            //input tabular data
            te::da::DataSourcePtr     m_iTabularDataSource;
            te::da::DataSetPtr        m_iTabularDataSet;
            std::string               m_iTabularDataSetName;

            std::string               m_iTabularColumnOriginId;
            std::string               m_iTabularColumnDestinyId;
            std::string               m_iTabularColumnDestinyAlias;

            //input dominance params
            std::vector<DominanceParams> m_vecDominance;

            //objects
            std::vector<std::string> m_objects;

        };

        class RegionalizationOutputParams
        {
          public:

            RegionalizationOutputParams()
            {
              m_oDataSetName = "";
              m_oVectorColumnOriginId = "";
            }

            ~RegionalizationOutputParams()
            {

            }

          public:

            te::da::DataSourcePtr     m_oDataSource;
            std::string               m_oDataSetName;
            std::string               m_oVectorColumnOriginId;
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

            bool generate();

            void setInputParameters(RegionalizationInputParams* inParams);

            void setOutputParameters(RegionalizationOutputParams* outParams);

            bool getDistinctObjects(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& idColumnName, const std::string& aliasColumnName, VecStringPair& vecIds);

          protected:

            te::da::DataSetPtr readFile(const std::string& fileName);

            te::da::DataSetPtr createMercadoDataSet(const std::string& originColumn, const std::string& destinyColumn, MercadoMap& mercadoMap);

            bool getAliasMap(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& columnId, const std::string& columnAlias, std::map<std::string, std::string>& mapAlias);

            SimpleMemDataSet* cloneDataSet(te::da::DataSourcePtr dataSource, const std::string& dataSetName) const;

            bool addDominanceProperty(const RegionalizationMapParams& params, const DominanceParams& dominanceParams);

            bool addOcurrenciesProperty(const RegionalizationMapParams& params, const std::string& destinyId, const std::string& newPropertyName);

          protected:

            std::auto_ptr<RegionalizationInputParams> m_inputParams;          //!< Regionalization input parameters.

            std::auto_ptr<RegionalizationOutputParams> m_outputParams;        //!< Regionalization output parameters.
        };
      }
    }
  }
}

#endif //__FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATION_H