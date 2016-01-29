/*
  libpuTools - Basic types/algorithms/containers
  
  $Id: puMathAlgo.h 3273 2010-05-18 17:32:21Z dages $

  Copyright (C) 2006 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef __miutil_algorithm_mathalgo_h__
#define __miutil_algorithm_mathalgo_h__

#include <stdlib.h>
#include <math.h>
#include <numeric>
#include <iterator>
#include <limits>
#include <stdexcept>


namespace miutil {
namespace algorithm {

template<typename T>
const T& max(const T &v1, const T &v2, const T &nullValue=std::numeric_limits<T>::max())
{
	if( v1==nullValue || v2==nullValue)
		return nullValue;
	return std::max(v1,v2);
}

template<typename T>
const T& max3(const T &v1, const T &v2, const T &v3, const T &nullValue=std::numeric_limits<T>::max())
{
	if( v1==nullValue || v2==nullValue || v3==nullValue)
		return nullValue;
	return std::max(v1,std::max(v2,v3));
}


template<typename T>
const T& min(const T &v1, const T &v2, const T &nullValue=std::numeric_limits<T>::max())
{
	if( v1==nullValue || v2==nullValue)
		return nullValue;
	return std::min(v1,v2);
}

template<typename T>
const T& min3(const T &v1, const T &v2, const T &v3, const T &nullValue=std::numeric_limits<T>::max())
{
	if( v1==nullValue || v2==nullValue || v3==nullValue)
		return nullValue;
	return std::min(v1,std::min(v2,v3));
}

  // Randomiser
class Random {
public:
  unsigned long operator()(unsigned long max) {
	  unsigned long rval=rand();
	  return rval%max;
  }

  //Return a value between [0,1>.
  float operator()() {
	  return rand()/RAND_MAX;
  }

  void seed(long s) { srand(s); }
};

  // Statistical functions: percentile, mean, and median
  // Works for (at least) vectors, lists and sets of numerical types.
  // The return value is type double.
  // Percentile and median need a functor for sorting.

template<class C, class sorter>
double Percentile(const C& seq, const double p, sorter& sf) {
  C sseq(seq);
  sf(sseq);
  int n=sseq.size();
  double np=p*(n-1);
  typename C::iterator low=sseq.begin(), high=sseq.begin();
  std::advance(low, static_cast<int>(floor(np)));
  std::advance(high, static_cast<int>(ceil(np)));
  return (*high+*low)/2;
}

template<class C, class sorter>
inline double Median(const C& seq, sorter& sf) {
  return Percentile(seq, 0.5, sf);
}

template<class C>
double Mean(const C& seq) {
  double acc=0;
  int n=seq.size();
  acc=std::accumulate(seq.begin(), seq.end(), 0);
  if( n == 0 )
	  throw std::range_error("Zero divide.");
  return acc/n;
}

// class to sum and average values

template<class T>
class Average {
  T sum_;
  T iter_;
  T min_;
  T max_;
public:
  Average()
    : sum_(0),iter_(0),
      min_( std::numeric_limits<T>::max() ),
      max_( -1 * std::numeric_limits<T>::max() ){}

  void operator()( T v ){
	  if( v == std::numeric_limits<T>::max() ||
		  v == std::numeric_limits<T>::min() )
		  return;

	  if( ( sum_ > (std::numeric_limits<T>::max() - v ) ) ||
		  ( sum_ < (-1*std::numeric_limits<T>::max() + v ) ) )
		  throw std::range_error("Over or underflow");

	  ++iter_;
	  if( v < min_ )
		  min_ = v;
	  if( v > max_ )
		  max_ = v;
	  sum_ += v;
  }

  void clear(){
	  sum_ = 0;
	  iter_ = 0;
	  min_ = std::numeric_limits<T>::max();
	  max_ = -1 * std::numeric_limits<T>::max();
  };

  T sum()const {
	  return sum_;
  }

  T sum( T missing_value )const {
	  if( iter_ == 0 )
		  return missing_value;

  	  return sum_;
    }

  T avg()const{
	  if( iter_ == 0 )
		  throw std::range_error( "Zero divide.");
    return sum_/iter_;
  };

  T avg( T missing_value )const{
	  if( iter_ == 0 )
		  return missing_value;
      return sum_/iter_;
    };

  T min() const {
	  if( iter_ == 0 )
		  throw std::range_error( "Count is 0.");
	  return min_;
  }

  T min( T missing_value ) const {
	  if( iter_ == 0 )
		  return missing_value;
	  return min_;
  }


  T max( T missing_value ) const {
	  if( iter_ == 0 )
		  return missing_value;
	  return max_;
  }
  
  int count() const {
	  return iter_;
  }
  
  Average<T>& operator=(const Average<T>& rhs){
	  if( &rhs != this ) {
		  sum_ = rhs.sum_;
		  iter_ = rhs.iter_;
		  min_ = rhs.min_;
		  max_ = rhs.max_;
	  }
	  return *this;
  };
};


template<class T, class Predicate>
class Count {
private:
  Average<T> avarageAbove_;
  Average<T> avarageBelove_;
  Average<T> avarage_;
  Predicate pred;
public:
  Count( Predicate &pred)
    : pred( pred ){}

  void operator()( T v ) {
	  if( v == std::numeric_limits<T>::max() ||
		  v == std::numeric_limits<T>::min() )
		  return;
	  avarage_( v );
	  if( pred( v ) ) {
		  avarageAbove_( v );
	  } else {
		  avarageBelove_( v );
	  }
  }

  void clear(){
	avarageAbove_.clear();
	avarageBelove_.clear();
	avarage_.clear();
  };

  Average<T> averageAbove()const{
	  return avarageAbove_;
  };

  Average<T> averageBelove()const{
  	  return avarageBelove_;
    };

  Average<T> average()const{
	  return avarage_;
  };

  int countAbove() const {
  	  return avarageAbove_.count();
  }

  int countBelove()const {
	  return avarageBelove_.count();
  }

  int count() const {
	  return avarage_.count();
  }

  Count<T, Predicate>& operator=(const Count<T, Predicate>& rhs){
	  if( &rhs != this ) {
		  avarageAbove_ = rhs.avarageAbove_;
		  avarageBelove_ = rhs.avarageBelove_;
		  avarage_ = rhs.avarage_;
		  pred = rhs.pred;
	  }
	  return *this;
  };
};


template<class T> class MinMax {
private:
  T min_;
  T max_;
public:
  MinMax()
    : min_( std::numeric_limits<T>::max() ),
      max_( -1 * std::numeric_limits<T>::max() ){}

  void operator()( T v ) {
	  if( v == std::numeric_limits<T>::max() ||
	  	  v == std::numeric_limits<T>::min() )
		  return;
	  if( v < min_ )
		  min_ = v;
	  if( v > max_ )
		  max_ = v;
  }
  
  void clear(){
	  min_ = std::numeric_limits<T>::max();
	  max_ = -1 * std::numeric_limits<T>::max();
  };

  T min() const {
	  if( min_ ==  std::numeric_limits<T>::max() )
		  throw std::range_error( "No value.");
	  return min_;
  }

  T min( T missing_value ) const {
 	  if( min_ ==  std::numeric_limits<T>::max() )
 		  return missing_value;
 	  return min_;
   }


  T max() const {
	  if( max_ ==  -1 * std::numeric_limits<T>::max() )
		  throw std::range_error( "No value.");
	  return max_;
  }

  T max( T missing_value ) const {
  	  if( max_ ==  -1 * std::numeric_limits<T>::max() )
  		  return missing_value;
  	  return max_;
  }

  bool valid() const {
	  return min_ !=  std::numeric_limits<T>::max();
  }

  MinMax<T>& operator=(const MinMax<T>& rhs){
	  if( &rhs != this ) {
		  min_ = rhs.min_;
		  max_ = rhs.max_;
	  }
	  return *this;
  };
};

}
}
#endif
