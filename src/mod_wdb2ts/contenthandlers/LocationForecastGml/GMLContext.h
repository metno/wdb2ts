#ifndef __GMLCONTEXT_H__
#define __GMLCONTEXT_H__

#include <SymbolContext.h>
#include <ProjectionHelper.h>
#include <map>
#include <string>
namespace wdb2ts {

class GMLContext 
{
	GMLContext();
	GMLContext( const GMLContext & );
	GMLContext& operator=( const GMLContext & );
	
public:
	
	//Helper to create the symbol data from providers
	//that has encoded symbol fields.
	SymbolContext symbolContext;
	const ProjectionHelper *projectionHelper;
	std::map<std::string, int > idMap;
	
	GMLContext( const ProjectionHelper *projectionHelper_ )
		: projectionHelper( projectionHelper_ )
	{
		
	}
	
	
};


}

#endif /*GMLCONTEXT_H_*/
