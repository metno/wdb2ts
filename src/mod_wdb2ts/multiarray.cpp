/*
 * multiarray.cpp
 *
 *  Created on: Apr 27, 2010
 *      Author: borgem
 */




#include <iostream>
#include <boost/multi_array.hpp>
#include <LocationPoint.h>
#include <cassert>

using namespace std;
using namespace wdb2ts;

typedef boost::multi_array<LocationPoint, 2> Matrix;
typedef Matrix::index Index;

int
main( int argn, char **argv )
{


   int level=3;
   int N=level*2;

   Matrix A(boost::extents[N][N]);
/*
   cerr << "1: "<< A.shape()[0] << " " <<  A.shape()[1] << "  " << A.num_dimensions() <<    endl ;
   A.resize( boost::extents[N][N] );
   cerr << "2: "<< A.shape()[0] << " " <<  A.shape()[1] << "  " << A.num_dimensions() <<    endl ;
*/
   for (Matrix::index ii = 0; ii < A.shape()[0]; ++ii)
      for (Matrix::index j = 0; j < A.shape()[1]; ++j) {
         cerr << "A[" << ii << "][" << j << "] = " << A[ii][j].asInt() << endl;
      }

   return 0;

   // Assign values to the elements
   int values = 0;
   for(Index i = 0; i != N; ++i)
     for(Index j = 0; j != N; ++j)
         A[i][j] = LocationPoint( i, j, values++ );

   // Verify values
   int verify = 0;
   for(Index i = 0; i != N; ++i)
     for(Index j = 0; j != N; ++j) {
         assert(A[i][j].asInt() == verify++);
         cerr << "A[" << i << "][" << j << "] = " << A[i][j].asInt() << endl;
       }

   typedef Matrix::index_range range;

   for( Index i=(level-1); i>=0; --i ) {
      int r= 2*level - i; //(level - i)*2 + i;
      cerr << "myview: [" << i << ", " << r << "> level: " << level << endl;

      Matrix::array_view<2>::type myview =
            A[ boost::indices[range(i,r)][range(i,r)]];

      cerr << *myview.shape()  << "  " << myview.num_dimensions() <<    endl ;
      for (Matrix::index ii = 0; ii < myview.shape()[0]; ++ii)
         for (Matrix::index j = 0; j < myview.shape()[1]; ++j) {
            cerr << "myview[" << ii << "][" << j << "] = " << myview[ii][j].asInt()
                 << " " << myview[ii][j].iLatitude() << "/" << myview[ii][j].iLongitude() << endl;
         }
      cerr << endl;
   }
   return 0;
 }

