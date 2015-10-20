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
#include "RegionalizationMap.h"

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
#include "terralib/memory/DataSetItem.h"

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
  //creates the dataSet

  std::string datasettypename;
  te::da::DataSetType* dataSetType = new te::da::DataSetType(datasettypename);

  //first property: origin column
  te::dt::Property* propertyId = new te::dt::StringProperty(originColumn, te::dt::STRING, 0, true);

  std::string namepk = datasettypename + "_pk";
  te::da::PrimaryKey* primaryKey = new te::da::PrimaryKey(namepk, dataSetType);
  primaryKey->add(propertyId);

  dataSetType->add(propertyId);


  //other properties: all the possible destinations
  MercadoMap::const_iterator itMercMap = mercadoMap.begin();
  while (itMercMap != mercadoMap.end())
  {
    te::dt::Property* property = new te::dt::SimpleProperty("obj_" + itMercMap->first, te::dt::INT32_TYPE, true);
    dataSetType->add(property);

    ++itMercMap;
  }

  te::mem::DataSet* dataSet = new te::mem::DataSet(dataSetType);

  //populates the dataSet based on the given MercadoMap
  std::vector<std::string> vecDistinctIds;

  for (size_t i = 0; i < vecDistinctIds.size(); i++)
  {
    std::string currentId = vecDistinctIds[i];

    te::mem::DataSetItem* dataSetItem = new te::mem::DataSetItem(dataSet);
    dataSetItem->setString(0, currentId); //Id

    itMercMap = mercadoMap.begin();

    size_t originColumnIndex = 1;
    while (itMercMap != mercadoMap.end())
    {
      int originCount = 0;

      OriginMap::const_iterator itOrigin = itMercMap->second.find(currentId);
      if (itOrigin != itMercMap->second.end())
      {
        originCount = (int)itOrigin->second;
      }
      
      dataSetItem->setInt32(originColumnIndex, originCount); //origin count
      ++originColumnIndex;

      ++itMercMap;
    }

    dataSet->add(dataSetItem);
  }

  te::da::DataSetPtr dataSetPtr(dataSet);
  return dataSetPtr;
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

te::mem::DataSet* te::qt::plugins::fiocruz::Regionalization::cloneDataSet(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& outputDataSetName)
{
  // we first get the information about the input vector DataSet
  std::auto_ptr<te::da::DataSet> inputDataSet = dataSource->getDataSet(dataSetName);
  std::auto_ptr<te::da::DataSetType> inputDataSetType = dataSource->getDataSetType(dataSetName);

  //then we start the creation of the output dataSet by
  //1 - Copying all the columns of the input DataSet to the output dataSet
  te::da::DataSetType* outputDataSetType = new te::da::DataSetType(*inputDataSetType.get());
  outputDataSetType->setName(outputDataSetName);
 
  //we now populate the dataSet
  size_t size = inputDataSetType->size();

  te::mem::DataSet* outputDataSet = new te::mem::DataSet(outputDataSetType);

  inputDataSet->moveBeforeFirst();
  while (inputDataSet->moveNext() == true)
  {
    te::mem::DataSetItem* dataSetItem = new te::mem::DataSetItem(outputDataSet);
    for (size_t i = 0; i < size; ++i)
    {
      dataSetItem->setValue(i, inputDataSet->getValue(i).release());
    }

    outputDataSet->add(dataSetItem);
  }
  
  return outputDataSet;
}

bool te::qt::plugins::fiocruz::Regionalization::addDominanceProperty(te::mem::DataSet* dataSet, const std::string& originColumn, int minLevel, int maxLevel, const std::string& destinyColumn, const RegionalizationMap& regMap, const std::string& newpropertyName)
{
  {
    std::string dominanceColumnName = "dom_primaria";
    te::dt::Property* propertyDominance = new te::dt::StringProperty(dominanceColumnName, te::dt::STRING, 255, false);
    //outputDataSetType->add(propertyDominance);
  }
  {
    std::string dominanceColumnName = "dom_secundaria";
    te::dt::Property* propertyDominance = new te::dt::StringProperty(dominanceColumnName, te::dt::STRING, 255, false);
    //outputDataSetType->add(propertyDominance);
  }
  {
    std::string dominanceColumnName = "dom_terciaria";
    te::dt::Property* propertyDominance = new te::dt::StringProperty(dominanceColumnName, te::dt::STRING, 255, false);
    //outputDataSetType->add(propertyDominance);
    return true;
  }
}

////and then we persist
//std::map<std::string, std::string> mapOptions;
//dataSource->createDataSet(outputDataSetType, mapOptions);
//dataSource->add(outputDataSetName, outputDataSet, mapOptions);