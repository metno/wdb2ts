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

#include <ReadMapFile.h>
#include <MapLoader.h>
#include <Logger4cpp.h>

using namespace std;

namespace wdb2ts {

void
MapLoader::
operator()()
{
   WEBFW_USE_LOGGER( "main" );
   Map           *hightMap=0;
   string mapfile = app->getMapFilePath();
   string error;

   WEBFW_LOG_DEBUG( "Loading map File: '" <<    mapfile << "'." );
   hightMap = readMapFile( mapfile, error );

   if( hightMap ) {
      WEBFW_LOG_INFO( "Map File: '" <<    mapfile << "' loaded." );
   } else {
      WEBFW_LOG_WARN( "No Map File: '" <<   mapfile << "'." );
   }

   app->setHeightMap( hightMap, true );
}

}

