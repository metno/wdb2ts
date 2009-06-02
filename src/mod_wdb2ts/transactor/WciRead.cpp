/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

#include <sstream>
#include <transactor/WciRead.h>
#include <wdb2tsProfiling.h>
#include <ProviderList.h>
#include <ptimeutil.h>
#include <Logger4cpp.h>

DECLARE_MI_PROFILE;



using namespace std;

namespace wdb2ts {

WciReadHelper::	
WciReadHelper()
{
}

WciReadHelper::
~WciReadHelper()
{
}
	
std::string 
WciReadHelper::
id()
{
	return "";
}

void 
WciReadHelper::
clear()
{
}

void 
WciReadHelper::
on_abort( const char msg_[] )throw ()
{
}

WciRead::
WciRead( WciReadHelper *helper_ )
      						
	: helper( helper_ )
{
}

WciRead::
~WciRead()
{
}


void 
WciRead::
on_abort( const char msg_[] )throw () 
{
	helper->on_abort( msg_ );
}

void 
WciRead::
operator () ( argument_type &t )
{
	WDB2TS_USE_LOGGER( "db" );
	USE_MI_PROFILE;
	MARK_ID_MI_PROFILE("WciRead::" + helper->id());
	
	try {
		WDB2TS_LOG_INFO("WciRead::" << helper->id() << "::SQL [" << helper->query() << "]" );

		helper->clear();
		
		MARK_ID_MI_PROFILE( "WciRead::"+helper->id() +"::data"  );
		pqxx::result  res = t.exec( helper->query() );
		MARK_ID_MI_PROFILE( "WciRead::"+helper->id() +"::data" );
				
		MARK_ID_MI_PROFILE( "WciRead::"+helper->id() +"::doRead" );
		helper->doRead( res );
		MARK_ID_MI_PROFILE(  "WciRead::"+helper->id() +"::doRead"  );
	}
	catch( const exception &ex ){
		WDB2TS_LOG_ERROR("EXCEPTION: WciRead::" << helper->id() << ": query [" << helper->query() << "]" 
			              << " reason: " << ex.what() );
		throw;
	}
	
	MARK_ID_MI_PROFILE( "WciRead::" + helper->id() );
}

}

