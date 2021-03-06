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
\file fiocruz/src/fiocruz/RasterInterpolate.h

\brief This class represents a raster interpolation
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_RASTERINTERPOLATE_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_RASTERINTERPOLATE_H

#include "KernelInterpolationAlgorithms.h"
#include "Regionalization.h"

#include <string>

namespace te
{
  namespace gm
  {
    class Geometry;
  }

  namespace rst
  {
    class Raster;
  }

  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {

        class ComplexDataSet;

        typedef std::vector<te::gm::Coord2D> CoordVector;
        typedef std::map<std::string, CoordVector > Ocurrencies;
        
        //! Gets the coordinates of all ocurrencies grouping by id. 
        Ocurrencies GetOcurrencies(const ComplexDataSet& ocurrenciesDataDriver, const std::string& destinyIdColumnName, const std::string& xColumnName, const std::string& yColumnName, const std::string& destinyIdFilter);
        Ocurrencies GetOcurrencies(const ComplexDataSet& ocurrenciesDataDriver, const std::string& destinyIdColumnName, const std::string& originLinkColumnName, const ComplexDataSet& originDataDriver, const std::string& originIdColumnName, const std::string& destinyIdFilter);

        te::gm::Geometry* unitePolygonsFromDataSet(const ComplexDataSet& complexDataSet);

        //! Builds a KDTree from a theme with samples: the theme must have a point representation
        /*
        \param complexDataSet       A dataSet containing polygons or points representing the samples
        \param sampleColumnName     Table column with sample values
        \param tree                 The tree to store samples
        \param destSRID             Destination projection
        \return TRUE if the tree is built correct, otherwise returns FALSE
        */
        bool BuildKDTree(const Ocurrencies& ocurrencies, KernelInterpolationAlgorithms::KD_ADAPTATIVE_TREE& tree);

        bool RasterInterpolate(const Ocurrencies& ocurrencies,
          te::rst::Raster* outputRaster, const int& band,
          const KernelInterpolationAlgorithm& algorithm,
          const te::sa::KernelFunctionType& method,
          const size_t& numberOfNeighbors, const double& boxRatio);

        std::vector<std::string> CreateDominancesMaps(std::string path, std::string baseName, std::vector<te::rst::Raster*> rasters, std::vector<te::qt::plugins::fiocruz::DominanceParams> dpVec);

        std::vector<std::string> CreateIndividualRegionalization(std::string path, std::string baseName, std::vector<std::string> objs, std::vector<te::rst::Raster*> rasters);
      }
    }
  }
}

#endif //__FIOCRUZ_INTERNAL_REGIONALIZATION_RASTERINTERPOLATE_H
