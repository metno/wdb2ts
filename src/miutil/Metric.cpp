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

#include <time.h>
#include <string.h>
#include <iostream>
#include <string>
#include <map>
#include <limits>
#include "gettimeofday.h"
#include "Metric.h"

using namespace std;

namespace miutil {
Metric::Metric(const std::string &name ):
	start_(std::numeric_limits<double>::max()),
	acum_(std::numeric_limits<double>::max()), counter_(std::numeric_limits<int>::max()), name_(name){}

Metric::~Metric(){
}

void Metric::resetAll(){
	start_=std::numeric_limits<double>::max();
	acum_=std::numeric_limits<double>::max();
	counter_=std::numeric_limits<int>::max();
}


bool Metric::hasTimer()const{
	return acum_!=std::numeric_limits<double>::max();
}

void Metric::cancel(){
	start_ = std::numeric_limits<double>::max();
}

void Metric::resetTimer(){
	acum_=std::numeric_limits<double>::max();
}

void Metric::startTimer(){

//	if( start_ == std::numeric_limits<double>::max())
//		start_= miutil::gettimeofday();

	if( start_ == std::numeric_limits<double>::max()) {
		struct timespec ts;
		if( clock_gettime(CLOCK_MONOTONIC, &ts)==0) {
			start_ = (double)ts.tv_sec+((double)ts.tv_nsec)/1000000000.0;
		} else {
			cerr << " ERROR:  METRIC START, clock_gettime failed.\n";
		}
	}
}
	

void Metric::stopTimer(){
	if( start_ != std::numeric_limits<double>::max() ){

		double stop;
		struct timespec ts;
		if( clock_gettime(CLOCK_MONOTONIC, &ts)==0) {
			stop = (double)ts.tv_sec+((double)ts.tv_nsec)/1000000000.0;
			if( acum_ == std::numeric_limits<double>::max())
				acum_=stop-start_;
			else
				acum_+=stop-start_;
		} else {
			cerr << " ERROR:  METRIC STOP, clock_gettime failed.\n";
		}
		start_=std::numeric_limits<double>::max();
	}
}

void Metric::addToTimer( double inc) {
	if( inc == std::numeric_limits<double>::max() || inc < 0)
		return;

	if(acum_==std::numeric_limits<double>::max())
		acum_=inc;
	else
		acum_+=inc;
}

double Metric::getTimerSum()const{
	if(acum_==std::numeric_limits<double>::max())
		return 0;
	else
		return acum_;
}


int Metric::getTimerSumInms()const{
	return static_cast<int>(acum_*1000);
}


bool Metric::hasCounter()const{
	return counter_ != std::numeric_limits<int>::max();
}

void Metric::resetCounter(){
	counter_=std::numeric_limits<int>::max();
}

long Metric::getCounter()const{
	if( counter_ == std::numeric_limits<int>::max() )
		return 0;
	else
		return counter_;
}
void Metric::count(long cnt){
	if( counter_==std::numeric_limits<int>::max())
		counter_=cnt;
	else
		counter_+=cnt;
}
		
std::string Metric::name()const{
	return name_;
}

std::string Metric::getMetrics(const std::string &ns) {
	std::ostringstream o;
	std::string newLine;

	if(hasTimer()) {
		o << ns << name() <<":" << getTimerSumInms() << "|ms";
		newLine="\n";
	}

	if( hasCounter())
		o << newLine << ns << name() <<":" << getCounter() << "|c";

	return o.str();
}

CollectMetric::CollectMetric(const std::string &ns_):cnt(0),ns(ns_){}

void CollectMetric::addTimerMetric(const miutil::Metric &m){
	if( m.hasTimer()) {
		if( cnt > 0 )
			o << "\n";

		o << ns << m.name() <<":" << m.getTimerSumInms() << "|ms";
		cnt++;
	}

}

void CollectMetric::addCountMetric(const miutil::Metric &m){
	if( m.hasCounter()) {
		if( cnt > 0 )
			o << "\n";

		o << ns << m.name() <<":" << m.getCounter() << "|c";
		cnt++;
	}
}

std::string CollectMetric::getStatdMetric()const {
	return o.str();
}

std::ostream& operator<<(std::ostream &output,
                         const miutil::Metric &t){
	std::string newLine;

	if( t.hasTimer()) {
		output << t.name() <<":" << t.getTimerSumInms() << "|ms";
		newLine="\n";
	}

	if( t.hasCounter())
		output << newLine << t.name() <<":" << t.getCounter() << "|c";

}


}
