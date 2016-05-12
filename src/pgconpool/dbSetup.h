/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: dbSetup.h 3235 2010-04-27 10:20:54Z borgem $

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


#ifndef __MIUTIL_PGPOOL_DBSETUP_H__
#define __MIUTIL_PGPOOL_DBSETUP_H__

#include <stdexcept>
#include <float.h>
#include <limits.h>
#include <string>
#include <map>

namespace miutil { 

namespace  pgpool {

class DbDef;

typedef std::map<std::string, DbDef>   DbDefList;
typedef DbDefList::iterator           IDbDefList;
typedef DbDefList::const_iterator    CIDbDefList;



class DbDef {
   std::string host_;
   int         port_;
   std::string dbname_;
   std::string user_;
   std::string password_;
   std::string wciuser_;
   int         minpoolsize_;
   int         maxpoolsize_;
   int         maxusecount_;
   bool        defaultDbDef_;

public:
   DbDef(): 
		   host_("localhost"), port_(0), minpoolsize_( 0 ),
		   maxpoolsize_( 0 ),
		   maxusecount_(-1), defaultDbDef_(false)
      {}
   DbDef(const DbDef &dd ) :
      host_(dd.host_), port_(dd.port_), dbname_(dd.dbname_), user_(dd.user_),
      password_(dd.password_), wciuser_(dd.wciuser_),
      minpoolsize_( dd.minpoolsize_) ,
      maxpoolsize_( dd.maxpoolsize_ ),
      maxusecount_( dd.maxusecount_ ), defaultDbDef_( dd.defaultDbDef_ )
      {}   
      
   DbDef& operator=(const DbDef &rhs ) {
      if( this!= &rhs ) {
         host_    = rhs.host_;
         port_    = rhs.port_;
         dbname_  = rhs.dbname_;
         user_    = rhs.user_;
         password_= rhs.password_;
         wciuser_ = rhs.wciuser_;
         minpoolsize_  = rhs.minpoolsize_;
         maxpoolsize_  = rhs.maxpoolsize_;
         maxusecount_  = rhs.maxusecount_;
         defaultDbDef_ = rhs.defaultDbDef_;
      }
      
      return *this;
   }
      
   std::string host()    const { return host_; }
   int         port()    const { return port_; }
   std::string dbname()  const { return dbname_; }
   std::string user()    const { return user_; }
   std::string password()const { return password_; }
   std::string wciuser() const { return wciuser_; }
   int         minpoolsize() const { return minpoolsize_;}
   int         maxpoolsize() const { return maxpoolsize_;}
   int         usecount() const { return maxusecount_;}

   void        minpoolsize( int poolsize ) { minpoolsize_ = poolsize;}
   void        maxpoolsize( int poolsize ) { maxpoolsize_ = poolsize;}
   void        usecount( int count)  { maxusecount_ = count;}

   bool defaultDbDef()   const { return defaultDbDef_; }   

   friend 
   void 
   parseDbSetup( const std::string &setupToParse,
                 DbDefList         &dbDefs        );
      
   friend std::ostream& operator<< (std::ostream &os, const miutil::pgpool::DbDef &dd);
};


/**
 * @excetion std::logic_error
 */
void 
parseDbSetup( const std::string &setupToParse,
              DbDefList         &dbDefs        );



std::ostream& 
operator<< (std::ostream &os, const miutil::pgpool::DbDef &dd);

      
}
}

#endif /*WDBPARAMSETUP_H_*/
