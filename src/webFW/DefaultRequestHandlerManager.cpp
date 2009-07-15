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
#include <iostream>
#include <sstream>
#include <DefaultRequestHandlerManager.h>
   
using namespace std;

webfw::
DefaultRequestHandlerManager::
Version::
Version( int mymajor, int myminor )
   : verMajor( mymajor ), verMinor( myminor )
{
}

webfw::
DefaultRequestHandlerManager::
Version::
Version( const Version &v )
   : verMajor( v.verMajor ), verMinor( v.verMinor )
{
}

webfw::
DefaultRequestHandlerManager::
Version::
Version()
   : verMajor( 0 ), verMinor( 0 )
{
}
 
bool
webfw::
DefaultRequestHandlerManager::
Version::
operator<(const Version &rhs)const
{
   if( verMajor < rhs.verMajor )
      return true;
   else if( verMajor == rhs.verMajor && verMinor<rhs.verMinor )
      return true;
   else
      return false;
}

webfw::DefaultRequestHandlerManager::Version&
webfw::
DefaultRequestHandlerManager::
Version::
operator=(const Version &rhs)
{
   if( this != &rhs ) {
      verMajor = rhs.verMajor;
      verMinor = rhs.verMinor;
   }
   
   return *this;
}

webfw::
DefaultRequestHandlerManager::
DefaultRequestHandlerManager()
{
}

   
webfw::RequestHandlerPtr 
webfw::
DefaultRequestHandlerManager::
findRequestHandlerPathImpl( const std::string &path, 
                            int major, int minor )
{
#if 0 
   {
   	std::map<std::string, std::map<Version, RequestHandlerPtr> >::iterator it = handlers.begin();
   	cerr << "RequestHandlers: find: <" << path << "> Ver: " << major << "." << minor << endl;
   	for( ; it != handlers.end(); ++it ) {
   		cerr << "   Path: <" << it->first <<">" << endl;
   		
   		for( std::map<Version, RequestHandlerPtr>::iterator hit = it->second.begin();
   		     hit != it->second.end();
   		     ++hit ) {
   				cerr << "      " << hit->second->name() 
   				     << " Version: " << hit->first.verMajor << "." << hit->first.verMinor << endl;
   		}
   	}
   }
#endif
   
   std::map<std::string, std::map<Version, RequestHandlerPtr> >::iterator it = handlers.find( path );
  
   if( it == handlers.end() )
      return RequestHandlerPtr();
    
   std::map<Version, RequestHandlerPtr>::iterator hit = it->second.find( Version( major, minor ) ); 
   
   if( hit != it->second.end() )
      return hit->second;
 
   return RequestHandlerPtr();            
}      

void 
webfw::
DefaultRequestHandlerManager::
addRequestHandler( RequestHandler *reqHandler,
                   const std::string &path )
{
   int major, minor;
   
   reqHandler->version( major, minor );
   miutil::thread::RWWriteLock lock( rwMutex );
   
   
   handlers[path][ Version( major, minor ) ] = RequestHandlerPtr( reqHandler );
}
      
void 
webfw::
DefaultRequestHandlerManager::
removeRequestHandler( const std::string &path, int major, int minor)
{
   miutil::thread::RWWriteLock lock( rwMutex );
 
   std::map<std::string, std::map<Version, RequestHandlerPtr> >::iterator it = handlers.find( path );
  
   if( it == handlers.end() )
      return;

   std::map<Version, RequestHandlerPtr>::iterator hit = it->second.find( Version( major, minor ) ); 
   
   
   if( hit != it->second.end() ) {
      //Need to check if this is a default handler, if so remove
      //the default handler too.
      
      if( !(major == 0 && minor == 0) ) {  //Safty guard
         std::map<Version, RequestHandlerPtr>::iterator defIt = it->second.find( Version( 0, 0 ) );
         
         if( defIt != it->second.end() ) {
            int vMajor, vMinor, defMajor, defMinor;
            
            defIt->second->version( defMajor, defMinor );
            hit->second->version( vMajor, vMinor );
            
            if( vMajor == defMajor && vMinor == defMinor )
               it->second.erase( defIt );
         }
      } 
      
      it->second.erase( hit );
   }
}
 
      
void 
webfw::
DefaultRequestHandlerManager::
setDefaultRequestHandler( const std::string &path, int major, int minor )
{
   miutil::thread::RWWriteLock lock( rwMutex );
   
   RequestHandlerPtr reqHandler = findRequestHandlerPathImpl( path, major, minor );
   
   if( ! reqHandler ) {
      ostringstream ost;
      ost << path << " ( " << major << " , " << minor << " )";
      throw NotFound( ost.str() );
   }
   
   handlers[path][ Version( 0, 0 ) ] = reqHandler;
}

