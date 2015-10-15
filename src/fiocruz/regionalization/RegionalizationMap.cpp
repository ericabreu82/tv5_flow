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

#include "RegionalizationMap.h"

te::qt::plugins::fiocruz::RegionalizationMap::RegionalizationMap()
{
}

te::qt::plugins::fiocruz::RegionalizationMap::~RegionalizationMap()
{
}

bool te::qt::plugins::fiocruz::RegionalizationMap::init(te::da::DataSetPtr dataSet, const std::string& columnOrigin, const std::string& columnDestiny)
{
  if (dataSet->moveBeforeFirst() == false)
  {
    return false;
  }

  m_originMap.clear();

  //for all ocorrencies in the dataset
  while (dataSet->moveNext())
  {
    std::string origin = dataSet->getString(columnOrigin);
    std::string destiny = dataSet->getString(columnDestiny);

    //we fist check if there is already any occurrency from the current origin
    OriginMap::iterator itOrigin = m_originMap.find(origin);
    if (itOrigin == m_originMap.end())
    {
      //if there isnt, we register the first occurrency from the current origin to the current destination 
      DestinyMap destinyMap;
      destinyMap.insert(DestinyMap::value_type(destiny, 1));
      m_originMap.insert(OriginMap::value_type(origin, destinyMap));
    }
    else
    {
      //if there is, we just add the occurrency from the current origin to the current destination 
      DestinyMap::iterator itDestiny = itOrigin->second.find(destiny);

      if (itDestiny == itOrigin->second.end())
      {
        //origem ainda nao se encontra no MAP
        itOrigin->second.insert(DestinyMap::value_type(destiny, 1));
      }
      else
      {
        //origem ja esta no MAP
        ++(itDestiny->second);
      }
    }
  }

  return true;
}

bool te::qt::plugins::fiocruz::RegionalizationMap::getDominanceId(const std::string& originId, int minLevel, int maxLevel, std::string& destinyId)
{
  //we first calculate the total count of occurencies
  OriginMap::iterator itOrigin = m_originMap.find(originId);
  if (itOrigin == m_originMap.end())
  {
    return false;
  }

  size_t totalOcurrencies = 0; //all the ocurrencies from the given originId

  //we first count all the occurencies from this origin to all destiny
  DestinyMap& destinyMap = itOrigin->second;
  DestinyMap::iterator itDestiny = destinyMap.begin();
  while (itDestiny != destinyMap.end())
  {
    totalOcurrencies += itDestiny->second;
    ++itDestiny;
  }

  //now, we check if there is any destiny which ocurrencies percentage from this origin is inside the given interval
  //if there are  more then one, we get the one with the highest percentege
  itDestiny = destinyMap.begin();

  double highestPercentage = 0.;
  while (itDestiny != destinyMap.end())
  {
    double percentage = (double)totalOcurrencies / (double)itDestiny->second;
    if (percentage >= minLevel && percentage <= maxLevel)
    {
      if (percentage > highestPercentage)
      {
        destinyId = itDestiny->first;
        highestPercentage = percentage;
      }
    }

    ++itDestiny;
  }

  return true;
}
