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
   


