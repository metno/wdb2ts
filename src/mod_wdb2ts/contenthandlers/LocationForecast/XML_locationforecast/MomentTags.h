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

#ifndef __WDB2TS_MOMENTTAGS_H__
#define __WDB2TS_MOMENTTAGS_H__

#include <float.h>
#include <math.h>
#include <IXmlTemplate.h>
#include <PointDataHelper.h>
#include <LocationData.h>
#include <SymbolContext.h>
#include <ProjectionHelper.h>

namespace wdb2ts {


class MomentTags : public IXmlTemplate 
{
	float ff;
	float dd;
	const LocationElem *pd;
	SymbolContext *symbolContext;
	const ProjectionHelper *projectionHelper;
	
public:
	MomentTags( ): pd(0), symbolContext( 0 ){}
	
	MomentTags( const LocationElem &pointData, 
			      SymbolContext &context, 
			      const ProjectionHelper *projectionHelper_ )
		: symbolContext( &context ), projectionHelper( projectionHelper_ )
		{
			init( pointData );
		}
	
	virtual ~MomentTags(){
	}
	
	void init( const LocationElem &pointData );
	void computeWind( float u, float v );
	
	/**
	 * If the provider have symboldata, read it.
	 * Correct the symbols if it has precipitation
	 * so it has correct agregate state: snow, sleet or precipitation. 
	 * The sun state is also corrected with polar night information. 
	 */
	void doSymbol( float temperature );
	
	float getTemperatureProability( float temp )const;
		
	virtual void output( std::ostream &out, const std::string &indent );
};

}


#endif 
