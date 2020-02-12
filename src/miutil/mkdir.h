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

#ifndef __DNMI_FILE_MKDIR_H__
#define __DNMI_FILE_MKDIR_H__

#include <string>

namespace dnmi{
  namespace file {
    
    /**
     * \addtogroup  fileutil
     *
     * @{
     */
	/** Creates a new directory.
	 * mkdir try to create a new directory in the directory given 
	 * with path. creates parent directory as needed.
	 * 
	 * The directory given with path must exist. If path is not given
	 * current working directory is used.
	 *	
	 * If the newdir directory already exist, true is returned.
	 * 
	 * It try to create the directory with the permissions owner (rwx), group (rwx),
	 * and other(r-x). But it is modified by the users umask.
	 * 
	 * \param newdir The new directory to create.
	 * \path  create the new directories in this directory.
	 * \return true if the newdir exist or was created. And false otherwise.
	 */
	bool mkdir(const std::string &newdir, const std::string &path=std::string());

	/**
	 * @throws std::logic_error on error.
	 */
	void checkAndMkDirs(const std::string &path);

    
    /** @} */
  }
}

#endif
