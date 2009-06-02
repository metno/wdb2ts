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

/** @} */ 
}
#endif
