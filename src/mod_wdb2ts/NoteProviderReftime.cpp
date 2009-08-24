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

#if 0
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#endif

#include <NoteProviderReftime.h>
#include <ptimeutil.h>
#include <splitstr.h>
#include <string>
#include <Logger4cpp.h>

namespace wdb2ts {

using namespace std; 
using namespace miutil;

bool 
NoteProviderReftimes::
saveNote( std::ostream &out )
{
	WEBFW_USE_LOGGER( "handler" );
	try {
		std::ostringstream toLog;

		for( NoteProviderReftimes::iterator it = begin(); it != end(); ++it ) {
			toLog << it->first << "|" << isotimeString( it->second.refTime, true, false )
			    << "|" << isotimeString( it->second.updatedTime, true, false ) 
			    << "|" << it->second.dataversion << endl;
		}
		out << toLog.str();
		WEBFW_LOG_DEBUG("NoteProviderReftimes::saveNote: " << size() << '\n' << toLog.str());
	}
	catch( ... ) {
		return false;
	}
	
	return true;
}

NoteTag* 
NoteProviderReftimes::
loadNote( std::istream &in )
{
	NoteProviderReftimes *note=0;
	vector<string> data;
	boost::posix_time::ptime refTime;
	boost::posix_time::ptime updatedTime;
	string buf;
	int dataversion;
	
	try{
		note = new NoteProviderReftimes();

		while( getline( in, buf ) ) {
			data = splitstr( buf, '|' );
			
			if( data.size() < 3 ) {
				delete note;
				return 0;
			}
			
			refTime = ptimeFromIsoString( data[1] );
			updatedTime = ptimeFromIsoString( data[2] );
			dataversion = -1;
			
			if( data.size() > 3 ) {
				int n;
				
				if( sscanf( data[3].c_str(), "%d", &n ) == 1)
					dataversion = n; 
			}
			
			(*note)[ data[0] ] = ProviderTimes( refTime, updatedTime, dataversion );
		}
	}
	catch( ... ) {
		if( note )
			delete note;
		return 0;
	}
	
	return note;
}


#if 0
bool 
NoteProviderReftimes::
saveNote( std::ostream &out )
{

	try {
		boost::archive::text_oarchive oa( out );
	
		oa & *this;
	}
	catch( ... ) {
		return false;
	}
	
	return true;
}

NoteTag* 
NoteProviderReftimes::
loadNote( std::istream &in )
{
	NoteProviderReftimes *note=0;
	try{
		boost::archive::text_iarchive ia( in );
		note = new NoteProviderReftimes();
		ia & *note;
	}
	catch( ... ) {
		if( note )
			delete note;
		return 0;
	}
	
	return note;
}
#endif

}

