#ifndef __GMLCONTEXT_H__
#define __GMLCONTEXT_H__

#include <SymbolContext.h>
#include <ProjectionHelper.h>
#include <WeatherSymbol.h>
#include <map>
#include <string>
namespace wdb2ts {

class GMLContext 
{
	GMLContext();
	GMLContext( const GMLContext & );
	GMLContext& operator=( const GMLContext & );
	

public:
	typedef enum { OceanForecast, LocationForecast } ForecastType;
	
	//Helper to create the symbol data from providers
	//that has encoded symbol fields.
	SymbolContext symbolContext;
	const ProjectionHelper *projectionHelper;
	SymbolDataElement *symData;
	std::map<std::string, int > idMap;
	ForecastType forecastType;
	
	GMLContext( const ProjectionHelper *projectionHelper_, bool useTempInFromtime )
		: symbolContext( useTempInFromtime ), projectionHelper( projectionHelper_ ),
		  symData( 0 ), forecastType( OceanForecast )
	{
		
	}
	
	
};


}

#endif /*GMLCONTEXT_H_*/
