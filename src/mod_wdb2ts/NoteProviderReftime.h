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


#ifndef __NOTEPROVIDERREFTIME_H__
#define __NOTEPROVIDERREFTIME_H__

#include <NoteTag.h>
#include <UpdateProviderReftimes.h>


#if 0
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/export.hpp>
#endif

namespace wdb2ts {

	class NoteProviderReftimes : virtual public NoteTag, public ProviderRefTimeList 
	{
#if 0		
		friend class boost::serialization::access;
		
		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & boost::serialization::base_object<NoteTag>(*this);
			ar & boost::serialization::base_object<ProviderRefTimeList>(*this);
		}
#endif
		public:
			NoteProviderReftimes(){}
			NoteProviderReftimes( const ProviderRefTimeList &reftimeList  )
				: ProviderRefTimeList( reftimeList )
				{}
				
			virtual char* noteType() const { return "NoteProviderReftimes"; }
			
			virtual bool saveNote( std::ostream &out );
			virtual NoteTag*  loadNote( std::istream &in );
	};
	


}

#if 0
BOOST_CLASS_EXPORT_GUID(wdb2ts::NoteProviderReftimes, "NoteProviderReftimes");
#endif

#endif
