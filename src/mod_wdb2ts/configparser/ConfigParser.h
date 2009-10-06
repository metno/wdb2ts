#ifndef __CONFIGPARSER_H__
#define __CONFIGPARSER_H__

#include <sstream>
#include <list>
#include <SAXParser.h>
#include <Config.h>
#include <State.h>

namespace wdb2ts {
namespace config {

class ConfigParser : public miutil::SAXParser
{
	Config *config;
	std::ostringstream chardata;
	bool inChardata;
	State state;
	bool invalidRequest;
	boost::shared_ptr<Request> currentRequest;
	boost::shared_ptr<RequestConf> currentRequestVersion;
	Config::QueryDefs::iterator itCurrentQueryDef;
	ParamDef currentParamDef;
	bool currentQueryProbe;
	bool currentQueryStopIfData;
	std::list<std::string> currentParamDefProvider;
	
	bool doRequestDefaultActionParam( const AttributeMap &atributes );
	bool doRequestActionParam( const AttributeMap &attributes, ActionParam &actionParam );
	void doRequestConf( const AttributeMap &attributes, RequestConf &conf, const std::string &versionAttrName );
	bool doRequest( const AttributeMap &atributes );
	bool doRequestVersion( const AttributeMap &attributes );
	void doQuery( const AttributeMap &attributes );
	bool doQueryDef( const AttributeMap &attributes );
   bool doParamDef( const AttributeMap &attributes );
   bool doValueParameter( const AttributeMap &attributes );
   bool doLevelParameter( const AttributeMap &attributes );
   bool doValue(  const AttributeMap &attributes );
   bool doDataVersion( const AttributeMap &attributes );
   bool addParamDef();
   
public:
	ConfigParser();
	~ConfigParser();
	
	virtual void characters( const std::string &buf );
	virtual void startElement( const std::string &fullname,
		  	                     const AttributeMap &atributes );
	virtual void endElement( const std::string &name );
	
	Config* parseFile( const std::string &filename );
	Config* parseBuf( const std::string &buf );
};


}
}


#endif
