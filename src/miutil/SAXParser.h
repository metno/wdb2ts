#ifndef __WDB2TS_SAXPARSER_H__
#define __WDB2TS_SAXPARSER_H__

#include <string>
#include <map>
#include <sstream>

namespace miutil {



class SAXParser
{
  bool   errFlag;
  std::ostringstream errMsg;
  std::ostringstream inputBuf;
  
public:
	typedef std::map<std::string, std::string> AttributeMap;
	
	SAXParser();
	virtual ~SAXParser();

	virtual void startDocument();
	virtual void endDocument();
	virtual void startElement( const std::string &fullname,
	  	                        const AttributeMap &attributes );
	virtual void endElement( const std::string &name );
	virtual void characters( const std::string &buf );
  
	virtual void error(const std::string &msg);
	virtual void warning(const std::string &msg);
	virtual void fatalError(const std::string &msg);
	
	/**
	 * return true if the attribute 'attr' is in the AtributeMap. The 
	 * value is returned in 'val'. If 'attr' is not in the attributes or the value is 
	 * an empty string the 'defaultValue' is returned in 'val'.
	 * 
	 * @param[in] attributes The attribute map to look in.
	 * @param[in] attr The attribute to look for.
	 * @param[out] val The attribute value.
	 * @param[in] defaultVal The value to return if the attr is not in the attributes map
	 *   or the value is an empty string. 
	 */
	bool getAttr( const AttributeMap &attributes, 
		           const std::string &attr, std::string &val, 
			        const std::string &defaultVal="");
		
	bool hasAttr( const AttributeMap &attributes, 
         		  const std::string &att );
	
	std::string getErrMsg()const{ return errMsg.str();}

	bool parse( const std::string &buf );
};

}

#endif 
