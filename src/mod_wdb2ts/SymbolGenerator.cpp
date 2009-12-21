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

#include <sstream>
#include <vector>
#include <map>
#include <stlContainerUtil.h>
#include <puMet/symbolMaker.h>
#include <puMet/paramet.h>
#include <SymbolGenerator.h>


using namespace std;

namespace wdb2ts {

symbolMaker SymbolGenerator::sm;

SymbolGenerator::
SymbolGenerator()
{
}
	
bool 
SymbolGenerator::
readConf( const std::string &confile )
{
	try {
		symbolMaker::readSymbols( confile );
	}
	catch( ... ) {
		return false;
	}

	return true;
}
	
	
bool
SymbolGenerator::
computeSymbols( std::map<miutil::miTime, std::map <std::string,float> >& data,
	             vector<miSymbol> &symbols,
	             float latitude, int min, int max, std::string &error)
{
   
   symbolMaker sm;
    
   std::map<miutil::miTime, std::map <std::string,float> >::iterator titr=data.begin();
   std::map<std::string,float>::iterator                             pitr;
    
   symbols.clear();
 
   symbols.reserve(data.size());
   
   // reorganise data
   map<string,map<miutil::miTime,float> > allParameters;
   vector<paramet> parameters;
   vector<miutil::miTime>  times;

   for (titr=data.begin() ;titr!=data.end();++titr) {
      times.push_back(titr->first);
     
      for( pitr=titr->second.begin();pitr!=titr->second.end();++pitr) 
         allParameters[pitr->first][titr->first] = pitr->second;
   }    
    
   map<miutil::miString,int>::iterator sitr = IDlist.begin();

   for( ;sitr != IDlist.end(); ++sitr) {
      if( allParameters.count( sitr->first ) ) {
         paramet p;
         p.AddPara( sitr->second, allParameters[sitr->first], latitude );
         parameters.push_back( p );
      }
   }
    
   //vector<miSymbol> symbols= sm.compute(parameters,times,2,3);
   symbols= sm.compute( parameters, times, min, max);

   return true;
}

SymbolHolder*
SymbolGenerator::
computeSymbols( LocationData& data,
		        const std::string &provider,
	            int min, int max, int precipHours, std::string &error )
{
	//stringstream ost;
	symbolMaker sm;
	boost::posix_time::ptime startAt;
	boost::posix_time::ptime pTime;
	boost::gregorian::date  date;
	boost::posix_time::time_duration time;
	boost::posix_time::ptime precipFromtime;
	miutil::miTime  mitime;
	vector<miSymbol> symbols;
	vector<miSymbol> tmpSymbols;
	map< int, map<miutil::miTime,float> > allParameters;
	vector<paramet>               parameters;
	vector<miutil::miTime>                times;
	float                         val;

	min--;
	
	if( min < 0 )
		min = 0;
	
	data.init( startAt, provider );
	
	while( data.hasNext() ) {
		//ost.str("");
		LocationElem &elem = *data.next();
		
		val = elem.TA();
		
		//Must allways have a temperature element
		if( val == FLT_MAX ) 
			continue;
	
		pTime = elem.time();
		date = pTime.date();
		time = pTime.time_of_day();
		mitime = miutil::miTime( date.year(), date.month(), date.day(),
            			         time.hours(), time.minutes(), time.seconds() );
		
		//ost << mitime << "(31): " << val;
		times.push_back( mitime );
		
		//Temperature
		allParameters[31][mitime] = val;
		
		//MSLP - mean sea level pressure
		val = elem.PR();
		
		if( val != FLT_MAX ) {
			//ost << " (58): " << val;
			allParameters[58][mitime] = val;
		}
		
		//Total cloud cower.
		val = elem.NN();
		
		if( val != FLT_MAX ) {
			//ost << " (25): " << val;	
			allParameters[25][mitime] = val;
		}
		
		//Thunder index
		val = elem.thunderProbability();
				
		if( val != FLT_MAX ) {
			//Must make an garanti that this can be read as an int
			//in symbolmaker.
			//This is an on/off switch, valid values 0 and 1;
			allParameters[661][mitime] = int( val+0.5 );
		}
		
		//Fog index
		val = elem.fogProbability();
						
		if( val != FLT_MAX ) {
			//Must make an garanti that this can be read as an int
			//in symbolmaker.
			//This is an on/off switch, valid values 0 and 1;
			allParameters[665][mitime] = int( val+0.5 );
		}
		
		//Precip agregated over precipHours.
		if( ( mitime.hour() % precipHours ) == 0 ) {
			val = elem.PRECIP( precipHours, precipFromtime );
			
			//cerr << "computeSym: hour: " << precipHours << " from: " << precipFromtime << " to: " << mitime << " (min: " << min << " max: " << max << ") val: " << val << endl;

			if( val != FLT_MAX ) {
				//ost << " (17): " << val;
				allParameters[17][mitime] = val;
			}
		}
		//ost << endl;
		
		//cerr << ost.str();
	}
	
	
	
	for( map< int, map<miutil::miTime,float> >::const_iterator it = allParameters.begin();
	     it != allParameters.end(); ++it ) {
		if( allParameters.count( it->first ) ) {
			paramet p;
			p.AddPara( it->first, allParameters[it->first], data.latitude() );
			parameters.push_back( p );
		}
	}
	   
   tmpSymbols = sm.compute( parameters, times, min, max);
   
   //Do some cleanup
   
   vector<miSymbol>::iterator tmpIt = tmpSymbols.begin();
   
   //Remove all symbols that has a hour  where hour % preciphours != 0. This symbols 
   //is garbage. 
  
   while ( tmpIt != tmpSymbols.end() ) 
	   tmpIt = miutil::eraseElementIf( tmpSymbols, tmpIt,
   			                           (tmpIt->getTime().hour() % precipHours) != 0 );
      
   symbols.reserve(tmpSymbols.size());

   //Only keep symbols we actually have data in both
   //fromtime and totime.
   
   int nSymbols=0;
   boost::posix_time::ptime testTime;

   for ( vector<miSymbol>::size_type i=0; i<tmpSymbols.size(); ++i) {
	   if ( symbolMaker::getErrorSymbol()==tmpSymbols[i] )
		   continue;
         
	   mitime = tmpSymbols[i].getTime();
	   pTime = boost::posix_time::ptime( boost::gregorian::date( mitime.year(),
   		                                                         mitime.month(),
   			                                                     mitime.day() ),
   			                             boost::posix_time::time_duration( mitime.hour(),
   			                           		                               mitime.min(),
   			                           		                               mitime.sec() )
   	                                    );
	   testTime = pTime + boost::posix_time::hours( -1*min-1 );
   	         
	   if ( !data.hasDataForTime(testTime, testTime, provider ) )
		   continue;
      
	   testTime = pTime + boost::posix_time::hours( max );
   	
	   if ( data.hasDataForTime( testTime, testTime, provider ) ) {
		   //cerr << "computeSym: insert: time: " << pTime << "(" << testTime<< ")  min (" << min << ") : " << -1*min-1 << " max: "<< max << endl;
		   symbols.push_back(tmpSymbols[i]);
		   nSymbols++;
	   }
   }
   
   symbols.resize(nSymbols);
   
   try {   
	   return new SymbolHolder(min, max, symbols);
   }
   catch ( ... ) {
   }
         
   return 0;
}

ProviderSymbolHolderList 
SymbolGenerator::
computeSymbols( LocationData& data, 
                const SymbolConfProvider &symbolConf,
                std::string &error )
{
	ProviderSymbolHolderList symbols;
	SymbolHolder *sh;
	std::string myerror;
	
	for( SymbolConfProvider::const_iterator it=symbolConf.begin();
	     it != symbolConf.end(); ++it  ) {
		for ( SymbolConfList::const_iterator itConf=it->second.begin();
		      itConf != it->second.end();
		      ++ itConf  ) {
			sh = computeSymbols( data, it->first, 
					               itConf->min(), itConf->max(), itConf->precipHours(),
					               myerror );
			if( !sh ) {
				error += myerror;
			} else {
				//If we allready have symbols for this provider and the timespan is equal we must merge the symbols.

				ProviderSymbolHolderList::iterator itProvider = symbols.find( it->first );

				if( itProvider != symbols.end() ) {
					for( SymbolHolderList::iterator itSym = itProvider->second.begin();
					     itSym != itProvider->second.end();
					     ++itSym )
					{
						if( (*itSym)->timespanInHours() == sh->timespanInHours() ) {
							SymbolHolder *mergedSymbols = (*itSym)->merge( *sh );

							if( mergedSymbols ) {
								delete sh;
								sh = 0;
								itSym->reset( mergedSymbols );
								break;
							}
						}
					}
				}

				if( sh )
					symbols[it->first].push_back( boost::shared_ptr<SymbolHolder>( sh ) );
			}
		}
	}
	
	return symbols;
}



}


