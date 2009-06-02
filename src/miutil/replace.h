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
	 * @return a reference to source.
	 */
	std::string& 
	replace(std::string &source, const std::string &what, const std::string &with);
}

#endif /*REPLACE_H_*/
