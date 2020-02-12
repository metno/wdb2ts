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

#ifndef __TRIMSTR_H__
#define __TRIMSTR_H__

#include <string>

namespace miutil{

  /**
   * \addtogroup miutil
   * @{
   */
typedef enum{TRIMFRONT, TRIMBACK, TRIMBOTH}ETrimStrWhere;

/**
 * \brief trimstr removes leading and trailing whitespace from
 * a string.
 * 
 * It can be specified what is to be considred as whitespace. 
 * Default whitespace is " \t\r\n". 
 * 
 * It can be specified if you only want to remove whitespace from the 
 * front or from the back. Default is both front and back.
 *
 * \param str   The string to trim for leading and trailing whitespace.
 * \param where Trim front, back or both.
 *              TRIMFRONT, trim leading whitespace from the string.
 *              TRIMBACK, trim trailing whitesapece from the string.
 *              TRIMBOTH  trim leading and trailing whitespace form the string.
 * \param trimset What is considred as a whitespace, dafault " \t\r\n". 
 */

void
trimstr(std::string &str, ETrimStrWhere where=TRIMBOTH, 
	const char *trimset=" \t\r\n");

std::string
trimstrCopy(const std::string &str, ETrimStrWhere where=TRIMBOTH,
	const char *trimset=" \t\r\n");


/** @} */ 
}
#endif
