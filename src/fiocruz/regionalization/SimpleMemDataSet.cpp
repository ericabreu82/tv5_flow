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
\file fiocruz/src/fiocruz/RegionalizationMap.cpp

\brief This class defines the representation of a Regionalization Map
*/

#include "SimpleMemDataSet.h"

#include "terralib/datatype/AbstractData.h"
#include "terralib/dataaccess/dataset/DataSetType.h"
#include "terralib/memory/DataSet.h"
#include "terralib/memory/DataSetItem.h"

te::qt::plugins::fiocruz::SimpleMemDataSet::SimpleMemDataSet(te::da::DataSetType* dataSetType)
  : m_dataSetType(dataSetType)
{

}

te::qt::plugins::fiocruz::SimpleMemDataSet::SimpleMemDataSet(const std::string& dataSetName)
{

}

te::qt::plugins::fiocruz::SimpleMemDataSet::~SimpleMemDataSet()
{
}

bool te::qt::plugins::fiocruz::SimpleMemDataSet::addProperty(te::dt::Property* property)
{
  m_dataSetType->add(property);

  for (size_t i = 0; i < m_vecData.size(); ++i)
  {
    m_vecData[i].resize(m_dataSetType->size());
  }

  return true;
}

te::da::DataSetType* te::qt::plugins::fiocruz::SimpleMemDataSet::getDataSetType() const
{
  return m_dataSetType;
}

bool te::qt::plugins::fiocruz::SimpleMemDataSet::setData(size_t row, size_t column, te::dt::AbstractData* data)
{
  if (m_vecData.size() <= row)
  {
    return false;
  }

  if (m_vecData[row].size() <= column)
  {
    return false;
  }

  m_vecData[row][column] = data;

  return true;
}

te::dt::AbstractData* te::qt::plugins::fiocruz::SimpleMemDataSet::getData(size_t row, size_t column) const
{
  if (m_vecData.size() <= row)
  {
    return 0;
  }

  if (m_vecData[row].size() <= column)
  {
    return 0;
  }

  return m_vecData[row][column];
}

void te::qt::plugins::fiocruz::SimpleMemDataSet::addRow(const Row& row)
{
  m_vecData.push_back(row);
}

size_t te::qt::plugins::fiocruz::SimpleMemDataSet::size() const
{
  return m_vecData.size();
}

void te::qt::plugins::fiocruz::SimpleMemDataSet::clear()
{
  for (size_t i = 0; i < m_vecData.size(); ++i)
  {
    for (size_t j = 0; i < m_vecData[i].size(); ++j)
    {
      delete m_vecData[i][j];
    }
  }

  m_vecData.clear();
}

te::qt::plugins::fiocruz::ComplexDataSet te::qt::plugins::fiocruz::SimpleMemDataSet::convertToDataSet() const
{
  size_t numColumns = m_dataSetType->size();

  te::da::DataSetType* memDataSetType = (te::da::DataSetType*)this->m_dataSetType->clone();
  te::mem::DataSet* memDataSet = new te::mem::DataSet(memDataSetType);
  for (size_t row = 0; row < this->size(); ++row)
  {
    te::mem::DataSetItem* item = new te::mem::DataSetItem(memDataSet);
    for (size_t column = 0; column < numColumns; ++column)
    {
      te::dt::AbstractData* data = this->getData(row, column);

      item->setValue(column, data->clone());
    }
    memDataSet->add(item);
  }

  ComplexDataSet complexDataSet;
  complexDataSet.m_dataSet = memDataSet;
  complexDataSet.m_dataSetType = memDataSetType;

  return complexDataSet;
}
