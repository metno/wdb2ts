/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

#ifndef __LOCATIONPOINTMATRIXDATA_H__
#define __LOCATIONPOINTMATRIXDATA_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <pqxx/transactor>
#include <ParamDef.h>
#include <LocationPoint.h>

namespace wdb2ts {


/**
 * Transactor class to fetch provider data from an wdb database.
 * The data is returned in as an PtrProviderRefTimes.
 * 
 */
class  LocationPointMatrixData
: public pqxx::transactor<>
{
public:
   LocationPointMatrixData();
   LocationPointMatrixData( const LocationPointMatrixData &lpmd );
   LocationPointMatrixData(  float latitude, float longitude,
                             const ParamDefList &params,
                             const ParamDef &paramDef,
                             const std::string &provider,
                             const boost::posix_time::ptime &reftimespec,
                             int surroundLevel,
                             int wciProtocol,
                             const std::string &logger="wdb");


   ~LocationPointMatrixData();
   LocationPointMatrixData& operator=( const LocationPointMatrixData &rhs );


   void doLocation( argument_type &t );
   void operator () ( argument_type &t );

   LocationPointMatrixTimeseriePtr result() const { return locations_; }
   std::string query() const { return *query_; }

private:
   float latitude_;
   float longitude_;
   ParamDefList params_;
   ParamDef paramDef_;
   std::string provider_;
   boost::posix_time::ptime reftimespec_;
   int surroundLevel_;
   int wciProtocol_;
   boost::shared_ptr<std::string> query_;
   LocationPointMatrixTimeseriePtr locations_;
   std::string logger_;
};

}

#endif 
