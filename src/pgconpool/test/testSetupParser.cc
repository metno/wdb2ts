/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: testSetupParser.cc 2937 2009-12-09 16:39:35Z dages $

  Copyright (C) 2006 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no
  
  This file is part of Diana

  Diana is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Diana is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with Diana; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../include/parser.h"

using namespace std;



int
main( int argn, char **argv ) 
{
   string line;
   ostringstream ost;
   
   if( argn < 2 ) {
      cerr << "Missing <file> argument!"<< endl << endl;
      return 1;
   }
      
   ifstream in( argv[1] );

   if( ! in ) {
      cerr << "Can't open file <" << argv[1] << ">!" << endl << endl;
      return 1;
   }
   
   while( getline( in, line ) ) 
      ost << line << endl;
          
   cerr << ost.str() << endl;
   
   
   miutil::pgpool::Sections sections;
   
   if( ! miutil::pgpool::parseKeysInSections( ost.str(), sections, cerr ) ) {
      cerr << "<< Cant parse the setup!" << endl << endl;
      //return 1;
   }
   
   for( miutil::pgpool::CISections it=sections.begin(); it!=sections.end(); ++it ) {
      
      cerr << it->first << ":" << endl
           << "--------------------------" << endl;
      for( miutil::pgpool::Keys::CIKeyVals kit=it->second.begin();
           kit!=it->second.end();
           ++kit)
           cerr << "   " << kit->first << " = " << kit->second << endl;
           
      cerr << endl;
   }
    
}   
