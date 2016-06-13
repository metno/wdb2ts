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

#include <ctype.h>
#include <MetaModelConf.h>
#include <splitstr.h> 
#include <trimstr.h>
#include <stdio.h>
#include <ProviderList.h>
#include <Logger4cpp.h>
#include <boost/regex.hpp>


namespace b=boost;

namespace wdb2ts {

using namespace boost::posix_time;
using namespace std;

namespace {
boost::posix_time::ptime nowTime__;
string rRelativeToLoadtime=" *\\+?(\\d+)";
string rEvery=" *(\\*|\\d+)/(\\d+)";
}

void MetaModelConf::setNowTimeForTest(const boost::posix_time::ptime &nowTime)
{
	nowTime__=nowTime;
}

MetaModelConf::
MetaModelConf()
	:specType(Undefined)
{
}

MetaModelConf::
MetaModelConf( const std::string &name )
	: name_( name ), specType(Undefined)
{
}

MetaModelConf::
MetaModelConf( const MetaModelConf &m )
	: name_( m.name_ ), specType(m.specType), nextrun_( m.nextrun_ )
{
}
	
MetaModelConf&
MetaModelConf::
operator=( const MetaModelConf &rhs )
{
	if( this != &rhs ) {
		name_ = rhs.name_;
		specType = rhs.specType;
		nextrun_ = rhs.nextrun_;
	}
	
	return *this;
}

	
boost::posix_time::ptime
MetaModelConf::
findNextrun(const boost::posix_time::ptime &refTime)const
{
	ptime now( (nowTime__.is_special()?second_clock::universal_time():nowTime__) );
	ptime midnight( now.date(), time_duration(0, 0, 0, 0) );
	time_duration tdNow( now.time_of_day() );
	

	if( nextrun_.empty() )
		return ptime(); //Udefined.
	
	if( specType==Absolute) {
		TimeDurationList::const_iterator it=nextrun_.begin();
		for( ; it != nextrun_.end() && *it < tdNow ; ++it );

		if( it == nextrun_.end() ) {
			midnight += hours( 24 ); //next day
			it = nextrun_.begin();
		}

		return midnight + *it;
	} else if( specType == RelativeToLoadTime ){
		if( refTime.is_special() || nextrun_.empty())
			return ptime();  // undefined

		return refTime + (*nextrun_.begin());
	} else {
		return ptime();  // Undefined
	}
}

void 
MetaModelConf::
addTimeDuration( const boost::posix_time::time_duration &td, MetaModelConf::SpecType specType)
{
	TimeDurationList::iterator it=nextrun_.begin();
	
	for( ; it != nextrun_.end() && *it < td; ++it );
	
	if( it != nextrun_.end() && *it == td )
		return;
	
	nextrun_.insert( it, td );
}

bool 
MetaModelConf::
isnumber( const std::string &val )
{
	for( string::const_iterator it=val.begin(); 
	     it != val.end(); 
	     ++it )
	{
		
		if( ! isdigit( *it ) ) 
			return false;
	}
	
	return true;
}

MetaModelConf::SpecType
MetaModelConf::parseNextrun1( const std::string &val )
{
	boost::regex reRelativToReftime( rRelativeToLoadtime, boost::regex::perl|boost::regex::icase );
	boost::regex reEvery( rEvery, boost::regex::perl|boost::regex::icase );
	boost::smatch match;

	if( b::regex_match( val, match, reRelativToReftime) ) {
		addTimeDuration( b::posix_time::seconds(b::lexical_cast<int>(match[1])));
		return RelativeToLoadTime;
	} else if( b::regex_match( val, match, reEvery) ) {
		int offset=0;
		int every=b::lexical_cast<int>(match[2]);

		if(string(match[1])[0]!='*')
			offset=b::lexical_cast<int>(match[1]);
		for(int i=offset; i<86400; i+=every)
			addTimeDuration(b::posix_time::seconds(i));

		return Absolute;
	} else {
		return Error;
	}
}

MetaModelConf::SpecType
MetaModelConf::parseNextrun2( const std::string &val1, const std::string &val2 )
{
	using namespace miutil;
	string sHH(val1), sMM(val2);
	int h, m;
	trimstr( sHH );
	trimstr( sMM );

	if( sHH.empty() || sMM.empty() )
		return Error;

	if( ! isnumber( sHH ) || ! isnumber( sMM ) )
		return Error;


	if( sscanf( sHH.c_str(), "%d", &h) != 1 )
		return Error;

	if( sscanf( sMM.c_str(), "%d", &m) != 1 )
		return Error;

	addTimeDuration( time_duration( h, m, 0, 0) );

	return Absolute;
}

bool
MetaModelConf::
parseNextrun( const std::string &val_ )
{
	using namespace miutil;
	WEBFW_USE_LOGGER( "main" );
	SpecType spec;
	vector<string> keyvals = splitstr( val_, ',' );
	
	for( vector<string>::iterator it = keyvals.begin(); 
   	  it != keyvals.end();
        ++it )
	{
		vector<string> timeDurationList = splitstr( *it, ':' );
		if( timeDurationList.size() == 2 )
			spec=parseNextrun2(timeDurationList[0], timeDurationList[1]);
		else if( timeDurationList.size() == 1 )
			spec=parseNextrun1(timeDurationList[0]);
		else {
			continue;
		}


		if( spec==Error) {
			WEBFW_LOG_ERROR( "meta-model: Spec type: 'HH:MM' can not be mixed with '+ss' or '*/ss'. (" << val_ << ")." );
			return false;
		}

		if( specType==Undefined)
			specType = spec;
		else if( specType == Absolute && spec != specType) {
			WEBFW_LOG_ERROR( "meta-model: Spec type: 'HH:MM' can not be mixed with '+ss' or '*/ss'. (" << val_ << ")." );
			return false;
		} else if( specType == RelativeToLoadTime ) {
			WEBFW_LOG_ERROR( "meta-model: Spec type: 'HH:MM' can not be mixed with '+ss' or '*/ss'. (" << val_ << ")." );
			return false;
		}
	}
	return true;
}

//  name=YR;nextrun=(18:00,14:00)
bool 
MetaModelConf::
parseConf( const std::string &conf )
{
	using namespace miutil;
	
	string key;
	string val;
	
	vector<string> keyvals = splitstr( conf, ';' );
	
	for( vector<string>::iterator it = keyvals.begin(); 
	     it != keyvals.end();
	     ++it )
	{
		vector<string> keyval = splitstr( *it, '=' );
		
		if( keyval.size() != 2 )
			continue;
		
		key = keyval[0]; 
		val = keyval[1]; 
		trimstr( key );
		trimstr( val );
		
		if( val.empty() || key.empty() )
			continue;
		
		if( key == "name" ) {
			name_ = val;
		} else if( key == "nextrun" ) {  
			if( ! parseNextrun( val ) )
				return false;
		}else {
			return false;
		}
	}
	
	return true;
}

MetaModelConfList
configureMetaModelConf( const wdb2ts::config::ActionParam &params )
{
	const string META_MODEL_KEY("meta_model-");
	MetaModelConfList metaModelConf;
	
	WEBFW_USE_LOGGER( "main" );

	string::size_type i;
	
	for( wdb2ts::config::ActionParam::const_iterator it = params.begin(); 
	     it!=params.end(); 
	     ++it ) 
	{
		i=it->first.find( META_MODEL_KEY );
			
		if( i != string::npos && i==0 ) {
			string provider=it->first;
			provider.erase(0, META_MODEL_KEY.size() );
			//miutil::replace( provider, "_", " ");

			ProviderList pvList = ProviderList::decode( provider );
			
			MetaModelConf conf;
			
			if( conf.parseConf( it->second.asString() ) ){
				for( ProviderList::iterator pit = pvList.begin(); pit != pvList.end(); ++pit )
					metaModelConf[pit->providerWithPlacename()] = conf;
			}else {
				WEBFW_LOG_ERROR( "Failed to parse value for: " << it->first << " (" << it->second << ")." );
			}
		}
	}

	std::ostringstream logMsg;
	for( MetaModelConfList::iterator metaIt = metaModelConf.begin();
	     metaIt != metaModelConf.end(); 
	     ++metaIt )
		logMsg << "Meta: " << metaIt->first << ": " << metaIt->second << endl;
	WEBFW_LOG_DEBUG( logMsg.str() );
	
	return metaModelConf;
}



std::ostream& 
operator<<( std::ostream &o, const MetaModelConf &c )
{
	o << "[ " << c.name_ ;
	
	for( TimeDurationList::const_iterator it=c.nextrun_.begin();
	     it != c.nextrun_.end(); 
	     ++it )
		o << " <" << *it << ">"; 
	
	o << " ]";
	return o;
}

std::ostream& operator<<( std::ostream &o, const MetaModelConf::SpecType c ){
	switch(c) {
	case MetaModelConf::Error: o << "Error"; break;
	case MetaModelConf::Undefined: o << "Undefined"; break;
	case MetaModelConf::Absolute: o << "Absolute"; break;
	case MetaModelConf::RelativeToLoadTime: o << "RelativeToLoadTime"; break;
	default:
		o << "<Unknown>";
	}
	return o;
}
}

