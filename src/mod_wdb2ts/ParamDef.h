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
#include <boost/shared_ptr.hpp>
#include <pqxx/pqxx>
#include <string>
#include <list>
#include <ProviderGroups.h>

namespace wdb2ts {
namespace config {
// from <RequestConf.h>, to break cyclisk include.
	struct ParamDefConfig;
}

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
	std::string interpolation;
	///defines an invalid ParamDef so the ParamDef can be used in a map, set etc.
	ParamDef();
	ParamDef( const std::string &alias,
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
	bool hasLevelparameters() const {
		//levelunitname are allowed to be empty so we do not include it in the test.
		return !levelparametername_.empty() && levelfrom_ != INT_MIN &&
				levelto_ != INT_MIN;

	}
	
	std::string alias()              const { return alias_; }
	std::string valueparametername() const { return valueparametername_; }
	std::string valueparameterunit() const { return valueparameterunit_; }
	std::string levelparametername() const { return levelparametername_; }
	void levelparametername( const std::string &name ) { levelparametername_ = name; }
	int         levelfrom()          const { return levelfrom_; }
	void        levelfrom( int f)    { levelfrom_=f; }
	int         levelto()            const { return levelto_; }
	void        levelto( int t)      { levelto_ = t; }
	std::string levelunitname()      const { return levelunitname_; }
	void        levelunitname( const std::string &un) { levelunitname_ = un; }
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

//typedef std::map<std::string, std::list<ParamDef> > ParamDefList;
class ParamDefList : public std::map<std::string, std::list<ParamDef> >
{
   std::list<std::string>   providerListFromConfig;
   ProviderGroups providerGroups_;

public:
   wdb2ts::config::ParamDefConfig* idDefsParams;

   ParamDefList();
   ParamDefList( const ParamDefList &pdl );
   ~ParamDefList();

   ParamDefList operator=(const ParamDefList &rhs );

   void setProviderList( const std::list<std::string> &providerList ){ providerListFromConfig = providerList; }


   bool
   findParam( pqxx::result::const_iterator it,
              ParamDefPtr &paramDef,
              std::string &providerGroup ) const;

   bool
   hasParam(  const std::string &alias,
              const std::string &provider )const;
   bool
   findParam( ParamDefPtr &paramDef,
              const std::string &alias, const std::string &provider="" )const;

/**
 * Return false if a paramdef with the same alias allready exist and replace is false.
 * Return true if the paramdef was added to the paramsDefs.
 */
   bool
   addParamDef( const ParamDef  &pd,
                const std::string &provider="",
                bool replace=false );

   ProviderGroups getProviderGroups() const { return providerGroups_;}
   void setProviderGroups( const ProviderGroups &pg ) { providerGroups_=pg; }
   std::string lookupGroupName( const std::string &providerName )const;
   void resolveProviderGroups( Wdb2TsApp &app, const std::string &wdbid );
   void merge( const ParamDefList *other, bool replace=false );
};

void
renameProvider( std::string &providerWithPlacename, const std::string &newProvider );

typedef boost::shared_ptr<ParamDefList> ParamDefListPtr;
}


#endif /*PARAM_H_*/
