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


#include <iostream>
#include <sstream>
#include <deque>
#include <State.h>

using namespace std;

namespace wdb2ts {
namespace config {

	
State::
State()
{
}

State::
~State()
{
}

	
bool 
State::
top( std::string &val)
{
	if( stack.empty() )
		return false;

	val = stack.front();
	return true;
}
	
bool 
State::
pop( std::string &val )
{
	if( stack.empty() )
		return false;
	
	val = stack.front();
	stack.pop_front();
	path_.erase();
	
	return true;
}

void 
State::
push( const std::string &val )
{
	path_.erase();
	stack.push_front( val );
}

#if 0
std::string 
State::
path()
{
	ostringstream ost;
	
	if( path_.empty() ) {
		for( std::deque<std::string>::reverse_iterator it = stack.rbegin();
			  it != stack.rend();
			  ++it ) {
			ost << "/" << *it; 
		}
		
		path_ = ost.str();
		
		if( path_.empty() )
			path_="/";
	}
	
	return path_;
}
#endif

std::string
State::
path()const
{
   ostringstream ost;

   for( std::deque<std::string>::const_reverse_iterator it = stack.rbegin();
        it != stack.rend();
        ++it ) {
      ost << "/" << *it;
   }

   return  ost.str();
}


bool
State::
operator==( const std::string &rhs ) const
{
   string p = path();


   if( ! rhs.empty() && rhs[0] == '.' ) {
      string end=rhs.substr( 1 );

      if( p.size() < end.size() )
         return false;

      return p.find( end, p.size() - end.size() ) != string::npos;
   } else {
      return rhs==p;
   }
}

}
}
