#include <iostream>
#include <stdarg.h>
#include <ctype.h>
#include <libxml/parser.h>
#include "SAXParser.h"
#include <trimstr.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

static void 
SAXstartDoc(void *user_data);
	  
static void 
SAXendDoc(void *user_data);
	  
static void 
SAXstartElement(void   *user_data,
	      		 const xmlChar *fullname,
					 const xmlChar **atts);
static void    
SAXendElement(void  *user_data,
	           const xmlChar *name);

static void        
SAXcharacters(void *user_data,
		        const xmlChar *ch,
				  int len);
	  
static void 
SAXerrorFunc(void *ctx,
	          const char *msg,
			    ...);

static void 
SAXwarningFunc(void *ctx,
	            const char *msg,
			      ...);

static void 
SAXfatalErrorSAXFunc( void *ctx,
							 const char *msg,
					       ...);


static xmlSAXHandler SAX_xmlSaxHandler={
	  0, //    internalSubsetSAXFunc internalSubset;
	  0, //    isStandaloneSAXFunc isStandalone;
	  0, //    hasInternalSubsetSAXFunc hasInternalSubset;
	  0, //    hasExternalSubsetSAXFunc hasExternalSubset;
	  0, //    resolveEntitySAXFunc resolveEntity;
	  0, //    getEntitySAXFunc getEntity;
	  0, //    entityDeclSAXFunc entityDecl;
	  0, //    notationDeclSAXFunc notationDecl;
	  0, //    attributeDeclSAXFunc attributeDecl;
	  0, //    elementDeclSAXFunc elementDecl;
	  0, //    unparsedEntityDeclSAXFunc unparsedEntityDecl;
	  0, //    setDocumentLocatorSAXFunc setDocumentLocator;
	  SAXstartDoc,     //    startDocumentSAXFunc startDocument;
	  SAXendDoc,       //    endDocumentSAXFunc endDocument;
	  SAXstartElement, //    startElementSAXFunc startElement;
	  SAXendElement,   //    endElementSAXFunc endElement;
	  0, //    referenceSAXFunc reference;
	  SAXcharacters,   //    charactersSAXFunc characters;
	  0, //    ignorableWhitespaceSAXFunc ignorableWhitespace;
	  0, //    processingInstructionSAXFunc processingInstruction;
	  0, //    commentSAXFunc comment;
	  SAXwarningFunc,       //    warningSAXFunc warning;
	  SAXerrorFunc,         //    errorSAXFunc error;
	  SAXfatalErrorSAXFunc, //    fatalErrorSAXFunc fatalError;
	  0, //    getParameterEntitySAXFunc getParameterEntity;
	  0, //    cdataBlockSAXFunc cdataBlock;
	  0, //    externalSubsetSAXFunc externalSubset;
	  1  //     int initialized;
};
#ifdef __cplusplus
}
#endif


namespace miutil {


SAXParser::
SAXParser() 
{
}

SAXParser::
~SAXParser()
{
}


void 
SAXParser::
startDocument()
{
  cerr << "startDocument!\n";
}

void 
SAXParser::
endDocument()
{
  cerr << "endDocument!\n";
}

void 
SAXParser::
startElement(const std::string &fullname,
		       const AttributeMap &atributes)
{
  	cerr << "startElement: " << fullname << endl;
  	cerr << "  attributer: \n";

	for( AttributeMap::const_iterator it=atributes.begin();
	     it != atributes.end();
		  ++it)
    cerr <<"      " << it->first  << "=" << it->second << endl;
}

void 
SAXParser::
endElement(const std::string &name )
{
	cerr << "endElement: " << name << endl;
}

void
SAXParser::
characters(const std::string &buf )
{ 
  cerr << "characters: [" << buf << "]\n";
  
}

void 
SAXParser::
error(const std::string &msg)
{

  errMsg << "xml error: "<< msg << endl;
}

void 
SAXParser::
warning(const std::string &msg)
{
  errMsg << "xml warning: "<< msg << endl;
}

void 
SAXParser::
fatalError(const std::string &msg)
{
  errMsg << "xml fatal error: "<< msg << endl;
}

bool
SAXParser::
getAttr( const AttributeMap &atributes, 
         const std::string &attr, std::string &val,
         const std::string &defaultVal )
{
	AttributeMap::const_iterator it = atributes.find( attr );
	
	if( it == atributes.end() ) {
		val = defaultVal;
		return false;
	}
	
	val = it->second;
	miutil::trimstr( val );
	
	if( val.empty() )
		val = defaultVal;
	
	return true;
}

bool
SAXParser::
hasAttr( const AttributeMap &atributes, 
         const std::string &att )
{
	string val;
	AttributeMap::const_iterator it = atributes.find( att );
	
	if( it == atributes.end() )
		return false;
	
	return true;
}



bool
SAXParser::
parse( const std::string &buf )
{
  int ret;
  errMsg.str("");
  ret=xmlSAXUserParseMemory(&SAX_xmlSaxHandler,
			                   this, buf.c_str(), buf.length());

  if(ret==0){
      return true;
  }

  return false;
}

}

#ifdef __cplusplus
extern "C" {
#endif

	

static void 
SAXstartDoc(void *user_data)
{
  ((miutil::SAXParser *)user_data)->startDocument();
}

static void 
SAXendDoc(void *user_data)
{
  ((miutil::SAXParser*)user_data)->endDocument();
}

static void 
SAXstartElement(void   *user_data,
		       const xmlChar *fullname,
		       const xmlChar **atts)
{
	miutil::SAXParser::AttributeMap attMap;
	string key;
	string val;
	
	for( int i=0; atts && atts[i]; ++i ) {
		if( i%2 == 0 )
			key = (const char*)atts[i];
		else {
			val = (const char*) atts[i];
			miutil::trimstr( val );
			attMap[key] = val;
		}
	}
		
  ((miutil::SAXParser*)user_data)->startElement( (const char*)fullname, attMap );
}

static void    
SAXendElement(void  *user_data,
           const xmlChar *name)
{
  ((miutil::SAXParser*)user_data)->endElement((const char*)name);
}

static void        
SAXcharacters( void *user_data,
            const xmlChar *ch,
            int len)
{
	std::string buf((const char*)ch, len);

  ((miutil::SAXParser*)user_data)->characters(buf);
}


static void 
SAXerrorFunc( void *ctx,
           const char *msg,
		     ...)
{
  va_list args;
  char errBuf[256];
  char *buf;
  int  n;
  string m;

  va_start(args, msg);
  
  n=vsnprintf(errBuf, 256, msg, args);

  if((n+1)>256){
    try{
      buf=new char[n+1];
      va_start(args, msg);
      n=vsnprintf(buf, n+1, msg, args);
      m=buf;
      delete buf;
    }
    catch(...){
       cerr << "SAXerrorFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);
  
  ((miutil::SAXParser*)ctx)->error(m);
}

static void 
SAXwarningFunc(void *ctx,
            const char *msg,
		      ...)
{
  va_list args;
  char errBuf[256];
  char *buf;
  int  n;
  string m;

  va_start(args, msg);
  
  n=vsnprintf(errBuf, 256, msg, args);

  if((n+1)>256){
    try{
      buf=new char[n+1];
      va_start(args, msg);
      n=vsnprintf(buf, n+1, msg, args);
      m=buf;
      delete buf;
    }
    catch(...){
       cerr << "SAXwarningFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);

  ((miutil::SAXParser*)ctx)->warning(m);
}

static void 
SAXfatalErrorSAXFunc(void *ctx,
   				   const char *msg,
	   			   ...)
{
  va_list args;
  char errBuf[256];
  char *buf;
  int  n;
  string m;

  va_start(args, msg);
  
  n=vsnprintf(errBuf, 256, msg, args);

  if((n+1)>256){
    try{
      buf=new char[n+1];
      va_start(args, msg);
      n=vsnprintf(buf, n+1, msg, args);
      m=buf;
      delete buf;
    }
    catch(...){
       cerr << "SAXFatalErrorSAXFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);


  ((miutil::SAXParser*)ctx)->fatalError(m);
}

#ifdef __cplusplus
}
#endif

