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
}

void
ConfigParser::
doQuery( const AttributeMap &attributes )
{
	string probe;
	getAttr( attributes, "must_have_data", probe, "false" );
	
	if( !probe.empty() && (probe[0]=='t' || probe[0]=='T') )
		currentQueryProbe = true;
	else
		currentQueryProbe = false;
}

bool
ConfigParser::
doQueryDef( const AttributeMap &attributes )
{
	string id;
	
	if( ! getAttr( attributes, "id", id) || id.empty() ) {
		error("Mandatory 'querydef' attribute 'id' missiung.");
		return false;
	}
	
	itCurrentQueryDef = config->querys.find( id );
	
	if( itCurrentQueryDef != config->querys.end() ){
		error("Querydef: id: '"+id+"' alleady defined.");
		return false;
	}
	
	config->querys[id]=Config::Query();
	itCurrentQueryDef = config->querys.find( id );
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
		
		if(  wdb2ts::hasParam( config->paramDefs, id, *it ) ) {
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
	
	if( provider.empty() ) {
		if( ! wdb2ts::addParamDef( config->paramDefs, pd, "" ) ) {
			error( "addParam: ParamDef '"+alias+"' provider: 'default' allready defined.");
			return false;
		}
	} else {
		for( std::list<std::string>::iterator it = provider.begin();
		     it != provider.end();
		     ++it )
		{
			if( ! wdb2ts::addParamDef( config->paramDefs, pd, *it ) ) {
				error( "addParam: ParamDef '"+alias+"' provider: '" + *it +"' allready defined.");
				return false;
			}
		}
	}
	
	return true;
}

void 
ConfigParser::
startElement( const std::string &fullname,
  	           const AttributeMap &attributes )
{
	string xmlState;
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
	
	state.push( fullname );
	xmlState = state.path();
	
	//cerr << "State: " << state.path() << endl;
	
	if( xmlState == "/wdb2ts/requests/request" ) {
		doRequest( attributes ); 
	} else if( xmlState == "/wdb2ts/requests/request/actionparam" ) {
		doRequestDefaultActionParam( attributes );
	} else if( xmlState == "/wdb2ts/requests/request/version" ) {
		doRequestVersion( attributes );
	} else if( xmlState == "/wdb2ts/requests/request/version/actionparam" ) {
		if( currentRequestVersion )
			doRequestActionParam( attributes,  currentRequestVersion->actionParam );
		else 
			warning("/wdb2ts/requests/request/version/actionparam: No currentRequestVersion!");
	} else if( xmlState == "/wdb2ts/querydefs/querydef" ) {
		doQueryDef( attributes );
	} else if( xmlState == "/wdb2ts/querydefs/querydef/query" ) {
		doQuery( attributes );
		inChardata = false;
		chardata.str("");
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
	}
	




}

void 
ConfigParser::
endElement( const std::string &name )
{
	string xmlState;
	string stateVal;
	string error;
	
	inChardata = false;
	string buf=chardata.str();
	chardata.str("");
	
	//cerr << "endelement: " << name << endl;
	
	xmlState = state.path();
	
	//cerr << "EndElement <" << name << "> State: " << xmlState << endl;
	
	if( xmlState == "/wdb2ts/requests/request/version" ) {
		if( currentRequest ) {
			//cerr << "*** currentRequestVersion: " << currentRequestVersion->version << endl;
			
			if( ! currentRequestVersion->version.invalid() &&
				 !	currentRequestVersion->version.defaultVersionHighest() ) 
				if( ! currentRequest->addRequestVersion( currentRequestVersion, error ) )
					warning( error );
		}
		currentRequestVersion.reset();
	} else if( xmlState == "/wdb2ts/requests/request" ) {
		currentRequest->resolve();
		addRequest( config->requests, currentRequest );
		currentRequest.reset();
	} else if(xmlState == "/wdb2ts/querydefs/querydef" ){
		itCurrentQueryDef = config->querys.end();
   } else if(xmlState == "/wdb2ts/querydefs/querydef/query" ) {
		miutil::trimstr( buf );
		miutil::replace(buf, "\n", "");
		miutil::compresSpace( buf );
		miutil::replace(buf, " ,", ",");
		miutil::replace(buf, "; ", ";");

		if( !buf.empty() ) {
			if( itCurrentQueryDef != config->querys.end() ){
				itCurrentQueryDef->second.push_back( Config::QueryElement(buf, currentQueryProbe ) );
			}
		}
	} else if( xmlState == "/wdb2ts/paramdefs/paramdef" ) {
		addParamDef();
	}
		
	state.pop( stateVal );	
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
