#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <trimstr.h>
#include <boost/regex.hpp>
#include <sstream>
using namespace std;
using namespace boost;

struct Point {
   string lat;
   string lon;

   Point( const std::string &lat_, const std::string &lon_ )
      : lat( lat_ ), lon( lon_ ) {}
};

int line=0;
int nShapes=0;
const string reFloat = "[-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?";
typedef std::list<Point> Polygon;
typedef std::list<Polygon> Polygons;
typedef std::map<int, Polygons > PolygonMap;

bool decodeShape( istream &in, const string &shape, Polygons &polygons, string &next );
bool decodePolygon( Polygons &polygons, int nVertics, int nParts );

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
         ++line;
      } else {
         break;
      }

      if( buf.empty() )
         continue;

      i = buf.find( "Shape:" );

      if( i == string::npos )
         continue;

      if( decodeShape( fin, buf, polygons, next ) ) {
         allPolygons[ nextPolygon ] = polygons;
         ++nextPolygon;
      }
   }

   cerr << "# polygons: " << allPolygons.size() << endl;
   if( allPolygons.size() > 0 ) {
      ofstream out;
      ostringstream name;
      for( int i=0; i<allPolygons.size(); ++i ) {
         name.str("");
         polygons = allPolygons[i];
         name << "mist/Shape"  << i << "-" << polygons.size() << ".polygon";
         out.open( name.str().c_str() );

         if( ! out.is_open() ) {
            cerr << "Cant open file: " << name.str() << endl;
            continue;
         }


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
         out << ")" << endl;
         out.close();
      }
   }

   cerr << "nShapes: " << nShapes << endl;
}


bool
decodeShape( istream &in, const string &shape, Polygons &polygons, std::string &next )
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
   int shapeAtLine = line;

   next.clear();
   polygons.clear();

   if( !regex_search( shape, match, reShape ) ) {
      cerr << "No match (1): '"<< shape << "'!" << endl;
      return false;
   }

   nVertices = atoi( match[1].str().c_str() );
   nParts = atoi( match[2].str().c_str() );
   ++nShapes;
   cerr << "Shape: nVertices=" << nVertices << " nParts=" << nParts << endl;

   while( getline( in, buf ) ) {
      ++line;
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
          cerr << "No match (2): '"<< buf << "'!" << endl;
          return false;
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

   return cntVertices != 0;
}
