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
	 * @param nToReplace Replace only this number of occurs of what. A value of 0 means all.
	 * @return a reference to source.
	 */
	std::string& 
	replace(std::string &source, const std::string &what, const std::string &with, int nToReplace = 0 );
}

#endif /*REPLACE_H_*/
