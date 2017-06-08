/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: dbSetup.cc 3235 2010-04-27 10:20:54Z borgem $

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
#include <sstream>
#include <string>
#include "parser.h"
#include "dbSetup.h"

using namespace std;


namespace miutil {
namespace pgpool {



void 
parseDbSetup( const std::string &setupToParse,
              DbDefList         &dbDefs        )
{
   const char *validParam[]={"default", "host", "port", "user", "dbname", "password", "wciuser",
                             "usecount", "poolsize", "minpoolsize","maxpoolsize", 0};

   ostringstream err;
   ostringstream os;
   DbDef dd;
   miutil::pgpool::Sections sections;
   miutil::pgpool::CISections it;
   list<string> sectionsList;
   int iVal;
   string sVal;
   string defaultId;
   int poolsize_ = INT_MIN;
   int maxpoolsize_ = INT_MIN;

   dbDefs.clear();

   if( ! miutil::pgpool::parseKeysInSections( setupToParse, sections, err ) ) {
      os.str("");
      os << "Error parsing the databasesetup: " << err.str();
      throw logic_error( os.str() );   
   }

   /* Debug: print out the parsed parameters.
   for( diWdb::CISections it_=sections.begin(); it_!=sections.end(); ++it_ ) {
      cerr << it_->first << ":" << endl
           << "--------------------------" << endl;
      for( diWdb::Keys::CIKeyVals kit_=it_->second.begin();
           kit_!=it_->second.end();
           ++kit_)
         cerr << "   " << kit_->first << " = " << kit_->second << endl;

      cerr << endl;
   }
    */

   //Check for valid paramters.
   for( it=sections.begin(); it!=sections.end(); ++it ) {
      sectionsList.push_back( it->first );

      for( miutil::pgpool::Keys::CIKeyVals kit=it->second.begin();
            kit!=it->second.end();
            ++kit) {
         int i;
         for( i=0; validParam[i] &&  validParam[i]!=kit->first ; ++i );

         if( ! validParam[i] ) {
            os.str("");
            os << "Unknown key: " << kit->first << ".";
            throw logic_error( os.str() );
         }
      }
   }

   for( list<string>::iterator sit=sectionsList.begin();
         sit != sectionsList.end();
         ++sit ) {
      dd = DbDef(); //clean

      try {         
         if( ! sections[*sit].key( "dbname", sVal ) ) 
            throw logic_error( "Missing mandatory key <dbname>." );

         dd.dbname_ = sVal;
         poolsize_ = INT_MIN;
         maxpoolsize_ = INT_MIN;

         if( sections[*sit].key( "user", sVal ) ) 
            dd.user_ = sVal;

         if(  sections[*sit].key( "wciuser", sVal ) ) 
            dd.wciuser_ = sVal;

         if(  sections[*sit].key( "port", iVal ) ) 
            dd.port_ = iVal;   

         if( sections[*sit].key( "host", sVal ) ) 
            dd.host_ = sVal;   

         if( sections[*sit].key( "password", sVal ) ) 
            dd.password_ = sVal;

         if( sections[*sit].key( "poolsize", iVal ) )
            poolsize_ = iVal;

         if( sections[*sit].key( "minpoolsize", iVal ) )
            dd.minpoolsize_ = iVal;

         if( sections[*sit].key( "maxpoolsize", iVal ) ) {
            dd.maxpoolsize_ = iVal;
            maxpoolsize_ = dd.maxpoolsize_;
         }


         if( sections[*sit].key( "usecount", iVal ) )
            dd.maxusecount_ = iVal;

         if( sections[*sit].key( "default", sVal ) ) {
            if( !sVal.empty() && ( sVal[0]=='t' || sVal[0]=='T' ) ) {
               if( ! defaultId.empty() ) 
                  throw logic_error( "Default wdb id allready set to <" + defaultId + ">!");

               defaultId = *sit;
               dd.defaultDbDef_ = true;
            }
         }

         if( dd.wciuser_.empty() )
            dd.wciuser_ = dd.user_;

         if( poolsize_ != INT_MIN ) {
            cerr << "Use of deprecated <poolsize>. Use maxpoolsize.";

            if( maxpoolsize_ == INT_MIN && poolsize_> -1 ) {
               cerr << " Setting maxpoolsize equal to poolsize <" << poolsize_ << ">." << endl;
               dd.maxpoolsize_ = poolsize_;
            } else {
               cerr << " maxpoolsize is set to <" << dd.maxpoolsize_ << "> ignoring poolsize <" << poolsize_ << ">." << endl;
            }
         }

         dbDefs[*sit] = dd;
      }
      catch( const logic_error &ex ) {
         os.str("");
         os << "Invalid value: " << ex.what() << ".";
         throw logic_error( os.str() );
      }   
   }
}

std::ostream&
operator<< (std::ostream &os, const miutil::pgpool::DbDef &dd)
{
   os << "        host: " << dd.host() << endl
         << "        port: " << dd.port() << endl
         << "      dbname: " << dd.dbname() << endl
         << "        user: " << dd.user() << endl
         << "    password: " << "*******"  /*dd.password()*/ << endl
         << "     wciuser: " << dd.wciuser() << endl
         << " maxpoolsize: " << dd.maxpoolsize() << endl
         << " minpoolsize: " << dd.minpoolsize() << endl
         << "    usecount: " << dd.usecount() << endl
         << "     default: " << (dd.defaultDbDef()?"true":"false") << endl;

   return os;
}

}
}
