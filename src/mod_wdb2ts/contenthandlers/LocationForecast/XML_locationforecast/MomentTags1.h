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

#ifndef __WDB2TS_MOMENTTAGS1_H__
#define __WDB2TS_MOMENTTAGS1_H__

#include <float.h>
#include <math.h>
#include <IXmlTemplate.h>
#include <PointDataHelper.h>
#include <LocationData.h>
#include <SymbolContext.h>
#include <ProjectionHelper.h>
#include <WeatherSymbol.h>

namespace {
wdb2ts::SymbolDataElement dummy;
}

namespace wdb2ts {


class MomentTags1 : public IXmlTemplate
{
	LocationElem *pd;
	SymbolDataElement *symData;
	const ProjectionHelper *projectionHelper;
	
	void setPrecipitation();

public:
	MomentTags1():
		pd(0), symData( &dummy ), projectionHelper( 0 ){
	}
	
	MomentTags1( LocationElem &pointData,
			    const ProjectionHelper *projectionHelper_ )
		:  symData( &dummy ), projectionHelper( projectionHelper_ )
		{
			init( pointData );
		}

	MomentTags1( LocationElem &pointData,
				SymbolDataElement &symData_,
				const ProjectionHelper *projectionHelper_ )
			:  symData( &symData_ ), projectionHelper( projectionHelper_ )
			{
				init( pointData );
			}


	MomentTags1( LocationElem &pointData, SymbolDataElement &symData_ )
			: symData( &symData_ ), projectionHelper( 0 )
	{
		init( pointData );
	}

	
	virtual ~MomentTags1(){
	}
	
	void init( LocationElem &pointData );
	
	float getTemperatureProability( float temp, bool tryHard )const;
		
	virtual void output( std::ostream &out, const std::string &indent );
};

}


#endif 
