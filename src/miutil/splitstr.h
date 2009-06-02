#ifndef __SPLITSTR_H__
#define __SPLITSTR_H__

#include <vector>
#include <string>

namespace miutil{

  /**
   * @addtogroup miutil
   * @{
   */

	/* @brif split a string separated with 'saparator'.
	 * 
	 * @param str the string to split.
	 * @param separator Split the string at this separator.
	 * @return a vector of strings.
	 */
	std::vector<std::string> splitstr(const std::string &str, char separator=',');
}

#endif /*SPLITSTR_H_*/
