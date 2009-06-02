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

namespace wdb2ts {

using namespace boost::posix_time;
using namespace std;

	

MetaModelConf::
MetaModelConf()
{
}

MetaModelConf::
MetaModelConf( const std::string &name )
	: name_( name )
{
}

MetaModelConf::
MetaModelConf( const MetaModelConf &m )
	: name_( m.name_ ), nextrun_( m.nextrun_ )
{
}
	
MetaModelConf::
MetaModelConf& 
MetaModelConf::
operator=( const MetaModelConf &rhs )
{
	if( this != &rhs ) {
		name_ = rhs.name_;
		nextrun_ = rhs.nextrun_;
	}
	
	return *this;
}

	
boost::posix_time::ptime
MetaModelConf::
findNextrun()const
{
	ptime now( second_clock::universal_time() );
	ptime midnight( now.date(), time_duration(0, 0, 0, 0) );
	time_duration tdNow( now.time_of_day() );
	
	if( nextrun_.empty() )
		return ptime(); //Udefined.
	
	TimeDurationList::const_iterator it=nextrun_.begin();
	for( ; it != nextrun_.end() && *it < tdNow ; ++it ); 
		
	if( it == nextrun_.end() ) {
		midnight += hours( 24 ); //next day
		it = nextrun_.begin();
	}
	
	return midnight + *it;
}

void 
MetaModelConf::
addTimeDuration( const boost::posix_time::time_duration &td )
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


bool
MetaModelConf::
parseNextrun( const std::string &val_ )
{
	using namespace miutil;
	
	string sHH, sMM;
	string val;
	int h, m;	
	
	vector<string> keyvals = splitstr( val_, ',' );
	
	for( vector<string>::iterator it = keyvals.begin(); 
   	  it != keyvals.end();
        ++it )
	{
		vector<string> timeDurationList = splitstr( *it, ':' );
		
		if( timeDurationList.size() != 2 )
					continue;
				
		sHH = timeDurationList[0]; 
		sMM = timeDurationList[1]; 
		trimstr( sHH );
		trimstr( sMM );

		if( sHH.empty() || sMM.empty() )
			continue;

		if( ! isnumber( sHH ) || ! isnumber( sMM ) )
			return false;
		
		
		if( sscanf( sHH.c_str(), "%d", &h) != 1 )
			return false;
		
		if( sscanf( sMM.c_str(), "%d", &m) != 1 )
			return false;
		
		addTimeDuration( time_duration( h, m, 0, 0) );
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
			}else
				cerr << "Failed to parse value for: " << it->first << " (" << it->second << ")." << endl;
		}
	}

	for( MetaModelConfList::iterator metaIt = metaModelConf.begin();
	     metaIt != metaModelConf.end(); 
	     ++metaIt )
		cerr << "Meta: " << metaIt->first << ": " << metaIt->second << endl;
	
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


}

