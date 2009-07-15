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


#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdexcept>

class Matrix
{
   int nRows;
   int nCols;
   
   int *mat;
   
   public:
      Matrix():nRows(-1), nCols(-1), mat(0){}
      Matrix(int rows, int cols);
      ~Matrix();
     
      bool valid()const { return mat;}
      
      bool init(int rows, int cols);      
      
      
      /**
       * @throws range_error
       */
      void set(int r, int c, int val);
      
      /**
       * @throws range_error
       */
      int  get(int r, int c)const ;
      
      int cols()const { return nCols;}
      int rows()const { return nRows; }
};
   


#endif
