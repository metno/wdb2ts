/*
 * ITupleContainer.cpp
 *
 *  Created on: Feb 8, 2014
 *      Author: borgem
 */
#include <boost/exception/all.hpp>
#include <boost/exception/errinfo_type_info_name.hpp>
#include "ITupleContainer.h"

using namespace std;

namespace miutil {
namespace container {

IIteratorPtr
ITupleContainer::
iteratorPtr()const
{
	return IIteratorPtr( iterator() );
}

//template <>
//boost::posix_time::ptime
//Field::as<boost::posix_time::ptime>() const
//{
//	try {
//		std::cerr << "\nField::as to ptime specialization. '" << val << "'\n" << std::endl;
//		return miutil::ptimeFromIsoString( val );
//	}
//	catch( ... ) {
//		throw boost::bad_lexical_cast();
//	}
//}

//template <>
//boost::posix_time::ptime
//Field::as<float>() const
//{
//	try {
////		std::cerr << "\nField::as to ptime specialization. '" << val << "'\n" << std::endl;
//		return miutil::ptimeFromIsoString( val );
//	}
//	catch( ... ) {
//		throw boost::bad_lexical_cast();
//	}
//}



}
}


