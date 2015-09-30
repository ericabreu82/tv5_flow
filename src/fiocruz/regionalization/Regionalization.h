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

          bool createMercadoMap(te::da::DataSetPtr dataSet, const std::string& columnOrigin, const std::string& columnDestiny, MercadoMap& mercadoMap);

          bool getDistinctObjects(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& columnName, std::vector<std::string>& vecIds);
        };
      }
    }
  }
}
