#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "matrix.h"
#include "Map.h"

using namespace std;



Map::
Map(int ncols, int nrows, int cellsize, float xllcorner, float yllcorner, 
    int NoDataValue)
   : ncols_(ncols), nrows_(nrows), cellsize_(cellsize), xllcorner_(xllcorner),
     yllcorner_(yllcorner), NODATA_value_(NoDataValue), matrix(nrows, ncols)
{
   yurcorner_ = yllcorner_ + nrows_ * cellsize_;
   xurcorner_ = xllcorner_ + ncols_ * cellsize_;
   
   pjFrom_ = pj_init_plus( "+proj=longlat +ellps=WGS84" );
   pjTo_ =   pj_init_plus( "+proj=utm +zone=33 +ellps=WGS84" );
}  
    
Map::
~Map()
{
   if ( pjFrom_ )
      pj_free( pjFrom_ );
   
   if ( pjTo_ )
      pj_free( pjTo_ );
}; 



bool 
Map::
altitude( float lat, float lon, int &alt, bool &error )const
{
   double x, y;
   long    iX, iY;
   error=true;
   
   if (!pjFrom_ || !pjTo_ ) 
      return false;
   
   if ( ! latLonToXY( lat, lon, x, y ) ) 
      return false;
 
#if 0
   ostringstream  tmpOst;
   
   tmpOst.setf(ios::floatfield, ios::fixed);
   tmpOst.precision(7);
 
   
   tmpOst << "xllcorner: " << xllcorner_ << " yllcorner: " << yllcorner_ << endl
          << "xurcorner: " << xurcorner_ << " yurcorner: " << yurcorner_ << endl
          << "x: " << x << " y: " << y << endl;
   cout << tmpOst.str();
#endif
      
   if ( x < xllcorner_ || x > xurcorner_ ||
        y < yllcorner_ || y > yurcorner_ ) {
      //We have no data for the cordinate.      
      error = false;
      return false;
   }
   
   iX = ( x - static_cast<double>(xllcorner_) ) / cellsize_;
   iY = nrows_ - ( y - static_cast<double>(yllcorner_) ) / cellsize_;
 
   //tmpOst.str("");     
   //tmpOst << "lat: " << lat << " lon: " << lon << " x: " << x << " y: " << y 
   //       << " iX: " << iX << " iY: " << iY << endl;
   //cout << tmpOst.str();    
        
   if ( iX < 0 || iX > ncols_ ||
        iY < 0 || iY > nrows_    ) 
      return false;
   
   try {
      alt = matrix.get( iY, iX );
   }
   catch ( ... ) {
      return false;
   }
   
   error = false;
         
   return true;
}

std::ostream& 
operator<<(std::ostream &ost, const Map &map)
{
   ostringstream  tmpOst;
   
   tmpOst.setf(ios::floatfield, ios::fixed);
   tmpOst.precision(7);
   
   tmpOst << "Map header"                          << endl
          << "-----------------------------------" << endl
          << "ncols:        " << map.ncols_        << endl
          << "nrows:        " << map.nrows_        << endl
          << "cellsize:     " << map.cellsize_     << endl
          << "xllcorner:    " << map.xllcorner_    << endl
          << "yllcorner:    " << map.yllcorner_    << endl 
          << "NODATA_VALUE: " << map.NODATA_value_ << endl;
   
   
   return ost << tmpOst.str();
}


bool 
Map::
latLonToXY( float lat, float lon, double &x, double &y)const
{
   double alt;
   x = lon * DEG_TO_RAD;
   y = lat * DEG_TO_RAD;
   
   int result = pj_transform( pjFrom_, pjTo_, 1, 1, &x, &y, &alt );
   
   // cerr << "latLonToXY: pj_tranform result. " << result << endl;
   
   return result == 0;
}
