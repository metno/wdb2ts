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
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>

namespace webfw {

class Logger 
{
   public:
      enum LogLevel { Fatal, Error, Warning, Info, Debug, Debug1, Debug2 };
      Logger(){};
      virtual ~Logger(){};
      
      virtual void log( LogLevel ll, const std::string &msg )const=0; 
      
      void fatal( const std::string &msg )const { log( Fatal, msg ); }
      void error( const std::string &msg )const { log( Error, msg ); }
      void warning( const std::string &msg )const { log( Warning, msg ); } 
      void info( const std::string &msg )const { log( Info, msg ); }
      void debug( const std::string &msg )const { log( Debug, msg ); }
      void debug1( const std::string &msg )const { log( Debug1, msg ); }
      void debug2( const std::string &msg )const { log( Debug2, msg ); }
};

}

#endif /*LOGGER_H_*/
