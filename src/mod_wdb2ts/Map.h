/*
    wdb - weather and water data storage

    Copyright (C) 2007 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: wdb@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA
*/


#ifndef __MAP_H__
#define __MAP_H__

#include <iostream>
#include <string>
#include <proj_api.h>

#include "matrix.h"

/**
 * \addtogroup ts2xml
 *
 * @{
 */



/**
 * A class to hold an georaphic field. The field is
 * assumed to be in a UTM projection in zone 33. And 
 * the earth datum is assumed to be WGS 84. The proj
 * library is used to convert from latitude/longitude 
 * coordinates to UTM x/y coordinates.
 */

class Map {
   Map();
   Map(const Map &);
   Map& operator=(const Map &);
   
   int   ncols_;
   int   nrows_;
   float xllcorner_;
   float yllcorner_;
   float xurcorner_;
   float yurcorner_;
   int   cellsize_;
   int   NODATA_value_;
   projPJ pjFrom_;
   projPJ pjTo_;
   
   Matrix matrix;
   
   bool latLonToXY( float lat, float lon, double &x, double &y)const;
   
   protected:
      Map(int ncols, int nrows, int cellsize, float xllcorner, float yllcorner, 
          int NoDataValue);
         
      friend Map *readMapFile(const std::string &filename, std::string &error);
      
   public:
      ~Map();   
   
      int ncols()const { return ncols_;}
      int nrows()const { return nrows_;}
      
      float xllcorner()const{ return xllcorner_;}
      float yllcorner()const{ return yllcorner_;}
      float xurcorner()const{ return xurcorner_;}
      float yurcorner()const{ return yurcorner_;}
      
      int cellsize()const{ return cellsize_;}
      int noDataValue()const { return NODATA_value_;}
      
      int get( int r, int c ) const { return matrix.get(r, c);}
      
      int mapSizeInMBytes()const { return sizeof(int)*ncols_*nrows_/(1024*1024); }
      
      bool altitude( float latitude, float longitude, int &altitude, bool &error )const;
      
      friend std::ostream& operator<<(std::ostream &ost, const Map &map);
};

std::ostream& operator<<(std::ostream &ost, const Map &map);

/** @} */

#endif 
