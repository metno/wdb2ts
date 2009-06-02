#ifndef __COMPRESSPACE_H__
#define __COMPRESSPACE_H__

#include <string>

namespace miutil {
/**
 * cmprspace komprimerer alle space slik at de bare opptar
 * en space. Dersom det er space før \n fjernes disse. Dersom
 * det er space i starten fjernes disse, dette gjelder også alle space før 
 * \n (newline). TAB og CR erstattes med SPACE.
 * Som space regnes SPACE, TAB og CR. Ved retur vil buf kun bestå av
 * ord separert med kun en SPACE og eventuelt \n (newline).
 *
 * Eks.
 *    "dette er  en \t  string\tmed space     \n  hei"
 *    blir komprimert til. "dette er en string med space\nhei"
 */

void 
compresSpace(std::string &buf);

}
#endif 
