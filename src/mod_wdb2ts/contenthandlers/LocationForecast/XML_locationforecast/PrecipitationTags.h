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

#ifndef __WDB2TS_PRECIPITATIONTAGS_H__
#define __WDB2TS_PRECIPITATIONTAGS_H__

#include <float.h>
#include <math.h>
#include <IXmlTemplate.h>
#include <PointDataHelper.h>

namespace wdb2ts {


class PrecipitationTags : public IXmlTemplate 
{
	float value;
	float min;
	float max;
	float prob;
public:
	PrecipitationTags( ): value( FLT_MAX ){}
	
	PrecipitationTags( float value, float min=FLT_MAX, float max=FLT_MAX, float prob=FLT_MAX )
		: value( value ), min( min ), max( max ), prob( prob )
		{
		}
	
	virtual ~PrecipitationTags(){
	}
	
	void init( float value );
	
	virtual void output( std::ostream &out, const std::string &indent );
};

}


#endif 
