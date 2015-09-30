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
\file fiocruz/src/fiocruz/AbstractAction.cpp

\brief This file defines the abstract class AbstractAction
*/

#include "Regionalization.h"

#include "terralib/dataaccess/datasource/DataSourceFactory.h"
#include "terralib/dataaccess/datasource/DataSourceTransactor.h"

#include "terralib/dataaccess/query/Select.h"
#include "terralib/dataaccess/query/Field.h"
#include "terralib/dataaccess/query/FromItem.h"
#include "terralib/dataaccess/query/DataSetName.h"
#include "terralib/dataaccess/query/Distinct.h"
#include "terralib/dataaccess/query/Expression.h"
#include "terralib/dataaccess/query/PropertyName.h"

#include "terralib/datatype/StringProperty.h"

#include "terralib/memory/DataSet.h"

te::qt::plugins::fiocruz::Regionalization::Regionalization()
{
}

te::qt::plugins::fiocruz::Regionalization::~Regionalization()
{

}

te::da::DataSetPtr te::qt::plugins::fiocruz::Regionalization::readFile(const std::string& fileName)
{
  std::map<std::string, std::string> connInfo;
  std::auto_ptr<te::da::DataSource> dataSource = te::da::DataSourceFactory::make("OGR");

  connInfo["URI"] = fileName;

  dataSource->setConnectionInfo(connInfo);
  dataSource->open();

  // get a transactor to interact to the data source origin
  std::auto_ptr<te::da::DataSourceTransactor> transactor = dataSource->getTransactor();

  std::vector<std::string> datasets = transactor->getDataSetNames();

  std::auto_ptr<te::da::DataSet> datasetPtr = transactor->getDataSet(datasets[0]);

  te::da::DataSetPtr dataSet(datasetPtr.release());
  return dataSet;
}

te::da::DataSetPtr te::qt::plugins::fiocruz::Regionalization::createMercadoDataSet(const std::string& originColumn, const std::string& destinyColumn, MercadoMap& mercadoMap)
{
  std::string datasettypename;
  te::da::DataSetType* dataSetType = new te::da::DataSetType(datasettypename);

  //first property: measure_id
  te::dt::Property* propertyId = new te::dt::StringProperty(originColumn, te::dt::STRING, 0, true);

  std::string namepk = datasettypename + "_pk";
  te::da::PrimaryKey* primaryKey = new te::da::PrimaryKey(namepk, dataSetType);
  primaryKey->add(propertyId);

  dataSetType->add(propertyId);

  MercadoMap::const_iterator itMercMap = mercadoMap.begin();
  while (itMercMap != mercadoMap.end())
  {
    te::dt::Property* property = new te::dt::SimpleProperty("obj_" + itMercMap->first, te::dt::INT32_TYPE, true);
    dataSetType->add(property);

    ++itMercMap;
  }

  te::da::DataSetPtr dataSet(new te::mem::DataSet(dataSetType));
  return dataSet;
}

bool te::qt::plugins::fiocruz::Regionalization::createMercadoMap(te::da::DataSetPtr dataSet, const std::string& columnOrigin, const std::string& columnDestiny, MercadoMap& mercadoMap)
{
  if (dataSet->moveBeforeFirst() == false)
  {
    return false;
  }

  mercadoMap.clear();

  while (dataSet->moveNext())
  {
    std::string origin = dataSet->getString(columnOrigin);
    std::string destiny = dataSet->getString(columnDestiny);

    MercadoMap::iterator it = mercadoMap.find(destiny);

    if (it == mercadoMap.end())
    {
      //destino ainda nao se encontra no MAP
      OriginMap auxMap;
      auxMap.insert(OriginMap::value_type(origin, 1));
      mercadoMap.insert(MercadoMap::value_type(destiny, auxMap));

    }
    else
    {
      //destino ja esta no MAP
      OriginMap::iterator itInter = it->second.find(origin);

      if (itInter == it->second.end())
      {
        //origem ainda nao se encontra no MAP
        it->second.insert(OriginMap::value_type(origin, 1));
      }
      else
      {
        //origem ja esta no MAP
        ++(itInter->second);
      }
    }
  }

  return true;
}

bool te::qt::plugins::fiocruz::Regionalization::getDistinctObjects(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& columnName, std::vector<std::string>& vecIds)
{
  vecIds.clear();

  te::da::Expression* expression = new te::da::PropertyName(columnName);

  te::da::Distinct* distinct = new te::da::Distinct;
  distinct->push_back(expression);

  te::da::FromItem* fromItem = new te::da::DataSetName(dataSetName);
  te::da::From* from = new te::da::From;
  from->push_back(fromItem);

  te::da::Select select;
  select.setDistinct(distinct);
  select.setFrom(from);

  std::auto_ptr<te::da::DataSet> dataSet = dataSource->query(select);
  if (dataSet->moveBeforeFirst() == false)
  {
    return false;
  }

  while (dataSet->moveNext())
  {
    vecIds.push_back(dataSet->getString(0));
  }

  return true;
}
