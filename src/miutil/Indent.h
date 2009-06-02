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

#ifndef __MIUTIL_INDENT_H__
#define __MIUTIL_INDENT_H__

#include <string>

namespace miutil {

/**
 * Helper class to maintain a indenting level. Used to indent
 * xmlcode, etc.
 */
class Indent
{
	int level_;
	int space_;
	
public:
	Indent( const Indent &indent )
		: level_(indent.level_), space_( indent.space_ ) {}
	Indent( int space=3): level_(0), space_ (space) {}
	
	Indent& operator=( const Indent &rhs ) {
		 if( &rhs != this ) {
			 level_ = rhs.level_;
			 space_ = rhs.space_;
		 }
		 return *this;
	}
	
	///Increment the indent level with one.
	void incrementLevel(){ level_++; }
	
	///Decrement the indent level with one.
	void decrementLevel(){ if( level_ > 0 ) level_--; }
	
	///Return a apropriate string of space that represent the indent level.
	std::string spaces() const{ return std::string( space_*level_,' ' ); }
	
};

/**
 * IndentLevel is a helper class for automatic maintaince of the indent
 * level of Indent.
 */
class IndentLevel
{
	Indent &indent_;
public:
	IndentLevel( Indent &indent)
		: indent_( indent ) {
		indent_.incrementLevel();
	}
	
	~IndentLevel() {
		indent_.decrementLevel();
	}
	
	std::string indent(){ return indent_.spaces(); }
};

}

#endif
