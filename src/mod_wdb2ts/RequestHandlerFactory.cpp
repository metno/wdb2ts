#include <RequestHandlerFactory.h>
#include <contenthandlers/Location/LocationHandler.h>
#include <contenthandlers/LocationForecast/LocationForecastHandler.h>
#include <contenthandlers/LocationForecast/LocationForecastUpdateHandler.h>
#include <contenthandlers/LocationForecastGml/LocationForecastGmlHandler.h>

namespace wdb2ts {

webfw::RequestHandler*
requestHandlerFactory( const std::string &id, 
                       const wdb2ts::config::Version &ver )
{
	if( id == "Location" ) 
		return new LocationHandler( ver.majorVer, ver.minorVer );
	else if( id == "LocationForecast") 
		return new LocationForecastHandler( ver.majorVer, ver.minorVer );
	else if( id == "LocationForecast2")
		return new LocationForecastHandler( ver.majorVer, ver.minorVer, "2" );
	else if( id == "LocationForecast3")
	   return new LocationForecastHandler( ver.majorVer, ver.minorVer, "3" );
	else if( id == "LocationForecastUpdate" )
		return new LocationForecastUpdateHandler( ver.majorVer, ver.minorVer );
	else if( id == "LocationForecastGml" )
			return new LocationForecastGmlHandler( ver.majorVer, ver.minorVer );
	else
		return 0;
}

};
