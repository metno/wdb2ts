/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: parser.h 3123 2010-03-04 10:04:58Z borgem $

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

#ifndef __MIUTIL_PGPOOL_PARSER_H__
#define __MIUTIL_PGPOOL_PARSER_H__

#include <string>
#include <map>
#include <list>
#include <stdexcept>

namespace miutil { 

namespace  pgpool {

namespace impl {
   class TokenReader {
      const std::string &buf;
      std::string::size_type next;
      int                    lineCount;
      char comment;
      
      std::string::size_type firstOf(const char *buf, std::string &out, bool &lastWasComment );
      void  skipComment();
      
   public:
      TokenReader( const std::string &buf_, const char commentCh='#' )
      : buf(buf_), next(0), lineCount(0), comment(commentCh) {}
      
      void skip(const char *skipset);      
      bool getToken( std::string &val, char &token, const char *firstOf, bool &lastWasComment );
      int  line() const { return lineCount; }
   };
}

class Keys {
   public:
      typedef std::map<std::string, std::string>                   KeyVals;
      typedef std::map<std::string, std::string>::const_iterator CIKeyVals;
    
   private:
      KeyVals keys;
   
   public:
   
      Keys(){}
      ~Keys(){}
      
      void addKey(const std::string &key, const std::string &value ) {
            keys[key]=value;
      }
      
      /**
       * @return false if the key do not exist or is an empty string.
       *         True otherwise.
       */
      bool key(const std::string &key, std::string &value );

      /**
       * @return false if the key do not exist or the value is an empty string.
       *         True otherwise.
       * @throws std::logic_error when the value cant be converted to an float.
       */
      bool key(const std::string &key, float &value );
      
      /**
       * @return false if the key do not exist or the value is an empty string.
       *         True otherwise.
       * @throws std::logic_error when the value cant be converted to an double.
       */
      bool key(const std::string &key, double &value );
      
      /**
       * @return false if the key do not exist or the value is an empty string.
       *         True otherwise.
       * @throws std::logic_error when the value cant be converted to an int.
       */
      bool key(const std::string &key, int &value );
      
      CIKeyVals begin()const { return keys.begin(); }
      CIKeyVals end()const { return keys.end(); }
      
      std::list<std::string> getKeys()const;
      
};   

typedef std::map<std::string, Keys>   Sections;
typedef Sections::iterator           ISections;
typedef Sections::const_iterator    CISections;


/**
 * Parse sections on the form:
 * 
 * section: key=value key1="a string" ....
 * 
 * \verbatim
   eks: 
      ff: param="* velocity of air (u-component)" leveldomain="above ground" levelfrom=10
      T.2M: param="* tempereatur of air" leveldomain="above ground" levelfrom=2
\endverbatim
*/
 
bool 
parseKeysInSections(const std::string &strToParse, Sections &sections, std::ostream &errors ); 


}

}

#endif
