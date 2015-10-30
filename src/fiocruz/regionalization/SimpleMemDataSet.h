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
\file fiocruz/src/fiocruz/RegionalizationMap.h

\brief This file defines the representation of a Regionalization Map
*/

#include <vector>
#include <string>

namespace te
{
  namespace da
  {
    class DataSetType;
  }

  namespace dt
  {
    class Property;
    class AbstractData;
  }

  namespace mem
  {
    class DataSet;
  }

  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        class ComplexDataSet
        {
          public:
            te::mem::DataSet*     m_dataSet;
            te::da::DataSetType*  m_dataSetType;
        };

        /*!
        \class SimpleMemDataSet

        \brief This class defines a Simple Memory DataSet

        */
        class SimpleMemDataSet
        {

        public:

          typedef std::vector<te::dt::AbstractData*> Row;

        public:

          SimpleMemDataSet(te::da::DataSetType* dataSetType);

          SimpleMemDataSet(const std::string& dataSetName);

          virtual ~SimpleMemDataSet();

          virtual bool addProperty(te::dt::Property* property);

          virtual te::da::DataSetType* getDataSetType() const;

          virtual bool setData(size_t row, size_t column, te::dt::AbstractData* data);

          virtual te::dt::AbstractData* getData(size_t row, size_t column) const;

          virtual void addRow(const Row& row);

          virtual size_t size() const;

          virtual void clear();

          virtual ComplexDataSet convertToDataSet() const;

        protected:
          te::da::DataSetType*  m_dataSetType;

          std::vector<Row> m_vecData;

        };
      }
    }
  }
}
