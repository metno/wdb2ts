/*
 * CSValues.h
 *
 *  Created on: Sep 12, 2013
 *      Author: borgem
 */

#ifndef __CVS_1_H__
#define __CVS_1_H__

#include <iostream>
#include "SimpleTupleContainer.h"

miutil::container::SimpleTupleContainer*
readCSV( std::istream &in, char sep=';' );

bool
writeCSV( std::ostream &out, const miutil::container::ITupleContainer &container, char sep=';'  );





#endif /* CSVALUES_H_ */
