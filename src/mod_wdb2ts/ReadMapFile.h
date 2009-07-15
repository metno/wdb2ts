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


#ifndef __READMAPFILE_H__
#define __READMAPFILE_H__

#include <iostream>
#include <string>
#include "Map.h"

/**
 * \addtogroup ts2xml
 *
 * @{
 */


/**
 * Reads a DTED geographic file with a height field and returns a Map.
 * The Map instance assume the data is given as an UTM projection for zone 33.
 * The size of the field and resolution for the field is read from the file header.
 * 
 * @param[in] filename The name of the DTED file.
 * @param[out] error. On error this string give an reason of the erreor.
 * @return A pointer to a Map instance on success and 0 on failure. The 
 *         caller is responsible to delete the Map after use.
 */

Map*
readMapFile(const std::string &filename, std::string &error);

/** @} */
#endif 
