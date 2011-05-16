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

#ifndef __wdb2ts_config_NEXTRUN_H__
#define __wdb2ts_config_NEXTRUN_H__


#include <string>
#include <list>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace wdb2ts {
namespace config {

   class TimePeriode {
      boost::posix_time::time_duration to;
      boost::posix_time::time_duration dif;

   public:
      TimePeriode( const boost::posix_time::time_duration &from,
                   const boost::posix_time::time_duration &to
                   );

      /**
       * Test the time agains this time period.
       *
       * @param time Is time in this time period.
       * @return true if in time periode and false otherwise.
       */
      bool inPeriode( const boost::posix_time::time_duration &time )const;

      bool operator==(const TimePeriode &rhs)const;

      bool operator!=(const TimePeriode &rhs)const;
      bool overlap( const TimePeriode &other );

      bool operator<( const TimePeriode &rhs )const;
      boost::posix_time::time_duration getTo() const { return to; };
      boost::posix_time::time_duration getDif() const { return dif; }

      boost::posix_time::ptime refto( const boost::posix_time::ptime &reftime,
                                      boost::posix_time::ptime &fromtime )const;
   };

   class UpdateGroup
   {
      std::string name;
      std::list<std::string> providerList;
      std::list<TimePeriode> timeList;
      boost::posix_time::ptime currentUpdate_;

   public:
      UpdateGroup( const std::string &name );

      std::list<TimePeriode> getTimeList()const { return timeList; }
      std::list<std::string> getProviderList()const { return providerList; }

      void addProvider( const std::string &provider );
      void addTimePeriod( const TimePeriode &timePeriod );

      /**
       * findTimePeriod search the list for the time period 'time' is in. If it do
       * not fall into one of the time period in the timeList return the time period
       * nearest in the "future".
       *
       *
       * @param time The time to find the time period to.
       * @param inPeriod Is true on return if 'time' is in the returned TimePeriod and
       *   false otherwise.
       * @return A pointer to a TimePeriod if the timeList is not empty and 0 if empty.
       */
      const TimePeriode* findTimePeriod( const boost::posix_time::time_duration &time,
                                   bool &inPeriod )const;

      boost::posix_time::ptime nextRun( const boost::posix_time::ptime &reqTime,
                                        boost::posix_time::ptime &fromTime )const;

      /**
       * Return the current set update time for this group. If the 'now' time is
       * greater than the current update time, find the next update time and set
       * the current update time to this value.
       * @param now The check time to test against.
       * @return Return the update time for this group.
       */
      boost::posix_time::ptime currentUpdate( const boost::posix_time::ptime &now );
   };

   class Update {
      std::list<UpdateGroup> groupList;

   public:
      Update();


      void addUpdateGroup( const UpdateGroup &group );
      UpdateGroup* findUpdateGroup( const std::string &provider );
   };
}
}

#endif
