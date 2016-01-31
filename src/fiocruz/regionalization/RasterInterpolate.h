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

#include <string>

namespace te
{
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
        Ocurrencies GetOcurrencies(const ComplexDataSet& ocurrenciesDataSet, const std::string& idColumnName, const std::string& xColumnName, const std::string& yColumnName);
        Ocurrencies GetOcurrencies(const ComplexDataSet& ocurrenciesDataSet, const std::string& idColumnName, const ComplexDataSet& originDataSet, const std::string& linkColumnName);


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
          const KernelInterpolationMethod& method,
          const unsigned int& numberOfNeighbors, const double& boxRatio);
      }
    }
  }
}

#endif //__FIOCRUZ_INTERNAL_REGIONALIZATION_RASTERINTERPOLATE_H
