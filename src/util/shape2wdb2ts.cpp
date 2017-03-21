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


#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <ShapeReader.h>
#include <algorithm>

using namespace std;
namespace b=boost;


int
main( int argn, char **argv )
{

   if( argn < 2 ) {
      cerr << "use shape2wdb2ts filename "  << endl;
      return 1;
   }

   ifstream fin;

   fin.open( argv[1] );

   if( ! fin.is_open() ) {
      cerr << "Cant open file <" << argv[1] << ">." << endl;
      return 1;
   }

   fin.close();

   b::shared_ptr< map<string, string> > polygons;

   polygons = miutil::ShapeReader( argv[1] );

   cerr << "# polygons: " << polygons->size() << endl;

   if( polygons->size() > 0 ) {
      ostringstream name;
      ofstream out;

      for( map<string, string>::iterator it = polygons->begin();
           it != polygons->end(); ++it ) {
         name.str("");
         name << "mist/Shape"  << it->first << "-"
              << (count( it->second.begin(), it->second.end(), '(' ) - 1 ) << ".polygon";
         out.open( name.str().c_str() );

         if( ! out.is_open() ) {
            cerr << "Cant open file: " << name.str() << endl;
            continue;
         }

         out << it->second << endl;
         out.close();
      }
   }

}

