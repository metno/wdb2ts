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
