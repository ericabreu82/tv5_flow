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

#include "terralib/dataaccess/query/Select.h"
#include "terralib/dataaccess/query/Field.h"
#include "terralib/dataaccess/query/FromItem.h"
#include "terralib/dataaccess/query/DataSetName.h"
#include "terralib/dataaccess/query/Distinct.h"
#include "terralib/dataaccess/query/Expression.h"
#include "terralib/dataaccess/query/PropertyName.h"

te::qt::plugins::fiocruz::Regionalization::Regionalization()
{
}

te::qt::plugins::fiocruz::Regionalization::~Regionalization()
{

}

bool te::qt::plugins::fiocruz::Regionalization::generateMercadoMap(te::da::DataSetPtr dataSet, const std::string& columnOrigin, const std::string& columnDestiny, MercadoMap& mercadoMap)
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

  te::da::Field* field = new te::da::Field(columnName);

  te::da::Fields* fields = new te::da::Fields;
  fields->push_back(field);

  te::da::FromItem* fromItem = new te::da::DataSetName(dataSetName);
  te::da::From* from = new te::da::From;
  from->push_back(fromItem);

  te::da::Select select(fields, from);
  select.setDistinct(distinct);

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
