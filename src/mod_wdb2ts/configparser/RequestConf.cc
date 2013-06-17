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


#include <splitstr.h>
#include <RequestConf.h>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <sstream>

namespace wdb2ts {
namespace config {

using namespace std;

namespace {
	const char *defaultid="";
}


void
ParamDefConfig::
clear()
{
	idParamDefs.clear();
}

bool
ParamDefConfig::
addParamDef( const std::string &paramdefId_,
		     const wdb2ts::ParamDef    &pd,
	         const std::list<std::string> &provider,
	         bool replace,
	         std::ostream &err )
{
	string paramdefId( paramdefId_ );
	if( paramdefId.empty() )
		paramdefId = defaultid;

	if( provider.empty() ) {
		if( ! idParamDefs[paramdefId].addParamDef( pd, "", replace) ) {
			err << "addParam: ParamDef '" << pd.alias() << "' provider: 'default' allready defined.";
			return false;
		}
	} else {
		for( std::list<std::string>::const_iterator it = provider.begin();
			it != provider.end(); ++it )
		{
			if( ! idParamDefs[paramdefId].addParamDef( pd, *it, replace ) ) {
				err << "addParam: ParamDef '" << pd.alias() << "' provider: '" << *it << "' allready defined.";
				return false;
			}
		}
	}

	return true;
}

bool
ParamDefConfig::
hasParam(  const std::string &alias,
           const std::string &provider,
           const std::string paramdefsId_ )const
{
	string paramdefsId( paramdefsId_ );

	if( paramdefsId.empty() )
		paramdefsId = defaultid;

	ParamDefs::const_iterator it=idParamDefs.find( paramdefsId );

	if( it == idParamDefs.end() )
		return false;
	else
		return it->second.hasParam( alias, provider );
}


bool
ParamDefConfig::
hasParamDefId( const std::string &id )const
{
	ParamDefs::const_iterator it=idParamDefs.find( id );

	return it != idParamDefs.end();
}


wdb2ts::ParamDefList
ParamDefConfig::
paramDefs( const std::string paramdefsId_ ) const
{
	string paramdefsId( paramdefsId_);

	if( paramdefsId.empty() )
		paramdefsId = defaultid;

	ParamDefs::const_iterator it=idParamDefs.find( paramdefsId );

	if( it != idParamDefs.end() )
		return it->second;
	else
		return wdb2ts::ParamDefList();
}

void
ParamDefConfig::
merge( ParamDefConfig *other, bool replace )
{
//	cerr << "Merge this:" << endl;
//	cerr << *this << endl << endl;
//	cerr << "With other:" << endl;
//	cerr << *other << endl << endl;

	for( ParamDefs::iterator it = other->idParamDefs.begin();
		 it!=other->idParamDefs.end(); ++it )
	{
		ParamDefs::iterator itIdParam = idParamDefs.find( it->first );

		if( itIdParam == idParamDefs.end() ) {
			idParamDefs[it->first] = it->second;
			continue;
		}

		itIdParam->second.merge( &it->second, replace );
	}

//	cerr << "Merge result:" << endl;
//	cerr << *this << endl << endl;
}

std::ostream&
operator<<(std::ostream &o, const ParamDefConfig &pd )
{
	for( ParamDefConfig::ParamDefs::const_iterator idIt = pd.idParamDefs.begin();
	     idIt != pd.idParamDefs.end(); ++idIt )
	{
		o << "{" <<  idIt->first << "}" <<  endl;
		for( ParamDefList::const_iterator provIt=idIt->second.begin();
			 provIt != idIt->second.end(); ++provIt	) {
			o << "   [" <<  provIt->first << "]" << endl;

			for(std::list<wdb2ts::ParamDef>::const_iterator parIt = provIt->second.begin();
				parIt != provIt->second.end(); ++parIt )
				o << "      " << parIt->alias() << ", " << parIt->valueparametername() << endl;
		}
	}

	return o;
}


Version::
Version( const std::string &version) 
	: majorVer( -1 ), minorVer( -1 ) 
{
	vector<string> v = miutil::splitstr( version, '.' );
	
	if( v.size() > 0  && v.size() <= 2 ) {
		try {
			majorVer = boost::lexical_cast<int>( v[0] );
	
			if( v.size() == 2 )
				minorVer = boost::lexical_cast<int>( v[1] );
			else if( majorVer != -1 )
				minorVer = 0;
			
			if( (majorVer == 0 && minorVer > 0)   ||
				 (majorVer > 0  && minorVer >= 0 ) ||
				 (majorVer == -1  && minorVer == -1) )
				return;
		} 
		catch( ... ) {
		}
	} 
		
	majorVer = 0;
	minorVer = 0;
}

Version& 
Version::
operator=(const Version& rhs ) 
{
	if( this != &rhs ) {
		majorVer = rhs.majorVer;
		minorVer = rhs.minorVer;
	}
	
	return *this;
}

bool 
Version::
operator<( const Version &rhs )
{
	if( (majorVer==-1 && minorVer==-1) ||
		 (majorVer==0 && minorVer==0)      )
		return false;
	
	if( majorVer < rhs.majorVer)
		return true;
	
	if( majorVer == rhs.majorVer && minorVer < rhs.minorVer )
		return true;
	
	return false;
}

bool 
Version::
operator==( const Version &rhs )
{
	if( majorVer == rhs.majorVer && minorVer == rhs.minorVer )
		return true;
	
	return false;
}

bool 
Version::
operator!=( const Version &rhs )
{
	if( majorVer != rhs.majorVer || minorVer != rhs.minorVer )
		return true;
	
	return false;
}


std::string
Version::
operator()()const
{
	ostringstream ost;
	
	ost << *this;
	return ost.str();
}

std::ostream& 
operator<<(std::ostream& out,
           const Version& v)
{
	if( v.majorVer==-1 && v.minorVer==-1)
		out << " (higest) ";
	else if( v.majorVer==0 && v.minorVer==0)
		out << " (invalid) ";
	else
		out << v.majorVer <<"." << v.minorVer;
	
	return out;
		
}

bool
Request::
addRequestVersion( boost::shared_ptr<RequestConf> requestVersion, std::string &warning )
{
	warning.erase();
	
	if( ! requestVersion ) {
		warning="Request::addRequestVersion: requestVersion == 0";
		return false;
	}
	
	if( requestVersion->version.invalid() || requestVersion->version.defaultVersionHighest() ) {
		warning="Request::addRequestVersion: Invalid version attribute. (request: '" + path.asString("") + "')";
		
		return false;
	}
	
	for( RequestVersions::const_iterator it=requestVersions.begin();
		  it!=requestVersions.end(); ++it ) {
		if( *it == requestVersion ) {
			warning = string("addRequestVersion: request: '" + path.asString("") + "' duplicate versions. (" + requestVersion->version() + ")." );
			return false;
		}
	}
	
	 RequestVersions::iterator it=requestVersions.begin();
	 
	 for(; it!=requestVersions.end() && requestVersion->version < (*it)->version; ++it );
	 
	 if( it == requestVersions.end() )
		 requestVersions.push_back( requestVersion );
	 else
		 requestVersions.insert( it, requestVersion );
	 
	 return true;
}



/**
 * resolve copys from the default values to the coresponding
 * values in the versions if they are undefined.
 * 
 * The same for actionParams.
 */
void
Request::
resolve( )
{
	using namespace miutil;
	ActionParam actionParam( requestDefault.actionParam );
	Value queryid( requestDefault.queryid );
	Value action( requestDefault.action );
	Value wdbDB( requestDefault.wdbDB );

	for( RequestVersions::const_iterator it=requestVersions.begin();
		  it!=requestVersions.end(); 
		  ++it ) {
		if( ! (*it)->queryid.defined() )
			(*it)->queryid = queryid;
		
		if( ! (*it)->action.defined() )
			(*it)->action = action;
		if( ! (*it)->wdbDB.defined() )
			(*it)->wdbDB = wdbDB;
	
		for( ActionParam::const_iterator apIt = actionParam.begin();
			  apIt != actionParam.end();
			  ++apIt ) {
			if( ! apIt->second.defined() )
				continue;
			
			ActionParam::const_iterator myIt = (*it)->actionParam.find( apIt->first );
			
			if( myIt != (*it)->actionParam.end() && myIt->second.defined() )
				continue;
			
			(*it)->actionParam[apIt->first] = apIt->second;
		}

		if( ! requestDefault.paramdef.idParamDefs.empty() ) {
			//cerr << " ---- Default params: " <<  requestDefault.paramdef << endl;
			ostringstream err;
			(*it)->paramdef.merge( &requestDefault.paramdef, false );
		}
	}
	
	if( requestDefault.version.defaultVersionHighest() ) {
		RequestVersions::reverse_iterator it=requestVersions.rbegin();
		
		if( it != requestVersions.rend() )
			requestDefault.version = (*it)->version;
		else 
			requestDefault.version = Version();
	} else if ( requestDefault.version.invalid() ) {
		RequestVersions::iterator it=requestVersions.begin();
		
		for( ; it != requestVersions.end() && (*it)->version != requestDefault.version ; ++it );
		
		if( it == requestVersions.end() )
			requestDefault.version = Version();
			
	}
}



void 
addRequest( RequestMap &requestMap, boost::shared_ptr<Request> request )
{
	if( ! request )
		return;
	
	if( request->path.asString("").empty() )
		return;
	
//	//Merge paramdefs from defaultRequest with the paramsdefs in each
//	//RequestVersions. Parameters in the RequestVersions take precedence
//	//over the the parameters in defaultRequest.
//	if( ! request->requestDefault.paramdef.idParamDefs.empty() ) {
//		for( Request::RequestVersions::iterator it=request->requestVersions.begin();
//			 it != request->requestVersions.end(); ++it )
//		{
//			cerr << request->requestDefault.paramdef << endl;
//			ostringstream err;
//			(*it)->paramdef.merge( &request->requestDefault.paramdef, err, "", false );
//		}
//	}

	RequestMap::iterator it = requestMap.find( request->path.asString() );
	
	if( it != requestMap.end() )
		return;
	
	requestMap[request->path.asString()] = request;
		
}

std::ostream& 
operator<<(std::ostream& output, const Request& r )
{
	output << "Request: " <<  r.path << endl
			 << "           wdbdb: " << r.requestDefault.wdbDB << endl
			 << "         queryid: " << r.requestDefault.queryid << endl
			 << "          action: " << r.requestDefault.action << endl
	       << "    def. version: " << r.requestDefault.version << endl
	       << "  def. ActionParam: ";
	
	if( r.requestDefault.actionParam.empty() ) 
		output << "(none)" << endl;
	else {
		output << endl;

		for( ActionParam::const_iterator it=r.requestDefault.actionParam.begin();
			it != r.requestDefault.actionParam.end(); ++it)
			output << "      " << it->first << ": " << it->second << endl;
	}
	 
	for(Request::RequestVersions::const_iterator it=r.requestVersions.begin();
		it != r.requestVersions.end(); ++it ) {
		output << "      Version: " << (*it)->version << endl
		       << "        wdbdb: " << (*it)->wdbDB << endl
			   << "      queryid: " << (*it)->queryid << endl
			   << "       action: " << (*it)->action << endl
			   << "  ActionParam: ";
		if( (*it)->actionParam.empty() ) 
				output << "(none)" << endl;
		else {
			output << endl;
			for( ActionParam::const_iterator itAP=(*it)->actionParam.begin();
				itAP != (*it)->actionParam.end(); ++itAP)
				output << "      " << itAP->first << ": " << itAP->second << endl;
		}
		output << " ----- Params --------\n";
		for( ParamDefConfig::ParamDefs::const_iterator itId = (*it)->paramdef.idParamDefs.begin();
					itId != (*it)->paramdef.idParamDefs.end(); ++itId )
		{
			output << "[" << itId->first << "] : id" << endl;

			for( wdb2ts::ParamDefList::const_iterator pit = itId->second.begin();
					pit != itId->second.end(); ++pit )
			{
				output << "   [" << pit->first << "] " << endl;
				for( std::list<wdb2ts::ParamDef>::const_iterator it = pit->second.begin();
				     it != pit->second.end();  ++it )
				{
					output << "      " <<  it->alias() << " : " << it->valueparametername()<<endl;
				}
			}
		}
	}

	return output;
}


}
}
