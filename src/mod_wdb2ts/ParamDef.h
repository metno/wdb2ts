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



#ifndef __WDB2TS_PARAMDEF_H__
#define __WDB2TS_PARAMDEF_H__

#include <limits.h>
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <list>

namespace wdb2ts {

/**
 * ParamDef defines a parameter as it is recognised from
 * the return value from a wci.read call. It also define an alias 
 * for the paramtere. The alias is a simplified name for the parameter. 
 */

class ParamDef 
{
public:
	typedef enum { undef, greater, lesser, equal } Compare;
	
private:
	std::string alias_;
	std::string valueparametername_;
	std::string valueparameterunit_;
	std::string levelparametername_;
	int         levelfrom_;
	int         levelto_;
	std::string levelunitname_;
	float       scale_;
	float       offset_;
	int         dataversion_;
	Compare     compare_;
	int         compareValue_;
	
public:
	///defines an invalid ParamDef so the ParamDef can be used in a map, set etc.
	ParamDef();
	ParamDef(const std::string &alias,
			   const std::string &valueparametername,
				const std::string &valueparameterunit,
				const std::string &levelparametername,
				int               levelfrom,
				int               levelto,
				const std::string &levelunitname,
				float             scale=1.0f,
				float             offset=0.0f,
				int               dataversion=-1,
				Compare           compare=undef,
				int               compareValue=INT_MAX );
	
	ParamDef( const ParamDef &paramDef );
	ParamDef& operator=( const ParamDef &paramDef );
	
	bool operator < ( const ParamDef &paramDef )const ;
	
	std::string alias()              const { return alias_; }
	std::string valueparametername() const { return valueparametername_; }
	std::string valueparameterunit() const { return valueparameterunit_; }
	std::string levelparametername() const { return levelparametername_; }
	int         levelfrom()          const { return levelfrom_; }
	int         levelto()            const { return levelto_; }
	std::string levelunitname()      const { return levelunitname_; }
	float       scale()              const { return scale_; }
	float       offset()             const { return offset_; }
	int         dataversion()        const { return dataversion_; }
	Compare     compare()            const { return compare_; }
	int         compareValue()       const { return compareValue_; }
	bool        isNullValue( float value )const;
	
	friend std::ostream& operator<<(std::ostream& output, const ParamDef &pd);
};

std::ostream& operator<<(std::ostream& output, const ParamDef &pd);

typedef std::list<ParamDef>::iterator ParamDefPtr;

typedef std::map<std::string, std::list<ParamDef> > ParamDefList;

bool
findParam( pqxx::result::const_iterator it, 
		     ParamDefPtr &paramDef,
		     const ParamDefList &paramsDefs );


bool
hasParam(  const ParamDefList &paramsDefs, 
		     const std::string &alias,
		     const std::string &provider );

bool
findParam( ParamDefPtr &paramDef,
		     const ParamDefList &paramsDefs,
		     const std::string &alias, const std::string &provider="" );

/**
 * Return false if a paramdef with the same alias allready exist.
 * Return true if the paramdef was added to the paramsDefs.
 */
bool 
addParamDef( ParamDefList &paramsDefs, const ParamDef  &pd,
		       const std::string &provider="" );

}


#endif /*PARAM_H_*/
