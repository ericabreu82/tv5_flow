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

#include <set>

te::qt::plugins::fiocruz::RegionalizationMap::RegionalizationMap()
{
}

te::qt::plugins::fiocruz::RegionalizationMap::~RegionalizationMap()
{
}

bool te::qt::plugins::fiocruz::RegionalizationMap::init(te::da::DataSetPtr dataSet, const std::string& columnOrigin, const std::string& columnDestiny, const std::vector<std::string>& vecFilterDestinyIds)
{
  if (dataSet->moveBeforeFirst() == false)
  {
    return false;
  }

  std::set<std::string> setFilterDestinyIds;
  setFilterDestinyIds.insert(vecFilterDestinyIds.begin(), vecFilterDestinyIds.end());

  m_originMap.clear();

  //for all ocorrencies in the dataset
  while (dataSet->moveNext())
  {
    std::string origin = dataSet->getString(columnOrigin);
    std::string destiny = dataSet->getString(columnDestiny);

    if (origin.empty() || destiny.empty())
      continue;

    if (setFilterDestinyIds.find(destiny) == setFilterDestinyIds.end())
    {
      continue;
    }

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

std::string te::qt::plugins::fiocruz::RegionalizationMap::getDominanceId(const std::string& originId, int minLevel, int maxLevel) const
{
  //we first calculate the total count of occurencies
  OriginMap::const_iterator itOrigin = m_originMap.find(originId);
  if (itOrigin == m_originMap.end())
  {
    return "";
  }

  size_t totalOcurrencies = 0; //all the ocurrencies from the given originId

  //we first count all the occurencies from this origin to all destiny
  const DestinyMap& destinyMap = itOrigin->second;
  DestinyMap::const_iterator itDestiny = destinyMap.begin();
  while (itDestiny != destinyMap.end())
  {
    totalOcurrencies += itDestiny->second;
    ++itDestiny;
  }

  //now, we check if there is any destiny which ocurrencies percentage from this origin is inside the given interval
  //if there are  more then one, we get the one with the highest percentege
  itDestiny = destinyMap.begin();

  std::string destinyId;
  double highestPercentage = 0.;
  while (itDestiny != destinyMap.end())
  {
    double factor = (double)itDestiny->second / (double)totalOcurrencies;
    double percentage = factor * 100.;
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

  return destinyId;
}

size_t te::qt::plugins::fiocruz::RegionalizationMap::getOccurrenciesCount(const std::string& originId, const std::string& destinyId) const
{
  //we first calculate the total count of occurencies
  OriginMap::const_iterator itOrigin = m_originMap.find(originId);
  if (itOrigin == m_originMap.end())
  {
    return 0;
  }

  const DestinyMap& destinyMap = itOrigin->second;
  DestinyMap::const_iterator itDestiny = destinyMap.find(destinyId);
  if (itDestiny == destinyMap.end())
  {
    return 0;
  }

  return itDestiny->second;
}

std::vector<std::string> te::qt::plugins::fiocruz::RegionalizationMap::getOriginIds() const
{
  std::vector<std::string> vecOrigins;

  OriginMap::const_iterator itOrigin = m_originMap.begin();
  while (itOrigin != m_originMap.end())
  {
    vecOrigins.push_back(itOrigin->first);
  }

  return vecOrigins;
}

