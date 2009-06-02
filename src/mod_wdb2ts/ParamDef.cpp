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

#include <iostream>
#include <ParamDef.h>
#include <math.h>

namespace {
const std::string defaultProvider("__DEFAULT__");
}

namespace wdb2ts {

using namespace std;

ParamDef::
ParamDef()
{
}

ParamDef::
ParamDef(const std::string &alias,
		   const std::string &valueparametername,
		   const std::string &valueparameterunit,
		   const std::string &levelparametername,
		   int               levelfrom,
		   int               levelto,
		   const std::string &levelunitname,
		   float             scale,
		   float             offset,
		   int               dataversion,
		   Compare           compare,
		   int               compareValue )
	: alias_( alias ),
	  valueparametername_( valueparametername ),
	  valueparameterunit_( valueparameterunit ),
	  levelparametername_( levelparametername ),
	  levelfrom_( levelfrom ),
	  levelto_( levelto ),
	  levelunitname_( levelunitname ),
	  scale_( scale ),
	  offset_( offset ),
	  dataversion_( dataversion ),
	  compare_( compare ),
	  compareValue_( compareValue )
{
}
	
ParamDef::
ParamDef( const ParamDef &paramDef )
	: alias_( paramDef.alias_ ),
	  valueparametername_( paramDef.valueparametername_ ),
	  valueparameterunit_( paramDef.valueparameterunit_ ),
	  levelparametername_( paramDef.levelparametername_ ),
	  levelfrom_( paramDef.levelfrom_ ),
	  levelto_( paramDef.levelto_ ),
	  levelunitname_( paramDef.levelunitname_ ),
	  scale_( paramDef.scale_ ),
	  offset_( paramDef.offset_ ),
	  dataversion_( paramDef.dataversion_ ),
	  compare_( paramDef.compare_ ),
	  compareValue_( paramDef.compareValue_ )
{
}

ParamDef& 
ParamDef::
operator=( const ParamDef &paramDef )
{
	if( &paramDef != this ) {
		alias_              = paramDef.alias_ ;
		valueparametername_ = paramDef.valueparametername_ ;
		valueparameterunit_ = paramDef.valueparameterunit_ ;
		levelparametername_ = paramDef.levelparametername_ ;
		levelfrom_          = paramDef.levelfrom_ ;
		levelto_            = paramDef.levelto_ ;
		levelunitname_      = paramDef.levelunitname_ ;
		scale_              = paramDef.scale_ ;
		offset_             = paramDef.offset_ ;
		dataversion_        = paramDef.dataversion_;
		compare_            = paramDef.compare_;
		compareValue_       = paramDef.compareValue_;

	}
	
	return *this;
}	


bool 
ParamDef::
operator < ( const ParamDef &rhs ) const
{
	if( valueparametername_ < rhs.valueparametername_ && 
	    valueparameterunit_ < rhs.valueparameterunit_ &&
	    levelparametername_ < rhs.levelparametername_ &&
	    levelfrom_ < rhs.levelfrom_                   &&
	    levelto_ < rhs.levelto_                       &&
	    levelunitname_ < rhs.levelunitname_ )
		return true;
	
	return false;
}


bool      
ParamDef::
isNullValue( float value )const
{
	switch( compare_ ) {
	case ParamDef::undef:   return false;
	case ParamDef::lesser:  return value < compareValue_;
	case ParamDef::greater: return value > compareValue_;
	case ParamDef::equal: int r;
	                      r = static_cast<int>( rintf( value ) );
	                      return r == compareValue_;
	default: return false;
	}
	
}

std::ostream& 
operator<<(std::ostream& output, const ParamDef &pd)
{
	string cmp;
	
	switch( pd.compare() ) {
	case ParamDef::undef:   cmp="undef"; break;
	case ParamDef::lesser:  cmp="lt"; break;
	case ParamDef::greater: cmp="gt"; break;
	case ParamDef::equal:   cmp="eq"; break;
	}
	
	output << "ParamDef[alias:'" << pd.alias() << "' vn:'" << pd.valueparametername() << "' vu:'" 
			 << pd.valueparameterunit() << "' ln:'" << pd.levelparametername() << "' lf:'" << pd.levelfrom()
			 << "' lt:'" << pd.levelto() << "' lu:'" << pd.levelunitname() << "' sc:'" << pd.scale()
			 << "' of:'" << pd.offset() << "' dv:'" << pd.dataversion() << "' cmp:'" << cmp << "' cmpV:'" 
			 << pd.compareValue() << "' ]";
	
	return output;
}


bool
findParam( pqxx::result::const_iterator it,
		     ParamDefPtr &paramDef,
		     const ParamDefList &paramsDefs_ )
{
	ParamDefList &paramsDefs=const_cast<ParamDefList&>( paramsDefs_ );
	
	string dataprovider( it.at("dataprovidername").c_str() );
	string valueparametername( it.at("valueparametername").c_str() );
	string valueparameterunit;
	string levelparametername( it.at("levelparametername").c_str() );
	int    levelfrom( it.at("levelfrom").as<int>() );
	int    levelto( it.at("levelto").as<int>() );
	string levelunitname;
	
	if( ! it.at("valueparameterunit").is_null() )
		valueparameterunit = it.at("valueparameterunit").c_str();
	
	if( ! it.at("levelunitname").is_null() )
		levelunitname = it.at("levelunitname").c_str();
	
	ParamDefList::iterator pit = paramsDefs.find( dataprovider );
	
	if( pit == paramsDefs.end() )
		pit = paramsDefs.find( defaultProvider );
	
	if( pit == paramsDefs.end() )
		return false;
	
	do {
		for( paramDef = pit->second.begin();
		     paramDef != pit->second.end();
		     ++paramDef ) {
			if( valueparametername == paramDef->valueparametername() &&
				 valueparameterunit == paramDef->valueparameterunit() &&
			    levelparametername == paramDef->levelparametername() &&
			    levelfrom == paramDef->levelfrom() &&
			    levelto == paramDef->levelto() &&
			    levelunitname == paramDef->levelunitname() )
				return true;
		}
		if( pit->first != defaultProvider )
			pit = paramsDefs.find( defaultProvider );
		else
			pit = paramsDefs.end();
	}while( pit !=  paramsDefs.end() );
	return false;
}

bool
hasParam(  const ParamDefList &paramsDefs, 
		     const std::string &alias,
		     const std::string &provider_ )
{
	std::string provider( provider_ );
	
	if( provider.empty() )
		provider = defaultProvider;
	
	ParamDefList::const_iterator it = paramsDefs.find( provider );
	
 	if( it == paramsDefs.end() )
		return false;

	for( std::list<ParamDef>::const_iterator paramDef = it->second.begin();
	     paramDef != it->second.end();
		  ++paramDef ) {
		if( paramDef->alias() == alias )
			return true;
	}

	return false;
}



bool
findParam( ParamDefPtr &paramDef,
		     const ParamDefList &paramsDefs_, 
		     const std::string &alias,
		     const std::string &provider )
{
	
	ParamDefList &paramsDefs=const_cast<ParamDefList&>( paramsDefs_ );
	ParamDefList::iterator it = paramsDefs.find( provider );
	
 	if( it == paramsDefs.end() )
		it = paramsDefs.find( defaultProvider );

	if( it == paramsDefs.end() )
			return false;
	do {
		for( paramDef = it->second.begin();
	        paramDef != it->second.end();
		     ++paramDef ) {
			if( paramDef->alias() == alias )
				return true;
		}

		if( it->first != defaultProvider )
			it = paramsDefs.find( defaultProvider );
		else
			it = paramsDefs.end();
	}while( it !=  paramsDefs.end() );

	return false;
}


bool 
addParamDef( ParamDefList &paramsDefs, 
		       const ParamDef  &pd,
		       const std::string &provider ) 
{
	
	if( hasParam( paramsDefs, pd.alias(), provider ) )
		return false;

	if( provider.empty() )
		paramsDefs[defaultProvider].push_back( pd );
	else
		paramsDefs[provider].push_back( pd );
	
	return true;
}




} // namespace wdb2ts
