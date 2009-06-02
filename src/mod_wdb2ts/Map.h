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
