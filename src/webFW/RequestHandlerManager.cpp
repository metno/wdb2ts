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
#include <ctype.h>
#include <splitstr.h>
#include <RequestHandlerManager.h>
#include <iostream>

using namespace std;
using namespace miutil;


namespace {
   bool decodeVersion( const std::string &sVer, int &major, int &minor );
   bool isNumber( const std::string &buf );
}

webfw::
RequestHandlerManager::
RequestHandlerManager()
{
}

void
webfw::
RequestHandlerManager::
decodePath( const std::string &path, 
            std::string &newPath, int &major, int &minor ) 
{
   string myPath( path ); 
   string buf;
   string::size_type i;
   
   major = 0;
   minor = 0;
   
   i = myPath.find_last_of( "/" );
   
   if ( i != string::npos ) {
      if ( (i+1) < myPath.length() ) 
         buf = myPath.substr( i+1 );
      else
         myPath.erase( i );
   }else {
      buf = myPath;
      i = 0;
   }
   
   if( ! buf.empty() && isdigit( buf[0] ) ) {
      if( ! decodeVersion( buf, major, minor ) ) 
         throw InvalidPath( path + "( version: " + buf +")" );
      
      //Remove the version part of the path.
      myPath.erase( i );    
   }
   
   if( myPath.empty() )
      newPath = "/";
   else if( myPath[0] != '/' )
      newPath = "/" + myPath;
   else
      newPath = myPath;
}



webfw::RequestHandlerPtr
webfw:: 
RequestHandlerManager::
findRequestHandlerPath( const std::string &pathIn )
{
   int major, minor;
   string path;
   string origPath;
   string::size_type i;
   
   if ( pathIn.length() == 0 )
      throw InvalidPath( "Empty path!" );
   
   //May throw InvalidPath.
   decodePath( pathIn, path, major, minor );
   
   
   origPath = path;
   
   RequestHandlerPtr req = findRequestHandlerPathImpl( path, major, minor );
   
   while( ! req ) {
      if( path.length() == 1 ) {
      	throw NotFound( origPath );
      }
         
      
      i = path.find_first_of( "/", 1 );
      
      if( i != string::npos ) 
         path.erase( 0, i );
      else
         path.erase();
         
      if( path.empty() ) {
      	throw NotFound( origPath );
      }
               
      req = findRequestHandlerPathImpl( path, major, minor );
   }
   
   if( ! req ) {
      throw NotFound( pathIn );
   }
    
   int major_, minor_;
   
   req->version( major_, minor_ ); 
      
   return req;                 
}




namespace {
  bool decodeVersion( const std::string &sVer, int &major, int &minor )
  {
      string sMajor, sMinor;
      string::size_type i;
      
      if( sVer.empty() ) {
         major = 0;
         minor = 0;
         
         return true;
      }
      
      i=sVer.find(".");
      
      if( i == string::npos ) 
         i = sVer.length();
      
      sMajor = sVer.substr( 0, i );
         
      if( (i+1) < sVer.length() )  
         sMinor = sVer.substr( i+1 );
         
      if( sMinor.find(".") != string::npos )
         return false;   

      if( sMajor.empty() )
         major=0;
      else if( isNumber( sMajor ) )
         major = atoi( sMajor.c_str() );
      else
         return false;
         
      if( sMinor.empty() )
         minor=0;
      else if( isNumber( sMinor ) )
         minor = atoi( sMinor.c_str() );
      else
         return false;

      return true;
  }
  
  bool isNumber( const std::string &buf )
  {
      string::const_iterator it=buf.begin();
      
      for( ; it!=buf.end() && isdigit( *it ); ++it );
      
      return it == buf.end(); 
  }
}
