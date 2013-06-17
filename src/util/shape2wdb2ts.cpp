#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <ShapeReader.h>
#include <algorithm>

using namespace std;
using namespace boost;


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

   shared_ptr< map<string, string> > polygons;

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

