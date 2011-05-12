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
#include <readfile.h>
#include <ConfigParser.h>
#include <trimstr.h>
#include <replace.h>
#include <splitstr.h>
#include <compresspace.h>

namespace wdb2ts {
namespace config {

using namespace std;

ConfigParser::
ConfigParser()
	:config( 0 ), inChardata( false )
	 
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

bool
ConfigParser::
doInclude( const AttributeMap &attributes )
{
   std::string file;
   if( ! getAttr( attributes, "file", file ) || file.empty() ) {
      error("Include statement. Missing mandatory attribute 'file'.");
      return false;
   }

   ostringstream ost;
   ConfigParser myParser;
   Config *myConfig = myParser.parseFile( file );


   if( ! myConfig ) {
      ost << "Parsing of file <" << file << "> failed.";
      error( ost.str().c_str() );
      return false;
   } else {
      std::auto_ptr<Config> p( myConfig ); //Delete myConfig on return.
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
			conf.version = Version(it->second);
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
	string probe;
	string stopIfData;

	getAttr( attributes, "must_have_data", probe, "false" );
	
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

	if( currentQueryWdbdb.empty() )
		currentQueryWdbdb = currentQueryDefWdbdb;
}

bool
ConfigParser::
doQueryDef( const AttributeMap &attributes )
{
	string id;
	string sParalell;
	

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

	getAttr( attributes, "wdbdb", currentQueryDefWdbdb, "");


	if( hasAttr( attributes, "parallel" ) )
	   getAttr( attributes, "parallel", sParalell, "1");
	else
	   getAttr( attributes, "paralell", sParalell, "1");

	try {
		miutil::Value val( sParalell );
		int n = val.asInt();

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
		
		if(  config->paramDefs.hasParam( id, *it ) ) {
			currentParamDefProvider.clear();
			error("doParamDef id: '" + id + "' provider: '" + *it + "' allready defined.");
			return false;
		}
		
		currentParamDefProvider.push_back( *it );
	}
	
	currentParamDef.id = id;
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
	miutil::Value vFrom;
	miutil::Value vTo;
	bool ret=true;
	
	if( ! getAttr( attributes, "name", name) || name.empty() ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. Mandatory 'levelparameter' attribute 'name' missing.");
		return false;
	}
	
	if( ! getAttr( attributes, "unit", unit)  ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. Mandatory 'levelparameter' attribute 'unit' missing.");
		return false;
	}
	
	if( ! getAttr( attributes, "from", from ) || from.empty() ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. Mandatory 'levelparameter' attribute 'from' missing.");
		return false;
	}
	
	if( ! getAttr( attributes, "to", to ) || to.empty() ) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"'. Mandatory 'levelparameter' attribute 'to' missing.");
		return false;
	}
	
	vFrom = from;
	vTo = to;
	
	try {
			currentParamDef.levelTo = vTo.asInt();
	}
	catch( std::exception & ex) {
			error("ParamDef id: '"+currentParamDef.id.asString()+"' : levelparameter invalid attribute value 'to' '"+ to + "' " + ex.what() +".");
			ret = false;
	}
	
	try {
		currentParamDef.levelFrom = vFrom.asInt();
	}
	catch( std::exception & ex) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"' : levelparameter invalid attribute value 'from' '"+ from + "' " + ex.what() +".");
		ret = false;
	}
	
	currentParamDef.levelName = name;
	currentParamDef.levelUnit = unit;
	
	return true;
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
		currentParamDef.valueScale = vScale.asFloat();
	}
	catch( std::exception & ex) {
		error("ParamDef id: '"+currentParamDef.id.asString()+"' : value element invalid attribute value scale '"+ scale + "' " + ex.what() +".");
		ret = false;
	}
	
	try {
		currentParamDef.valueOffset = vOffset.asFloat();
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
			currentParamDef.compareValue = vNull.asInt();
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
		currentParamDef.dataVersion = vValue.asInt();
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
	
	if( ! currentParamDef.valid() ) {
		error("Some invalid values for paramdef id '"+alias+"'.");
		currentParamDef.clear();
		currentParamDefProvider.clear();
		return false;
	}
	
	wdb2ts::ParamDef pd( alias,
			               currentParamDef.valueparameterName.asString(),
			               currentParamDef.valueparameterUnit.asString(),
			               currentParamDef.levelName.asString(),
			               currentParamDef.levelFrom,
			               currentParamDef.levelTo,
			               currentParamDef.levelUnit.asString(),
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

	if( ! config->addParamDef( currentParamDefsId, pd, provider, err ) ) {
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
	if( xmlState == "/wdb2ts/include" ) {
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
		currentQueryDefWdbdb.erase();
		doQueryDef( attributes );
	} else if( xmlState == "/wdb2ts/querydefs/querydef/query" ) {
		doQuery( attributes );
		inChardata = false;
		chardata.str("");
	} else if( xmlState == "/wdb2ts/paramdefs" ) {
	   getAttr( attributes, "id", currentParamDefsId, "" );
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef" ) {
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
	   } else {
	      cerr << "startElement: Parameter: <" << xmlState.path() << ">." << endl;
	   }
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
		itCurrentQueryDef = config->querys.end();
   } else if(xmlState == "/wdb2ts/querydefs/querydef/query" ) {
		miutil::trimstr( buf );
		miutil::replace(buf, "\n", "");
		miutil::compresSpace( buf );
		miutil::replace(buf, " ,", ",");
		miutil::replace(buf, "; ", ";");

		if( !buf.empty() ) {
			if( itCurrentQueryDef != config->querys.end() ){
				itCurrentQueryDef->second.push_back( Config::QueryElement(buf, currentQueryProbe, currentQueryStopIfData, currentQueryWdbdb ) );
			}
		}
	} else if( xmlState == "/wdb2ts/paramdefs" ) {
	   currentParamDefsId.erase();
   } else if( xmlState == "/wdb2ts/paramdefs/paramdef" ) {
		addParamDef();
	}else if( xmlState == "./meta/update" ) {

   }else if( xmlState == "./meta/update/name" ) {

   }else if( xmlState == "./parameter") {
	   cerr << "endElement: Parameter: <" << xmlState.path() << ">." << endl;
	} else if( xmlState == "./providerpriority" ) {
	   cerr << "endElement: providerpriority: <" << xmlState.path() << ">." << endl;
	} else if( xmlState == "./providerpriority/provider" ) {
	   cerr << "endElement: providerpriority/provider: <" << xmlState.path() << ">." << endl;
   }

	xmlState.pop( stateVal );	
}



Config* 
ConfigParser::
parseFile( const std::string &filename )
{
	std::string content;
	
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
