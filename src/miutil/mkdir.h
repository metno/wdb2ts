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
	 * with path. creates parrent directory as needed.
	 * 
	 * The directory given with path must exist. If path is not given
	 * current working directory is used.
	 *	
	 * If the newdir directory allready exist, true is returned.
	 * 
	 * It try to create the directory with the permisions owner (rwx), group (rwx),
	 * and other(r-x). But it is modified by the users umask.
	 * 
	 * \param newdir The new directory to create.
	 * \path  create the new directoryes in this dierectory.
	 * \return true if the newdir exist or was created. And false otherwise.
	 */
	bool mkdir(const std::string &newdir, const std::string &path=std::string());    
    
    /** @} */
  }
}

#endif
