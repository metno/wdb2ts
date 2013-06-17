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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <list>
#include <trimstr.h>
#include <boost/regex.hpp>
#include <sstream>

using namespace std;
using namespace boost;

namespace {

struct Point {
   string lat;
   string lon;

   Point( const std::string &lat_, const std::string &lon_ )
      : lat( lat_ ), lon( lon_ ) {}
};

struct Context {
   int line;
   int nShapes;
   Context() :
      line( 0 ), nShapes( 0 ) {}
};

static const string reFloat = "[-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?";
typedef std::list<Point> Polygon;
typedef std::list<Polygon> Polygons;
typedef std::map<int, Polygons > PolygonMap;

int decodeShape( Context &context, istream &in, const string &shape, Polygons &polygons, string &next );
bool decodePolygon( Context &context, Polygons &polygons, int nVertics, int nParts );

}


namespace miutil {


boost::shared_ptr< std::map<std::string, std::string> >
ShapeReader( const std::string &filename )
{
   shared_ptr< std::map<std::string, std::string> > polygonMap( new std::map<std::string, std::string>() );
   ifstream fin;

   fin.open( filename.c_str() );

   if( ! fin.is_open() )
      throw logic_error("Cant open file <" + filename + ">." );

   Context context;
   string buf;
   string what;
   int nextPolygon=0;
   string::size_type i;
   string next;
   Polygons polygons;
   PolygonMap allPolygons;

   while( true ) {
      if( ! next.empty() )
         buf = next;
      else if(  getline( fin, buf ) ) {
         miutil::trimstr( buf );
         ++context.line;
      } else {
         break;
      }

      if( buf.empty() )
         continue;

      i = buf.find( "Shape:" );

      if( i == string::npos )
         continue;

      if( decodeShape( context, fin, buf, polygons, next ) > 0 ) {
         allPolygons[ nextPolygon ] = polygons;
         ++nextPolygon;
      }
   }

   //cerr << "# polygons: " << allPolygons.size() << endl;
   if( allPolygons.size() > 0 ) {
      ostringstream out;
      ostringstream name;
      for( PolygonMap::size_type i=0; i<allPolygons.size(); ++i ) {
         name.str("");
         polygons = allPolygons[i];
         name << i;

         out.str("");
         out << "POLYGON((";
         bool firstRing=true;
         for( Polygons::const_iterator it=polygons.begin(); it != polygons.end(); ++it ) {
            bool first=true;
            if( ! firstRing )
               out << ", (";
            else
               firstRing = false;

            for( Polygon::const_iterator itPoint=it->begin(); itPoint != it->end(); ++itPoint) {
               if( first ) {
                  out << itPoint->lon << " " << itPoint->lat;
                  first=false;
               } else {
                  out << ", " << itPoint->lon << " " << itPoint->lat;
               }
            }

            out << ")";
         }
         out << ")";

         (*polygonMap)[name.str()] = out.str();
      }
   }

   //cerr << "nShapes: " << context.nShapes << endl;
   return polygonMap;
}

}

namespace {
int
decodeShape( Context &context, istream &in, const string &shape, Polygons &polygons, std::string &next )
{
                       //  Shape:1 (Polygon)  nVertices=111, nParts=1
   static regex reShape("\\s*Shape:1 *\\(Polygon\\) +nVertices=(\\d+)[ ,]+nParts=(\\d+)");
   static regex rePoint("\\+? *\\( *(" + reFloat + "), *(" + reFloat + ").+\\) *(?:Ring)?");
   int nVertices;
   int nParts;
   int cntVertices=0;
   int cntParts=1;
   smatch match;
   string buf;
   string::size_type i;
   Polygon polygon;
   bool inPolygon=false;
   int shapeAtLine = context.line;

   next.clear();
   polygons.clear();

   if( !regex_search( shape, match, reShape ) ) {
      throw logic_error("No match (1): '" + shape + "'!");
   }

   nVertices = atoi( match[1].str().c_str() );
   nParts = atoi( match[2].str().c_str() );
   ++context.nShapes;
   //cerr << "Shape: nVertices=" << nVertices << " nParts=" << nParts << endl;

   while( getline( in, buf ) ) {
      ++context.line;
      miutil::trimstr( buf );

      if( buf.empty() ) {
         if( inPolygon )
            continue;
         break;
      }

      i=buf.find("Bounds:");

      if( i != string::npos )
         continue;

      i=buf.find("to");

      if( i != string::npos )
         continue;

      i=buf.find_first_of("(+");

      if( i == string::npos || i != 0 ) {
         next = buf;
         break;
      }


      inPolygon = true;

      if( buf[i] == '+' ) {
         polygons.push_back( polygon );
         polygon.clear();
         ++cntParts;
      }

      ++cntVertices;

      if( !regex_match( buf, match, rePoint ) ) {
         throw logic_error("No match (2): '"+ buf + "'!" );
      }

      polygon.push_back( Point( match[2], match[1]) );
   }

   if( ! polygon.empty() )
      polygons.push_back( polygon );

   if( cntVertices != nVertices ) {
      cerr << "WARNING: line: " << shapeAtLine << " specified nVertices: " << nVertices
           << " differ from given vertices: " << cntVertices << endl;
   }

   if( nParts != cntParts ) {
      cerr << "WARNING: line: " << shapeAtLine << " specified nParts: " << nParts
           << " differ from given nParts: " << cntParts << endl;
   }

   return cntVertices;
}
}
