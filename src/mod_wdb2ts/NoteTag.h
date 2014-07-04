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


#ifndef __NOTETAG_H__
#define __NOTETAG_H__

#include <string>
#include <iostream>
#include <exception>


#if 0
//bost serialization do not work on etch 64

#include <boost/serialization/utility.hpp>
#include <boost/serialization/is_abstract.hpp>
//#include <boost/serialization/export.hpp>
#endif

namespace wdb2ts {


/**
 * A base class used to tag a note. A note is a simple 
 * communication mechanism uses to comunicate between
 * ActionHandler.
 */
class NoteTag
{
#if 0
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		//NOOP 
	}
#endif
public:
	
	class NoteException : public std::exception {
		std::string msg_;
	public:
		NoteException( const std::string &msg ) : msg_( msg ) {}
		~NoteException() throw() {}

		const char* what()const throw()  { return msg_.c_str(); }
	};
	
	NoteTag():version_(0){}
	virtual ~NoteTag(){}
	virtual const char* noteType() const =0;
	
	virtual bool saveNote( std::ostream &out ){ throw NoteException( "Not implemented!"); }
	
	/**
	 * Return a pointer to a new note on the heap. 
	 */
	virtual NoteTag* loadNote( std::istream &in ){ throw NoteException( "Not implemented!"); };

	void version( long v ){ version_ = v; }
	long version()const{ return version_; }

	private:
		long version_;
};

}

#if 0
BOOST_IS_ABSTRACT( wdb2ts::NoteTag );
//BOOST_CLASS_EXPORT_GUID(wdb2ts::NoteTag, "NoteTag");
#endif

#endif 
