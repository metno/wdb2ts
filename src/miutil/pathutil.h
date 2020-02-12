/*
 * pathutil.h
 *
 *  Created on: Sep 14, 2017
 *      Author: borgem
 */

#ifndef SRC_MIUTIL_PATHUTIL_H_
#define SRC_MIUTIL_PATHUTIL_H_


namespace miutil {

std::string dirname(const std::string &path_);
std::string basename(const std::string &path_);
std::string fixPath(const std::string &path_, bool pathSepAtEnd);


}


#endif /* SRC_MIUTIL_PATHUTIL_H_ */
