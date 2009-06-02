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
