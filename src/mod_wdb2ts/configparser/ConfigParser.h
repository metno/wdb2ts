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
	bool providerAsWdbid;
	std::ostringstream chardata;
	bool inChardata;
	State xmlState;
	bool invalidRequest;
	boost::shared_ptr<Request> currentRequest;
	boost::shared_ptr<RequestConf> currentRequestVersion;
	Config::QueryDefs::iterator itCurrentQueryDef;
	std::string currentQueryDefWdbdb;
	int currentQueryDefPrognosisLengthSeconds;
	std::string currentQueryId;
	int currentQueryAutoId;
	bool currentQueryProbe;
	bool currentQueryStopIfData;
	int currentQueryPrognosisLengthSeconds;
	ParamDefConfig currentParamDefConfig;
	ParamDef currentParamDef;
	std::string currentParamDefsId;
	bool currentParamDefOverrid;
	std::string currentQueryWdbdb;
	std::list<std::string> currentParamDefProvider;
	int recursionDepth; //Used to break out of circular reading of configuration files.
	std::string basedir;
	std::string checkPath( const std::string &filename );
	bool mergeConfig( Config *config );
	Config* readFile( const std::string &filename );
	int  getPrognosisLength( const AttributeMap &atributes, int defaultValue );
	bool doWdb2ts( const AttributeMap &atributes );
	bool doInclude( const AttributeMap &atributes );
	bool doRequestDefaultActionParam( const AttributeMap &atributes );
	bool doRequestActionParam( const AttributeMap &attributes, ActionParam &actionParam );
	void doRequestConf( const AttributeMap &attributes, RequestConf &conf, const std::string &versionAttrName );
	bool doRequest( const AttributeMap &atributes );
	bool doRequestVersion( const AttributeMap &attributes );
	void doQuery( const AttributeMap &attributes );
	bool doQueryDef( const AttributeMap &attributes );
	bool doParamDef( const AttributeMap &attributes );
	bool doParamDefs( const AttributeMap &attributes );
	bool doValueParameter( const AttributeMap &attributes );
	bool doLevelParameter( const AttributeMap &attributes );
	bool doValue(  const AttributeMap &attributes );
	bool doDataVersion( const AttributeMap &attributes );
	bool addParamDef();
   
public:
	ConfigParser();
	ConfigParser( const std::string &basedir, bool providerAsWdbidDefaultValue=false);
	~ConfigParser();
	
	virtual void characters( const std::string &buf );
	virtual void startElement( const std::string &fullname,
		  	                     const AttributeMap &atributes );
	virtual void endElement( const std::string &name );
	
	void setBasedir( const std::string basedir );
	std::string getBasedir()const;
	Config* parseFile( const std::string &filename );
	Config* parseBuf( const std::string &buf );
};


}
}


#endif
