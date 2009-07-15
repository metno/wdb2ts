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

#include "matrix.h"

Matrix::
Matrix(int rows, int cols)
 :nRows(rows), nCols(cols), mat(0)
{
   init(rows, cols);
}

Matrix::
~Matrix()
{
   if (mat)
      delete[] mat;
}
      
bool 
Matrix::
init(int rows, int cols)
{
   if (!mat)
      delete[] mat;

   nRows=rows;
   nCols=cols;
         
   try{
      mat=new int[nRows*nCols];
   }
   catch(...) {
      mat=0;
      return false;
   }
   
   return true;
}      
      
void 
Matrix::
set(int r, int c, int val)
{
   if (!mat)
      return;
      
   if (r>=nRows || c>=nCols)
      throw std::range_error("Index out of range!");
      
   mat[nCols*r+c]=val;
}

int
Matrix::
get(int r, int c) const
{
   if (!mat)
      return 0;
      
   if (r>=nRows || c>=nCols)
      throw std::range_error("Index out of range!");

   return mat[nCols*r+c];
}
   


