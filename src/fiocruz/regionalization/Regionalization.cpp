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
#include "SimpleMemDataSet.h"

#include "terralib/common/StringUtils.h"

#include "terralib/dataaccess/dataset/DataSetAdapter.h"
#include "terralib/dataaccess/dataset/DataSetTypeConverter.h"
#include "terralib/dataaccess/dataset/ObjectId.h"
#include "terralib/dataaccess/dataset/ObjectIdSet.h"

#include "terralib/dataaccess/datasource/DataSourceFactory.h"
#include "terralib/dataaccess/datasource/DataSourceTransactor.h"

#include "terralib/dataaccess/query/Select.h"
#include "terralib/dataaccess/query/Field.h"
#include "terralib/dataaccess/query/FromItem.h"
#include "terralib/dataaccess/query/DataSetName.h"
#include "terralib/dataaccess/query/Distinct.h"
#include "terralib/dataaccess/query/Expression.h"
#include "terralib/dataaccess/query/PropertyName.h"

#include "terralib/dataaccess/utils/Utils.h"

#include "terralib/datatype/SimpleData.h"
#include "terralib/datatype/StringProperty.h"

#include "terralib/memory/DataSet.h"
#include "terralib/memory/DataSetItem.h"

te::qt::plugins::fiocruz::Regionalization::Regionalization()
{
}

te::qt::plugins::fiocruz::Regionalization::~Regionalization()
{

}

bool te::qt::plugins::fiocruz::Regionalization::generate()
{
  assert(m_inputParams.get());
  assert(m_outputParams.get());

  //input vector data
  te::da::DataSourcePtr iVectorDataSource = m_inputParams->m_iVectorDataSource;
  te::da::DataSetPtr iVectorDataSet = m_inputParams->m_iVectorDataSet;
  const std::string& iVectorDataSetName = m_inputParams->m_iVectorDataSetName;

  const std::string& iVectorColumnOriginId = m_inputParams->m_iVectorColumnOriginId;

  //input tabular data
  te::da::DataSourcePtr iTabularDataSource = m_inputParams->m_iTabularDataSource;
  te::da::DataSetPtr iTabularDataSet = m_inputParams->m_iTabularDataSet;
  const std::string& iTabularDataSetName = m_inputParams->m_iTabularDataSetName;

  const std::string& iTabularColumnOriginId = m_inputParams->m_iTabularColumnOriginId;
  const std::string& iTabularColumnDestinyId = m_inputParams->m_iTabularColumnDestinyId;
  const std::string& iTabularColumnDestinyAlias = m_inputParams->m_iTabularColumnDestinyAlias;

  //dominance params
  const std::vector<DominanceParams>& vecDominance = m_inputParams->m_vecDominance;

  //then we add the occurrences information
  std::vector<std::string> vecIds = m_inputParams->m_objects;

  //output vector data
  te::da::DataSourcePtr oDataSource = m_outputParams->m_oDataSource;
  const std::string& oDataSetName = m_outputParams->m_oDataSetName;
  const std::string& oVectorColumnOriginId = iVectorColumnOriginId;

  //we first calculate the regionalization
  RegionalizationMap regMap;
  regMap.init(iTabularDataSet, iTabularColumnOriginId, iTabularColumnDestinyId);

  //we create the output dataset by cloning the input dataset
  SimpleMemDataSet* simpleDataSet = cloneDataSet(iVectorDataSource, iVectorDataSetName);
  
  //then we add the dominance information
  RegionalizationMapParams regParams;
  regParams.m_simpleDataSet = simpleDataSet;
  regParams.m_originColumn = oVectorColumnOriginId;
  regParams.m_regMap = regMap;

  for (size_t i = 0; i < vecDominance.size(); ++i)
  {
    const DominanceParams& dominanceParams = vecDominance[i];
    addDominanceProperty(regParams, dominanceParams);
  }

  //std::map<std::string, std::string> mapAlias;
  //getAliasMap(iTabularDataSource, iTabularDataSetName, iTabularColumnDestinyId, iTabularColumnDestinyAlias, mapAlias);

  for (size_t i = 0; i < vecIds.size(); ++i)
  {
    const std::string& destinyId = vecIds[i];
    //std::string propertyName = destinyId;
    std::string propertyName = "obj_" + te::common::Convert2String(i); // TEMP
    addOcurrenciesProperty(regParams, destinyId, propertyName);
  }

  ComplexDataSet complexDataSet = simpleDataSet->convertToDataSet();


  //try to save
  //exchange
  te::da::DataSetTypeConverter* converter = new te::da::DataSetTypeConverter(complexDataSet.m_dataSetType, oDataSource->getCapabilities(), oDataSource->getEncoding());

  te::da::DataSetType* dsTypeResult = converter->getResult();

  std::map<std::string, std::string> nopt;

  oDataSource->createDataSet(dsTypeResult, nopt);

  std::auto_ptr<te::da::DataSetAdapter> dsAdapter(te::da::CreateAdapter(complexDataSet.m_dataSet, converter));

  oDataSource->add(dsTypeResult->getName(), dsAdapter.get(), oDataSource->getConnectionInfo());
  //std::map<std::string, std::string> mapOptions;
  //m_outputParams->m_oDataSource->createDataSet(complexDataSet.m_dataSetType, mapOptions);
  //m_outputParams->m_oDataSource->add(complexDataSet.m_dataSetType->getName(), complexDataSet.m_dataSet, mapOptions);

  delete simpleDataSet;

  return true;
}

void te::qt::plugins::fiocruz::Regionalization::setInputParameters(RegionalizationInputParams* inParams)
{
  m_inputParams.reset(inParams);
}

void te::qt::plugins::fiocruz::Regionalization::setOutputParameters(RegionalizationOutputParams* outParams)
{
  m_outputParams.reset(outParams);
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

bool te::qt::plugins::fiocruz::Regionalization::getAliasMap(te::da::DataSourcePtr dataSource, const std::string& dataSetName, const std::string& columnId, const std::string& columnAlias, std::map<std::string, std::string>& mapAlias)
{
  mapAlias.clear();

  //OGR does not support the use of two columns in a distinct clause
  //for this reason, we use a map to discard the repeated values

  te::da::Field* expression1 = new te::da::Field(columnId);
  te::da::Field* expression2 = new te::da::Field(columnAlias);

  te::da::Fields* fields = new te::da::Fields();
  fields->push_back(expression1);
  fields->push_back(expression2);


  te::da::FromItem* fromItem = new te::da::DataSetName(dataSetName);
  te::da::From* from = new te::da::From;
  from->push_back(fromItem);

  te::da::Select select;
  select.fields(fields);
  select.setFrom(from);

  std::auto_ptr<te::da::DataSet> dataSet = dataSource->query(select);
  if (dataSet->moveBeforeFirst() == false)
  {
    return false;
  }

  while (dataSet->moveNext())
  {
    std::string id = dataSet->getString(columnId);
    std::string alias = dataSet->getString(columnAlias);
    mapAlias[id] = alias;
  }
  return true;
}

te::qt::plugins::fiocruz::SimpleMemDataSet* te::qt::plugins::fiocruz::Regionalization::cloneDataSet(te::da::DataSourcePtr dataSource, const std::string& dataSetName) const
{
  // we first get the information about the input vector DataSet
  std::auto_ptr<te::da::DataSet> inputDataSet = dataSource->getDataSet(dataSetName);
  std::auto_ptr<te::da::DataSetType> inputDataSetType = dataSource->getDataSetType(dataSetName);
  size_t numProperties = inputDataSetType->size();

  //we create a simple memory dataSet
  SimpleMemDataSet* simpleDataSet = new SimpleMemDataSet((te::da::DataSetType*)inputDataSetType->clone());

  //then we copy all the data from the input dataSet into the simple memory dataSet
  inputDataSet->moveBeforeFirst();
  while (inputDataSet->moveNext())
  {
    SimpleMemDataSet::Row row;
    for (size_t column = 0; column < numProperties; ++column)
    {
      std::auto_ptr<te::dt::AbstractData> data = inputDataSet->getValue(column);
      row.push_back(data.release());
    }
    simpleDataSet->addRow(row);
  }
  return simpleDataSet;
}

bool te::qt::plugins::fiocruz::Regionalization::addDominanceProperty(const RegionalizationMapParams& params, const DominanceParams& dominanceParams)
{
  SimpleMemDataSet* simpleDataSet = params.m_simpleDataSet;
  const std::string& originColumn = params.m_originColumn;
  const RegionalizationMap& regMap = params.m_regMap;
  int minLevel = dominanceParams.m_minLevel;
  int maxLevel = dominanceParams.m_maxLevel;
  std::string newPropertyName = dominanceParams.m_propertyName;

  //adds the column to the output simple dataSet
  te::dt::Property* propertyDominance = new te::dt::StringProperty(newPropertyName, te::dt::STRING, 255, false);
  simpleDataSet->addProperty(propertyDominance);

  size_t size = simpleDataSet->size();
  size_t originColumnIndex = simpleDataSet->getDataSetType()->getPropertyPosition(originColumn);
  size_t destinyColumnIndex = simpleDataSet->getDataSetType()->getPropertyPosition(newPropertyName);

  for (size_t row = 0; row < size; ++row)
  {
    te::dt::AbstractData* absData = simpleDataSet->getData(row, originColumnIndex);
    std::string originId = absData->toString();

    std::string destinyId = regMap.getDominanceId(originId, minLevel, maxLevel);

    if (destinyId.empty() == false)
    {
      te::dt::AbstractData* newData = new te::dt::String(destinyId);
      simpleDataSet->setData(row, destinyColumnIndex, newData);
    }
  }
 
  return true;
}

bool te::qt::plugins::fiocruz::Regionalization::addOcurrenciesProperty(const RegionalizationMapParams& params, const std::string& destinyId, const std::string& newPropertyName)
{
  SimpleMemDataSet* simpleDataSet = params.m_simpleDataSet;
  const std::string& originColumn = params.m_originColumn;
  const RegionalizationMap& regMap = params.m_regMap;

  te::dt::Property* propertyOccurrencies = new te::dt::SimpleProperty(newPropertyName, te::dt::INT32_TYPE);
  simpleDataSet->addProperty(propertyOccurrencies);

  size_t size = simpleDataSet->size();
  size_t originColumnIndex = simpleDataSet->getDataSetType()->getPropertyPosition(originColumn);
  size_t destinyColumnIndex = simpleDataSet->getDataSetType()->getPropertyPosition(newPropertyName);

  for (size_t row = 0; row < size; ++row)
  {
    te::dt::AbstractData* absData = simpleDataSet->getData(row, originColumnIndex);
    std::string originId = absData->toString();

    size_t count = regMap.getOccurrenciesCount(originId, destinyId);

    te::dt::AbstractData* newData = new te::dt::SimpleData<int, te::dt::INT32_TYPE>((int)count);
    simpleDataSet->setData(row, destinyColumnIndex, newData);
  }

  return true;
}

