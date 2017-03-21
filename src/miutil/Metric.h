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

#ifndef __METRIC_H__
#define __METRIC_H__

#include <string>
#include <sstream>
#include <iostream>

namespace miutil{
	
	class Metric{
		double start_;
		double acum_;
		long counter_;
		std::string name_;

		public:
		Metric(const std::string &name);
		~Metric();
			
		void resetAll();

		void resetTimer();
		void startTimer();
		void stopTimer();

		void cancel();

		bool hasTimer()const;
		void addToTimer( double inc);
		double getTimerSum()const;
		int getTimerSumInms()const;

		bool hasCounter()const;
		void resetCounter();
		long getCounter()const;
		void count(long cnt=1);

		bool hasMetric()const { return hasCounter() || hasTimer(); }

		std::string getMetrics(const std::string &ns="");

		std::string name()const;
		friend std::ostream& operator<<(std::ostream& output,
                                            const miutil::Metric& t);


	};

	class MetricTimer {
	public:
		Metric &metric;
		MetricTimer( Metric &metric_ ):metric(metric_){
			metric.startTimer();
		}
		~MetricTimer(){
			metric.stopTimer();
		}

		void stop(){ metric.stopTimer();}
	};

	std::ostream& operator<<(std::ostream &output,
                            const miutil::Metric &t);

	class CollectMetric {
		int cnt;
		std::ostringstream o;
		std::string ns;
	public:
		explicit CollectMetric(const std::string &ns="");
		void addTimerMetric(const miutil::Metric &m);
		void addCountMetric(const miutil::Metric &m);
		bool hasMetrics()const { return cnt>0;}
		std::string getStatdMetric()const;
	};

					
};



#endif /*METRIC_H*/
