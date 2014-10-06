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

#include <algorithm>
#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <NextRun.h>

using namespace std;


namespace wdb2ts {
namespace config {

TimePeriode::
TimePeriode( const boost::posix_time::time_duration &from,
             const boost::posix_time::time_duration &to )
   : to( to )
{
   boost::posix_time::time_duration c24( 24, 0, 0 );


   if( from > to ) {
      dif = c24 - from + to;
      dif *= -1;
   } else {
      dif = to - from;
   }
}

bool
TimePeriode::
inPeriode( const boost::posix_time::time_duration &time )const
{
   boost::posix_time::time_duration d;

   if( dif.is_negative() ) {
      if( time > to ) {
         boost::posix_time::time_duration c24( 24, 0, 0 );
         d = c24 - time + to;
      } else {
         d = to - time;
      }

      d *= -1;
      return d >= dif;
   } else {
      d = to - time;

      if( d.is_negative() )
         return false;

      return d <= dif;
   }
}

bool
TimePeriode::
operator<( const TimePeriode &rhs )const
{

   if( to < rhs.to ) {
      return true;
   } else if( to == rhs.to ) {
      boost::posix_time::time_duration d=dif;
      boost::posix_time::time_duration rd=rhs.dif;

      if( d.is_negative() )
         d *= -1;

      if( rd.is_negative() )
         rd *= -1;

      return d < rd;
   }else {
      return false;
   }
}

bool
TimePeriode::
operator==(const TimePeriode &rhs)const
{
   if( to == rhs.to ) {
      boost::posix_time::time_duration d=dif;
      boost::posix_time::time_duration rd=rhs.dif;

      if( d.is_negative() )
         d *= -1;

      if( rd.is_negative() )
         rd *= -1;

      return d == rd;
   }else {
      return false;
   }
}

bool
TimePeriode::
operator!=(const TimePeriode &rhs)const
{
   return ! ( *this == rhs );
}

bool
TimePeriode::
overlap( const TimePeriode &other )
{
   return false;
}

boost::posix_time::ptime
TimePeriode::
refto( const boost::posix_time::ptime &reftime,
       boost::posix_time::ptime &fromTime )const
{
   using namespace boost::posix_time;
   using namespace boost::gregorian;

   time_duration clock( reftime.time_of_day() );
   ptime retTime;
   date d=reftime.date();

   fromTime = ptime();

   if( inPeriode( clock ) ) {
      if( dif.is_negative() && clock > to )
         d += days( 1 );

      retTime = ptime( d, to );
   } else if( dif.is_negative() ) {
      retTime = ptime( d + days(1), to );
   } else {

      if( clock > to )
         d += days( 1 );

      retTime = ptime( d, to );
   }

   fromTime = retTime;

   if( dif.is_negative() )
      fromTime += dif;
   else
      fromTime -= dif;

   return retTime;
}

UpdateGroup::
UpdateGroup( const std::string &name )
   : name( name )
{
}

void
UpdateGroup::
addProvider( const std::string &provider )
{
}

void
UpdateGroup::
addTimePeriod( const TimePeriode &timePeriod )
{
   timeList.push_back( timePeriod );

   timeList.sort();

//   cerr << "addTimePeriod: #: " << timeList.size() << endl;
//   for( std::list<TimePeriode>::const_iterator it=timeList.begin(); it != timeList.end(); ++it ) {
//      cerr << "  " << it->getTo() << "  d: " << it->getDif() << endl;
//   }

}

boost::posix_time::ptime
UpdateGroup::
nextRun( const boost::posix_time::ptime &reqTime,
         boost::posix_time::ptime &fromtime )const
{
   boost::posix_time::time_duration td;
   bool inPeriod;
   const TimePeriode *tp = findTimePeriod( reqTime.time_of_day(), inPeriod );

   if( ! tp )
      return boost::posix_time::ptime();

   return tp->refto( reqTime, fromtime );
}

const TimePeriode*
UpdateGroup::
findTimePeriod( const boost::posix_time::time_duration &time,
                bool &inPeriod )const
{
   std::list<TimePeriode>::const_iterator it=timeList.begin();
   inPeriod = false;

   if( it == timeList.end() )
      return 0;

   if( it->inPeriode( time ) ) {
      inPeriod = true;
      return &(*it);
   }

   ++it;

   for( ;it != timeList.end(); ++it ) {
      if( it->inPeriode( time ) ) {
         inPeriod = true;
         return &(*it);
      }

      if( time < it->getTo())
         return &(*it);
   };

   return &(*timeList.begin());
}

boost::posix_time::ptime
UpdateGroup::
currentUpdate( const boost::posix_time::ptime &now )
{
   return currentUpdate_;
}

Update::
Update()
{
}

void
Update::
addUpdateGroup( const UpdateGroup &group )
{
}

UpdateGroup*
Update::
findUpdateGroup( const std::string &provider )
{
   return 0;
}

}
}

