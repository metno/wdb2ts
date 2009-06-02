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
#ifndef __ENCODECSV_H__
#define __ENCODECSV_H__

#include <string>
#include <WciWebQuery.h>
#include <DbManager.h>

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup mod_wdb2ts 
 * @{
 */
/** @file
 * Definition of the EncodeCSV class
 */
namespace wdb2ts {

/**
 * Encode the result as CSV (Comma separated values).
 */
class EncodeCSV
{
public:
	/// Default Constructor
	EncodeCSV();
	
	/**
	 * Constructor given result set and geographical position
	 * @param	weQuery	  The webQuery decoded from the URL.
	 * @param	connection A database connection.
	 * @param	protocol	  The wci protocol version.
	 */
	EncodeCSV( const WciWebQuery &webQuery, WciConnectionPtr connection, int protocol );
	/// Destructor
	virtual ~EncodeCSV();
    	
	/**
	 * Encode the Result Set as CSV
	 * @param	out		Stream on which the encoded result is returned
	 * @exception	logic_error
	 */
    virtual void encode( std::ostream &out );
      
private:
	///The query from the client.
	WciWebQuery webQuery_;
	
	///The database connection to use.
	WciConnectionPtr connection_;
	
	///The wci protocol version.
	int protocol_;
	
	
};

} // namespace

/**
 *  @} 
 *  @} 
 */

#endif 
