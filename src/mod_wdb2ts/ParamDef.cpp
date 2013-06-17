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
#include <RequestConf.h>

namespace {
const std::string defaultProvider("__DEFAULT__");
}

namespace wdb2ts {

using namespace std;

ParamDef::
ParamDef():interpolation( "nearest" )
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
	  compareValue_( compareValue ),
	  interpolation("nearest")
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
	  compareValue_( paramDef.compareValue_ ),
	  interpolation( paramDef.interpolation )
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
		interpolation       = paramDef.interpolation;
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

ParamDefList::
ParamDefList()
	:idDefsParams( 0 )
{
}

ParamDefList::
ParamDefList( const ParamDefList &pdl )
	: idDefsParams( 0 )
{
   providerGroups_ = pdl.providerGroups_;
   providerListFromConfig = pdl.providerListFromConfig;

   if( pdl.idDefsParams )
	   idDefsParams = new wdb2ts::config::ParamDefConfig( *pdl.idDefsParams );

   insert( pdl.begin(), pdl.end() );
}


ParamDefList::
~ParamDefList()
{
	if( idDefsParams )
		delete idDefsParams;
}

ParamDefList
ParamDefList::
operator=(const ParamDefList &rhs )
{
   if( this != &rhs ) {
      providerGroups_ = rhs.providerGroups_;
      providerListFromConfig = rhs.providerListFromConfig;

      if( idDefsParams ) {
    	  delete idDefsParams;
    	  idDefsParams = 0;
      }

      if( rhs.idDefsParams )
    	  idDefsParams = new wdb2ts::config::ParamDefConfig( *rhs.idDefsParams );

      clear();
      insert( rhs.begin(), rhs.end() );
   }
   return *this;
}

bool
ParamDefList::
findParam( pqxx::result::const_iterator it,
		     ParamDefPtr &paramDef,
		     std::string &providerGroup )const
{
	string valueparametername( it.at("valueparametername").c_str() );
	string valueparameterunit;
	string levelparametername( it.at("levelparametername").c_str() );
	int    levelfrom( it.at("levelfrom").as<int>() );
	int    levelto( it.at("levelto").as<int>() );
	string levelunitname;
	
	providerGroup = providerGroups_.lookUpGroupName( it.at("dataprovidername").c_str() );

	if( ! it.at("valueparameterunit").is_null() )
		valueparameterunit = it.at("valueparameterunit").c_str();
	
	if( ! it.at("levelunitname").is_null() )
		levelunitname = it.at("levelunitname").c_str();
	
	ParamDefList::iterator pit = const_cast<ParamDefList*>(this)->find( providerGroup );
	
	if( pit == const_cast<ParamDefList*>(this)->end() )
		pit = const_cast<ParamDefList*>(this)->find( defaultProvider );
	
	if( pit == const_cast<ParamDefList*>(this)->end() )
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
			pit = const_cast<ParamDefList*>(this)->find( defaultProvider );
		else
			pit = const_cast<ParamDefList*>(this)->end();
	}while( pit !=  const_cast<ParamDefList*>(this)->end() );
	return false;
}

bool
ParamDefList::
hasParam(  const std::string &alias,
		     const std::string &provider_ )const
{
	std::string provider( provider_ );
	
	if( provider.empty() )
		provider = defaultProvider;
	
	//provider = providerGroups_.lookUpGroupName( provider );

	ParamDefList::const_iterator it = find( provider );

 	if( it == end() )
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
ParamDefList::
findParam( ParamDefPtr &paramDef,
		     const std::string &alias,
		     const std::string &provider )const
{
   //string provider = providerGroups_.lookUpGroupName( provider_ );

	ParamDefList::iterator it = const_cast<ParamDefList*>(this)->find( provider );
	
 	if( it == const_cast<ParamDefList*>(this)->end() )
		it = const_cast<ParamDefList*>(this)->find( defaultProvider );

	if( it == end() )
			return false;
	do {
		for( paramDef = it->second.begin();
	        paramDef != it->second.end();
		     ++paramDef ) {
			if( paramDef->alias() == alias )
				return true;
		}

		if( it->first != defaultProvider )
			it = const_cast<ParamDefList*>(this)->find( defaultProvider );
		else
			it = const_cast<ParamDefList*>(this)->end();
	}while( it !=  const_cast<ParamDefList*>(this)->end() );

	return false;
}


bool 
ParamDefList::
addParamDef( const ParamDef  &pd,
		     const std::string &provider_,
		     bool replace )
{
	std::string provider( provider_ );

	if( provider.empty() )
		provider = defaultProvider;

	bool hasp=hasParam( pd.alias(), provider );
	
	if( hasp && ! replace )
		return false;

	if( !hasp ) {
		(*this)[provider].push_back( pd );
		return true;
	}

	ParamDefList::iterator it = find( provider );

	//Should never happend. Since we allready has checked this.
	if( it == end() )
		return false;

	for( std::list<ParamDef>::iterator paramDef = it->second.begin();
		 paramDef != it->second.end(); ++paramDef )
	{
		if( paramDef->alias() == pd.alias() ) {
			*paramDef = pd;
			return true;
		}
	}

	//Should never be reached.
	return false;
}

std::string
ParamDefList::
lookupGroupName( const std::string &providerName )const
{
   return providerGroups_.lookUpGroupName( providerName );
}

void
ParamDefList::
resolveProviderGroups( Wdb2TsApp &app, const std::string &wdbid )
{
   providerGroups_.clear();

   providerGroups_.resolve( app, wdbid, providerListFromConfig );
}

void
ParamDefList::
merge( const ParamDefList *other, bool replace )
{
	for( wdb2ts::ParamDefList::const_iterator pit = other->begin();
		 pit != other->end(); ++pit )
	{
		wdb2ts::ParamDefList::iterator pitTmp = find( pit->first );

		//No params is defined for provider pit->first.
		if( pitTmp == end() ) {
			(*this)[ pit->first ] = pit->second;
			continue;
		}

		for( std::list<wdb2ts::ParamDef>::const_iterator itParam = pit->second.begin();
			 itParam != pit->second.end(); ++itParam )
			addParamDef( *itParam, pit->first, replace );
	}
}



void
renameProvider( std::string &providerWithPlacename, const std::string &newProvider ) {
   string::size_type i = providerWithPlacename.find( " [" );

   if( i == string::npos )
      providerWithPlacename = newProvider;
   else
      providerWithPlacename.replace( 0, i, newProvider );
}


} // namespace wdb2ts
