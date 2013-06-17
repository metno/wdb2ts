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

#ifndef __REPLACE_H__
#define __REPLACE_H__

#include <string>

namespace miutil{

  /**
   * @addtogroup miutil
   * @{
   */

	/* @brif replace a substrings in a string with new content.
	 * 
	 * @param source the string to replace content in.
	 * @param what The content to be replaced.
	 * @param with The new content.
	 * @param nToReplace Replace only this number of occurrences of what. A value of 0 means all.
	 * @return a reference to source.
	 */
	std::string& 
	replaceString(std::string &source, const std::string &what, const std::string &with, int nToReplace = 0 );

	/* @brif replace a substrings in a string. The source string is not touched a copy with
	 * replaced content is returned.
	 *
	 *
	 * @param source the string to replace content in.
	 * @param what The content to be replaced.
	 * @param with The new content.
	 * @param nToReplace Replace only this number of occurrences of what. A value of 0 means all.
	 * @return a reference to source.
	 */
	std::string
	replaceStringCopy( const std::string &source, const std::string &what, const std::string &with, int nToReplace = 0 );

}

#endif /*REPLACE_H_*/
