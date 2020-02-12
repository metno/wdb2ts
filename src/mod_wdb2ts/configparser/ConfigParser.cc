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

#include <memory>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <readfile.h>
#include <ConfigParser.h>
#include <trimstr.h>
#include <replace.h>
#include <splitstr.h>
#include <compresspace.h>
#include "mod_wdb2ts/WciWebQuery/WciWebQuery.h"

namespace wdb2ts {
namespace config {

using namespace std;

ConfigParser::
ConfigParser()
	:config( 0 ), providerAsWdbid(false), inChardata( false ),
	 currentQueryDefPrognosisLengthSeconds(0),
	 recursionDepth( 0 ),basedir( WDB2TS_DEFAULT_SYSCONFDIR )
	 
{
}

ConfigParser::
ConfigParser( const std::string &basedir_, bool providerAsWdbidDefaultValue )
	: config( 0 ), providerAsWdbid(providerAsWdbidDefaultValue),inChardata( false ),
	  currentQueryAutoId(1),
	  currentQueryDefPrognosisLengthSeconds(0),
	  recursionDepth( 0 ),basedir( basedir_ )
{
}

ConfigParser::
~ConfigParser()
{
}
	
void 
ConfigParser::
characters( const std::string &buf )
{
	if( ! inChardata ) {
		inChardata = true;
		chardata.str("");
	}
		
	chardata << buf;
}


bool
ConfigParser::
mergeConfig( Config *config )
{
   return false;
}

std::string
ConfigParser::
checkPath( const std::string &filename )
{
	string file( filename );

	//If the filename is NOT an absolute path we
	//prepend the basedir path to the file. The
	//default basedir is SYSCONF path.
	//The basedir path must be an absolute path.
	miutil::trimstr( file );
	if( file.size() > 0 && file[0] != '/' ) {
		std::string confpath( basedir );
		miutil::trimstr( confpath );
		if( confpath.size() > 0  && confpath[0] == '/' ) {
			if( confpath[confpath.length()-1] != '/' )
				confpath += '/';
			file = confpath + file;
		} else {
			error("Basedir '" + basedir + "' is NOT an absolute path and the file to read is not an absolute path '" + filename + "'.");
			return "";
		}
	}

	return file;
}


Config*
ConfigParser::
readFile( const std::string &filename )
{
	string file( checkPath( filename ) );

	if( file.empty() ) {
		return 0;
	}


	//cerr << "readFile: '" << file << "'.\n";

	if( recursionDepth > 30 ) {
		error( "Recursion dept error: Circular reading of configuaration files?");
		return 0;
	}

	++recursionDepth;

	ostringstream ost;
	ConfigParser myParser( basedir, providerAsWdbid );
	myParser.recursionDepth = recursionDepth;

	Config *myConfig = myParser.parseFile( file );

	--recursionDepth;

	if( ! myParser.getErrMsg().empty() ) {
		ost << "Problems in file '" << file << "'. "
				<<  myParser.getErrMsg() << endl << " --- End file '" << file << "'";
		warning( ost.str() );
		ost.str("");
	}

	if( ! myConfig ) {
		if( recursionDepth <= 10 ) {
			ost << "Parsing of file <" << file << "> failed.";
			error( ost.str().c_str() );
		}
		return 0;
	} else {
		myConfig->includedFiles.insert(filename);
		return myConfig;
	}
}

int
ConfigParser::
getPrognosisLength( const AttributeMap &attributes, int defaultValue )
{
	string prognosisLength;
	int scale;
	char modifier='h';

	getAttr( attributes, "prognosis_length", prognosisLength, "");

	if( prognosisLength.empty() )
		return defaultValue;

	string::size_type i = prognosisLength.find_first_not_of("0123456789");

	//No dimension identifier given, h (hours) assumed.
	//Valid values h (hours) and s (seconds).
	if( i == string::npos ) {
		scale = 3600;
		i = prognosisLength.size();
	} else {
		modifier = prognosisLength[i];
	}

	switch( modifier ) {
	case 'h': scale=3600; break;
	case 's': scale=1; break;
	default:
		//modifier (h) hour is used.
		ostringstream ost;
		ost << "prognosis_length: Invalid modifier '" << prognosisLength[i] << "'. Valid modifier h (hours) and s (seconds).";
		error( ost.str() );
	}

	try {
		return boost::lexical_cast<int>( prognosisLength.substr( 0, i ) ) * scale;
	}catch( const boost::bad_lexical_cast &ex) {
	}

	return 0;
}


//int
//ConfigParser::
//getPrognosisLength( const AttributeMap &attributes, int defaultValue )const
//{
//	string prognosisLength;
//	int scale;
//
//	getAttr( attributes, "prognosis_length", prognosisLength, "");
//
//	if( prognosisLength.empty() )
//		return defaultValue;
//
//	string::size_type i = prognosisLength.find_first_not_of("0123456789");
//
//	//No dimension identifier given, h (hours) assumed.
//	//Valid values h (hours) and s (seconds).
//	if( i == string::npos ) {
//		scale = 3600;
//		i = prognosisLength.size();
//	} else {
//		switch( prognosisLength[i] ) {
//		case 'h': scale=3600; break;
//		case 's': scale=1; break;
//		default:
//			ostringstream ost;
//			ost << "prognosis_length: Invalid modifier '" << prognosisLength[i] << "'. Valid modifier h (hours) and s (seconds).";
//			error( ost.str() );
//	}
//
//	try {
//		return boost::lexical_cast<int>( prognosisLength.substr( i ) ) * scale;
//	}catch( const boost::bad_lexical_cast &ex) {
//	}
//
//	return 0;
//}

bool
ConfigParser::
doWdb2ts( const AttributeMap &attributes )
{
	string sProviderAsWdbId;

	if(  getAttr( attributes, "provider_as_wdbdb", sProviderAsWdbId, "false" ) ) {
		if( sProviderAsWdbId[0]=='t' || sProviderAsWdbId[0]=='T' )
			providerAsWdbid = true;
		else
			providerAsWdbid = false;
	}

	return true;
}



bool
ConfigParser::
doInclude( const AttributeMap &attributes )
{
	std::string file;

	//cerr << "doInclude called" << endl;
	if( ! getAttr( attributes, "file", file ) || file.empty() ) {
		error("Include statement. Missing mandatory attribute 'file'.");
		return false;
	}

	//cerr << "Include file: '" << file << "' (" << checkPath(file)<<").\n";
	Config *myConfig = readFile( file );

	if( ! myConfig ) {

		return false;
	} else {
		ostringstream ost;
		std::auto_ptr<Config> p( myConfig ); //Delete myConfig on return.
		//cerr << "Do merge!" << endl;

		if( ! config->merge( myConfig, ost, file ) ) {
			error( ost.str() );
			return false;
		}

		return true;
	}
}

void
ConfigParser::
doRequestConf( const AttributeMap &attributes, RequestConf &conf, const std::string &versionAttrName )
{
	AttributeMap::const_iterator it;
		
	for( it=attributes.begin(); it != attributes.end(); ++it ) {
		if( it->first == "wdbdb" )
			conf.wdbDB = it->second;
		else if( it->first == "action" )
			conf.action = it->second;
		else if( it->first == "queryid" )
			conf.queryid = it->second;
		else if( it->first == versionAttrName )
			conf.version = Version( it->second );
		else if( it->first == "path" )
			continue;
		else if( it->first == "schema")
		   conf.schema = it->second;
		else
			warning( "Unknown attribute '" + it->first +"' value='" + it->second + "'.");
	}
}


bool
ConfigParser::
doRequest( const AttributeMap &attributes )
{
	string val;
	
	if( currentRequest ) {
		error("Allready parsing a request.");
		return false;
	}
	
	if( ! getAttr( attributes, "path", val ) || val.empty() ) {
		error("Missing mandatory attribute 'path'.");
		return false;
	}
		
	currentRequest.reset(new Request());
	currentRequest->path = val;
	
	doRequestConf( attributes, currentRequest->requestDefault, "default_version" );
	return true;
}


bool 
ConfigParser::
doRequestActionParam( const AttributeMap &attributes, ActionParam &actionParam )
{		
	string key;
	string val;
	
	if( ! getAttr( attributes, "key", key ) ||
		 ! getAttr( attributes, "value", val )	|| 
		 key.empty() || val.empty() ) {
		if( key.empty() )
			error("doRequestActionParam: missing mandatory attribute 'key'.");
		if( val.empty() )
			error("doRequestActionParam: missing mandatory attribute 'val'.");

		return false;
	}
		
	actionParam[key] = val;
	return true;
}


bool 
ConfigParser::
doRequestDefaultActionParam( const AttributeMap &attributes )
{
	if( ! currentRequest ) {
		error("doRequestDefaultActionParam: NO request.");
		return false;
	}
	
	return doRequestActionParam( attributes, currentRequest->requestDefault.actionParam );
}

bool
ConfigParser::
doRequestVersion( const AttributeMap &attributes )
{
	if( currentRequestVersion ) {
		error("doRequestVersion: Allready started a version element.");
		return false;
	}
	
	currentRequestVersion.reset( new RequestConf() );
	
	doRequestConf( attributes, *currentRequestVersion, "version" );
	return true;
}

void
ConfigParser::
doQuery( const AttributeMap &attributes )
{
	ostringstream sid;
	string probe;
	string stopIfData;
	string prognosisLength;

	getAttr( attributes, "must_have_data", probe, "false" );
	
	sid << "q" << currentQueryAutoId;
	getAttr( attributes, "id", currentQueryId, sid.str() );


	if( !probe.empty() && (probe[0]=='t' || probe[0]=='T') )
		currentQueryProbe = true;
	else
		currentQueryProbe = false;


	getAttr( attributes, "stop_if_data", stopIfData, "false" );

	if( !stopIfData.empty() && (stopIfData[0]=='t' || stopIfData[0]=='T') )
		currentQueryStopIfData = true;
	else
		currentQueryStopIfData = false;

	getAttr( attributes, "wdbdb", currentQueryWdbdb, "" );

	if( currentQueryWdbdb.empty() && !providerAsWdbid) {
		currentQueryWdbdb = currentQueryDefWdbdb;
	}

	currentQueryPrognosisLengthSeconds = getPrognosisLength( attributes, currentQueryDefPrognosisLengthSeconds );
}

bool
ConfigParser::
doQueryDef( const AttributeMap &attributes )
{
	string id;
	string sParalell;
	string prognosisLength;

	if( ! getAttr( attributes, "id", id) || id.empty() ) {
		error("Mandatory 'querydef' attribute 'id' missing.");
		return false;
	}
	
	itCurrentQueryDef = config->querys.find( id );
	
	if( itCurrentQueryDef != config->querys.end() ){
		error("Querydef: id: '"+id+"' alleady defined.");
		return false;
	}
	
	config->querys[id]=Config::Query();
	itCurrentQueryDef = config->querys.find( id );

	currentQueryDefPrognosisLengthSeconds = getPrognosisLength( attributes, 0);

	getAttr( attributes, "wdbdb", currentQueryDefWdbdb, "");

	if( hasAttr( attributes, "parallel" ) )
	   getAttr( attributes, "parallel", sParalell, "1");
	else if( hasAttr( attributes, "paralell" ) )
	   getAttr( attributes, "paralell", sParalell, "1");
	else if( providerAsWdbid )
		sParalell="0";

	try {
		miutil::Value val( sParalell );
		int n = val.as<int>();

		if( n < 0 )
			n = 1;

		itCurrentQueryDef->second.dbRequestsInParalells( n );
	}
	catch( std::exception & ex) {
		error("querydef: The value to 'paralell' must be a number. Was: paralell=\""+ sParalell + "\"");
		return false;
	}

	return true;
}

bool
ConfigParser::
doParamDef( const AttributeMap &attributes )
{
	string id;
	string provider;
	
	currentParamDef.clear();
	currentParamDefProvider.clear();
	
	
	if( ! getAttr( attributes, "id", id) || id.empty() ) {
		error("Mandatory 'paramdef' attribute 'id' missing.");
		return false;
	}
	
	getAttr( attributes, "dataprovider", provider );
	
	std::vector<std::string> vProviders = miutil::splitstr( provider, ';');
	
	
	for( std::vector<std::string>::iterator it=vProviders.begin();
	     it != vProviders.end(); 
	     ++it ) 
	{
		miutil::trimstr( *it );
		
		if( it->empty() )
			continue;
		

		if(  config->paramdef.hasParam( id, *it, currentParamDefsId ) ) {
			if( ! currentParamDefOverrid ) {
				currentParamDefProvider.clear();
				error("doParamDef id: '" + id + "' provider: '" + *it + "' allready defined.");
				return false;
			}
		}
		
		currentParamDefProvider.push_back( *it );
	}
	
	currentParamDef.id = id;
	return true;
}

//bool
//ConfigParser::
//doParamDefs( const AttributeMap &attributes )
//{
//	string idref;
//	vector<string> idRefs;
//	getAttr( attributes, "id", currentParamDefsId, "" );
//	getAttr( attributes, "idref", idref, "" );
//
//	currentParamDefConfig.clear();
//
//	if( idref.empty() )
//		currentParamDefOverrid = false;
//	else {
//		currentParamDefOverrid = true;
//		idRefs = miutil::splitstr( idref, ';');
//	}
//
//	if( ! idRefs.empty() ) {
//		currentParamDefConfig.idParamDefs[currentParamDefsId] = wdb2ts::ParamDefList();
//
//		for( vector<string>::iterator it=idRefs.begin();
//			 it != idRefs.end(); ++it	)
//		{
//			Config *conf=0;
//			string file;
//			string id;
//			string::size_type i = it->find("@");
//			if( i != string::npos ) {
//				id = it->substr( 0, i );
//				file = it->substr(i+1);
//			} else {
//				id = *it;
//			}
//
//			miutil::trimstr( id );
//			miutil::trimstr( file );
//			cerr << "idref: " << *it << "(" << id << "," << file << ")" << endl;
//
//			if( !file.empty() ) {
//				conf = readFile( file );
//			}
//
//			if( conf ) {
//				wdb2ts::ParamDefList tmp=conf->paramdef.paramDefs( id );
//				currentParamDefConfig.idParamDefs[currentParamDefsId].merge( &tmp, true );
//				delete conf;
//			}
//		}
//	}
//	return true;
//}


bool 
ConfigParser::
doParamDefs( const AttributeMap &attributes )
{
	string idref;
	vector<string> idRefs;
	getAttr( attributes, "id", currentParamDefsId, "__UNDEFINED__" );
	getAttr( attributes, "idref", idref, "" );

	currentParamDefConfig.clear();

	if( idref.empty() )
		currentParamDefOverrid = false;
	else {
		currentParamDefOverrid = true;
		idRefs = miutil::splitstr( idref, ';');
	}

	if( ! idRefs.empty() ) {
		currentParamDefConfig.idParamDefs[currentParamDefsId] = wdb2ts::ParamDefList();

		for( vector<string>::iterator it=idRefs.begin();
			 it != idRefs.end(); ++it	)
		{
			Config *conf=0;
			string file;
			string id;
			bool allParamIds=false;
			string::size_type i = it->find("@");

			if( i != string::npos ) {
				id = it->substr( 0, i );
				file = it->substr(i+1);
			} else {
				allParamIds = true;
				file = *it;
			}

			miutil::trimstr( id );
			miutil::trimstr( file );
			//cerr << "idref: " << *it << "(" << id << "," << file << ")" << endl;

			if( !file.empty() ) {
				conf = readFile( file );
			}

			if( ! conf )
				continue;

			if( allParamIds ) {
				for( ParamDefConfig::ParamDefs::iterator pit=conf->paramdef.idParamDefs.begin();
					 pit != conf->paramdef.idParamDefs.end(); ++pit ) {
					wdb2ts::ParamDefList tmp=pit->second;
					if( currentParamDefsId == "__UNDEFINED__" )
						currentParamDefConfig.idParamDefs[pit->first].merge( &tmp, true );
					else
						currentParamDefConfig.idParamDefs[currentParamDefsId].merge( &tmp, true );
				}
			} else {
				wdb2ts::ParamDefList tmp=conf->paramdef.paramDefs( id );
				if( currentParamDefsId == "__UNDEFINED__" )
					currentParamDefConfig.idParamDefs[id].merge( &tmp, true );
				else
					currentParamDefConfig.idParamDefs[currentParamDefsId].merge( &tmp, true );
			}

			delete conf;
		}
	}

	//If the currentParamDefsId id undefined set it to the empty id.
	//We could also have set it to the predefined 'default' id but,
	//this id should be set explicit since it has a predefined meaning
	//at the moment.
	if( currentParamDefsId == "__UNDEFINED__" )
		currentParamDefsId="";

	return true;
}



bool
ConfigParser::
doValueParameter( const AttributeMap &attributes )
{
	string name;
	string unit;
		
	if( ! getAttr( attributes, "name", name) || name.empty() ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. Mandatory 'valueparameter' attribute 'name' missing.");
		return false;
	}
		
	if( ! getAttr( attributes, "unit", unit) ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. Mandatory 'valueparameter' attribute 'unit' missing.");
		return false;
	}
		
	currentParamDef.valueparameterName = name;
	currentParamDef.valueparameterUnit = unit;

	return true;
}

bool
ConfigParser::
doLevelParameter( const AttributeMap &attributes )
{
	string name;
	string unit;
	string from;
	string to;

	bool ret=true;
	
	if( ! getAttr( attributes, "name", name) || name.empty() ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. Mandatory 'levelparameter' attribute 'name' missing.");
		return false;
	}
	
	
	if( ! getAttr( attributes, "unit", unit ) ) {
			error("ParamDef id: '"+currentParamDef.id.asString()+"'. 'levelparameter' attribute 'unit' must be defined.");
			return false;
	}
	if( ! getAttr( attributes, "from", from ) ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. 'levelparameter' attribute 'from' must be defined.");
		return false;
	}
	if( ! getAttr( attributes, "to", to ) ){
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. 'levelparameter' attribute 'to' must be defined.");
		return false;
	}

	currentParamDef.levelName = name;
	currentParamDef.levelUnit = unit;

	string par, parVal;

	try {
		par="to"; parVal = to;
		currentParamDef.levelTo = miutil::Value( to ).as<int>();

		par="from"; parVal = from;
		currentParamDef.levelFrom = miutil::Value(from).as<int>();
	}
	catch( std::exception & ex) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"' : levelparameter, invalid attribute value for '" + par + "' value '"
				+ parVal +"': Reason: " + ex.what() +".");
		ret = false;
	}

	return ret;
}

bool
ConfigParser::
doValue(  const AttributeMap &attributes )
{
	string scale;
	string offset;
	string null;
	miutil::Value vScale;
	miutil::Value vOffset;
	bool ret = true;
	
	getAttr( attributes, "scale", scale, "1.0" );
	getAttr( attributes, "offset", offset, "0.0" );
	getAttr( attributes, "null", null, "" );

	vScale = scale;
	vOffset = offset;
	
	try {
		currentParamDef.valueScale = vScale.as<float>();
	}
	catch( std::exception & ex) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"' : value element invalid attribute value scale '"+ scale + "' " + ex.what() +".");
		ret = false;
	}
	
	try {
		currentParamDef.valueOffset = vOffset.as<float>();
	}
	catch( std::exception & ex) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"' : value element invalid attribute value offset '"+ offset + "' " + ex.what() +".");
		ret = false;
	}	
	
	miutil::trimstr( null );
	
	if( ! null.empty() ) {
		string::size_type i = null.find_first_of( ":");
		string val;
		string cmp;
		
		if( i != string::npos ) {
			cmp = null.substr( 0, i );
			
			if( (i+1) < null.length() )
				val = null.substr( i+1 );
		} else {
			val = null;
		}
		miutil::trimstr( val );
		miutil::trimstr( cmp );
		
		if( cmp == "lt" ) 
			currentParamDef.compare = wdb2ts::ParamDef::lesser;
		else if( cmp == "gt" )
			currentParamDef.compare = wdb2ts::ParamDef::greater;
		else if( cmp == "eq" || cmp.empty() )
			currentParamDef.compare = wdb2ts::ParamDef::equal;
		else {
			error("ParamDef id: '"+currentParamDef.id.asString()+"' : value attribut 'null' operator '" + cmp + "' invalid!");
			return false;
		}

		try {
			miutil::Value vNull( val ); 
			currentParamDef.compareValue = vNull.as<int>();
		}
		catch( std::exception & ex) {
			error("ParamDef id: '"+currentParamDef.id.asString()+"' : value attribut 'null' value '" + val + "' not an integer!");
			currentParamDef.compare = wdb2ts::ParamDef::undef;
			ret = false;
		}
	}
	
	return ret;
}

bool
ConfigParser::
doDataVersion( const AttributeMap &attributes )
{
	string value;
	miutil::Value vValue;
	bool ret = true;
	
	getAttr( attributes, "version", value, "-1" );

	vValue = value;
	
	try {
		currentParamDef.dataVersion = vValue.as<int>();
	}
	catch( std::exception & ex) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"' : dataversion element, invalid attribute version '"+ value + "' " + ex.what() +".");
		ret = false;
	}
	
	return ret;
}

bool
ConfigParser::
addParamDef()
{
	string alias = currentParamDef.id.asString("");
	
	if( alias.empty() ) {
		error( "Paramdef without an valid 'id' attribute.");
		currentParamDef.clear();
		currentParamDefProvider.clear();
		return false;
	}
	
	if( ! currentParamDef.valid( true ) ) {
		error("Some invalid values for paramdef id '"+alias+"'.");
		currentParamDef.clear();
		currentParamDefProvider.clear();
		return false;
	}
	
	wdb2ts::ParamDef pd( alias,
			               currentParamDef.valueparameterName.asString(),
			               currentParamDef.valueparameterUnit.asString(),
			               currentParamDef.levelName.asString(""),
			               currentParamDef.levelFrom,
			               currentParamDef.levelTo,
			               currentParamDef.levelUnit.asString(""),
			               currentParamDef.valueScale,
			               currentParamDef.valueOffset,
			               currentParamDef.dataVersion,
			               currentParamDef.compare,
			               currentParamDef.compareValue
			               );
	
	std::list<std::string> provider( currentParamDefProvider );
	currentParamDef.clear();
	currentParamDefProvider.clear();
	ostringstream err;

	if( ! currentParamDefConfig.addParamDef( currentParamDefsId, pd, provider, currentParamDefOverrid, err ) )
	{
	   error( err.str() );
	   return false;
	}
	
	return true;
}

void 
ConfigParser::
startElement( const std::string &fullname,
  	           const AttributeMap &attributes )
{
	AttributeMap::const_iterator itKey;
	string val;
	inChardata = false;

#if 0
  	cerr << "startElement: " << fullname << endl;
  	cerr << "  attributer: \n";

	for( AttributeMap::const_iterator it=attributes.begin();
	     it != attributes.end();
		  ++it)
    cerr <<"      " << it->first  << "=" << it->second << endl;
#endif
	
	xmlState.push( fullname );
	
	//cerr << "State: " << state.path() << endl;
	if( xmlState == "/wdb2ts" ) {
		doWdb2ts(attributes);
	} else if( xmlState == "/wdb2ts/include" ) {
	   doInclude( attributes );
	}else if( xmlState == "/wdb2ts/requests/request" ) {
		doRequest( attributes ); 
	} else if( xmlState == "/wdb2ts/requests/request/actionparam" ) {
		doRequestDefaultActionParam( attributes );
	} else if( xmlState == "/wdb2ts/requests/request/version" ) {
		doRequestVersion( attributes );
	} else if( xmlState == "/wdb2ts/requests/request/meta/update" ||
	       xmlState == "/wdb2ts/requests/request/version/meta/update" ) {

	} else if( xmlState == "/wdb2ts/requests/request/meta/update/name" ||
         xmlState == "/wdb2ts/requests/request/version/meta/update/name" ) {
   } else if( xmlState == "/wdb2ts/requests/request/version/actionparam" ) {
		if( currentRequestVersion )
			doRequestActionParam( attributes,  currentRequestVersion->actionParam );
		else 
			warning("/wdb2ts/requests/request/version/actionparam: No currentRequestVersion!");
	} else if( xmlState == "/wdb2ts/querydefs/querydef" ) {
		currentQueryAutoId=0;
		currentQueryDefWdbdb.erase();
		doQueryDef( attributes );
	} else if( xmlState == "/wdb2ts/querydefs/querydef/query" ) {
		++currentQueryAutoId;
		doQuery( attributes );
		inChardata = false;
		chardata.str("");
	} else if( xmlState == "/wdb2ts/paramdefs" ||
			   xmlState == "/wdb2ts/requests/request/paramdefs" ||
			   xmlState == "/wdb2ts/requests/request/version/paramdefs") {
	   doParamDefs( attributes );
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef" ||
			   xmlState == "/wdb2ts/requests/request/paramdefs/paramdef" ||
			   xmlState == "/wdb2ts/requests/request/version/paramdefs/paramdef") {
		doParamDef( attributes );
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef/valueparameter" ) {
		doValueParameter( attributes );
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef/levelparameter" ) {
		doLevelParameter( attributes );
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef/value" ) {
		doValue( attributes );
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef/dataversion" ) {
		doDataVersion( attributes );
	} else if( xmlState == "./parameter") {
	   string name;
	   getAttr( attributes, "name", name, "" );
	   if( name.empty() ) {
	      cerr << "ERROR: Parameter: <" << xmlState.path() << "> Missing attr 'name'" << endl;
	   } /*else {
	      cerr << "startElement: Parameter: <" << xmlState.path() << ">." << endl;
	   }*/
	} else if( xmlState == "./providerpriority/provider") {

	}
	




}

void 
ConfigParser::
endElement( const std::string &name )
{
	//string xmlState;
	string stateVal;
	string error;
	
	inChardata = false;
	string buf=chardata.str();
	chardata.str("");
	
	//cerr << "endelement: " << name << endl;
	
	//cerr << "EndElement <" << name << "> State: " << xmlState << endl;
	
	if( xmlState == "/wdb2ts/requests/request/version" ) {
		if( currentRequest ) {
			//cerr << "*** currentRequestVersion: " << currentRequestVersion->version << endl;
			
			if( ! currentRequestVersion->version.invalid() &&
				 !	currentRequestVersion->version.defaultVersionHighest() ) 
				if( ! currentRequest->addRequestVersion( currentRequestVersion, error ) )
					warning( error );

			if( ! currentRequestVersion->schema.defined() &&
			      currentRequest->requestDefault.schema.defined() )
			   currentRequestVersion->schema = currentRequest->requestDefault.schema;
		}
		currentRequestVersion.reset();
	} else if( xmlState == "/wdb2ts/requests/request" ) {
		currentRequest->resolve();
		addRequest( config->requests, currentRequest );
		currentRequest.reset();
	} else if(xmlState == "/wdb2ts/querydefs/querydef" ){
		currentQueryDefWdbdb.erase();
		if(providerAsWdbid && itCurrentQueryDef->second.dbRequestsInParalells()==0 ) {
			int nParallels=itCurrentQueryDef->second.querys().size();
			itCurrentQueryDef->second.dbRequestsInParalells(nParallels);
		}
		itCurrentQueryDef = config->querys.end();
   } else if(xmlState == "/wdb2ts/querydefs/querydef/query" ) {
		miutil::trimstr( buf );
		miutil::replaceString(buf, "\n", "");
		miutil::compresSpace( buf );
		miutil::replaceString(buf, " ,", ",");
		miutil::replaceString(buf, "; ", ";");

		if( !buf.empty() ) {
			if( itCurrentQueryDef != config->querys.end() ){
				if( providerAsWdbid && currentQueryWdbdb.empty()) {
					wdb2ts::WciWebQuery q;
					try {
						q.onlyDecodeParams(buf);

						if( q.dataprovider.valueList.empty() ) {
							currentQueryWdbdb = currentQueryDefWdbdb;
							warning("No dataprovider given in section: '" + buf +"'" );
						} else if( q.dataprovider.valueList.size()>1) {
							warning("More than one dataprovider given in section: '" + buf +"' using the first: '"+ *q.dataprovider.valueList.begin()+"'");
							currentQueryWdbdb = *q.dataprovider.valueList.begin();
						} else {
							currentQueryWdbdb = *q.dataprovider.valueList.begin();
						}
					}
					catch( const std::exception &ex) {
						cerr << "Exception: " << ex.what() << endl;
						warning(string("Failed to parse query section: ") + ex.what() );
					}
				}

				itCurrentQueryDef->second.push_back(
						Config::QueryElement(
								buf, currentQueryProbe, currentQueryStopIfData,
								currentQueryWdbdb, currentQueryPrognosisLengthSeconds,
								currentQueryId
						)
				);
			}
		}
	} else if( xmlState == "/wdb2ts/paramdefs" ) {
		ostringstream err;
	   config->paramdef.merge( &currentParamDefConfig, true );
	} else if( xmlState == "/wdb2ts/requests/request/paramdefs" ) {
		currentRequest->requestDefault.paramdef.merge( &currentParamDefConfig, true );
	} else if( xmlState == "/wdb2ts/requests/request/version/paramdefs" ) {
		currentRequestVersion->paramdef.merge( &currentParamDefConfig, true );
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef" ||
		       xmlState == "/wdb2ts/requests/request/paramdefs/paramdef" ||
		       xmlState == "/wdb2ts/requests/request/version/paramdefs/paramdef") {
		addParamDef();
	}else if( xmlState == "./meta/update" ) {

   }else if( xmlState == "./meta/update/name" ) {

   }else if( xmlState == "./parameter") {
	   //cerr << "endElement: Parameter: <" << xmlState.path() << ">." << endl;
	} else if( xmlState == "./providerpriority" ) {
	   //cerr << "endElement: providerpriority: <" << xmlState.path() << ">." << endl;
	} else if( xmlState == "./providerpriority/provider" ) {
	   //cerr << "endElement: providerpriority/provider: <" << xmlState.path() << ">." << endl;
   }

	xmlState.pop( stateVal );	
}

void
ConfigParser::
setBasedir( const std::string basedir )
{
	this->basedir = basedir;
}

std::string
ConfigParser::
getBasedir()const
{
	return basedir;
}

Config* 
ConfigParser::
parseFile( const std::string &filename_ )
{
	std::string content;
	std::string filename( checkPath( filename_ ) );

	if( filename.empty() )
		return 0;
	
	try{
		config = new Config();
	}
	catch( ... ) {
		fatalError( "Out of memmory.");
		return 0;
	}
	// cerr << "Parsing file: " << filename << endl;
	
	if( ! miutil::readFile( filename, content) ) {
		fatalError( "Cant read file: " + filename );
		return 0;
	}
	
	//cerr << "Content[" << endl << content << endl <<"===============" << endl;
	
	if( parse( content ) )
		return config;
	
	delete config;
	return 0;
}

Config* 
ConfigParser::
parseBuf( const std::string &buf )
{
	try{
		config = new Config();
	}
	catch( ... ) {
		fatalError( "Out of memmory.");
		return 0;
	}
	
	if( parse( buf ) )
		return config;
	
	delete config;
	return 0;
}


}
}
