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
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stlContainerUtil.h>
#include <puMet/symbolMaker.h>
#include <puMet/paramet.h>
#include <SymbolGenerator.h>
#include "LocationElem.h"
#include <Logger4cpp.h>


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
	

void
SymbolGenerator::
correctSymbol( SymbolHolder::Symbol &symbol,
               const PartialData &pd,
               const Precipitation &precip )
{
   //WEBFW_USE_LOGGER( "encode" );
   WEBFW_USE_LOGGER("+wetbulb");
// SymbolHolder::Symbol oldSymbol;

   if( pd.totalCloud != FLT_MAX && symbol.symbol == symbolMaker::Cloud ) {
      if( pd.mediumCloud < 13  && pd.lowCloud < 13 && pd.fog < 13 ) {
         miSymbol partlyCloud( symbolMaker::createSymbol( symbolMaker::PartlyCloud ));
         partlyCloud.setTime( symbol.symbol.getTime() );
         partlyCloud.setLightStat( symbol.symbol.getLightStat() );

//       oldSymbol = symbol;
         symbol.symbol = partlyCloud;
/*
         if( oldSymbol.idnumber() != symbol.idnumber() ) {
            WEBFW_LOG_DEBUG( "SymbolGenerator::correctSymbol: Correct for misleading cloudtype: old: "
                             << oldSymbol.idnumber() << " (" << oldSymbol.idname()
                           << ") new: " << symbol.idnumber() << " (" << symbol.idname() << ")");
         }
*/
      }
   }

   if( precip.max != FLT_MAX ) {
      float noRainLimit, rainLimit;
      symbolMaker::rainLimits( symbol.timespanInHours(), noRainLimit, rainLimit );

      if( precip.max > noRainLimit &&
          (symbol.symbol == symbolMaker::Sun ||
           symbol.symbol == symbolMaker::LightCloud ) ) {
         miSymbol partlyCloud( symbolMaker::createSymbol( symbolMaker::PartlyCloud ));
         partlyCloud.setTime( symbol.symbol.getTime() );
         partlyCloud.setLightStat( symbol.symbol.getLightStat() );

         symbol.symbol = partlyCloud;
      }
   }

   if( ! symbol.withOutStateOfAgregate  ||
         (pd.temperatureCorrected==FLT_MAX && pd.wetBulbTemperature==FLT_MAX) ) {
      return;
   }


   if( pd.wetBulbTemperature != FLT_MAX ) {

      symbol.wetBulbTemp = pd.wetBulbTemperature;
      SymbolHolder::Symbol oldSymbol;
      oldSymbol = symbol;

      symbolMaker::stateMaker( symbol.symbol, pd.wetBulbTemperature, AggregateLimits( 0.5, 1.0 ) );

      if( pd.temperatureCorrected != FLT_MAX ) {
         symbol.heightCorrectedTemp = pd.temperatureCorrected;
         symbolMaker::stateMaker( oldSymbol.symbol, pd.temperatureCorrected );
         symbol.oldHeightCorrectedSymbol = oldSymbol.symbol;

         WEBFW_LOG_DEBUG(pd.time << " T.HC: " << fixed << setprecision(1) << setw(5) << pd.temperatureCorrected
                         << " T.WB: " << setw( 5 ) << pd.wetBulbTemperature
                         << " (" << setw(5) << pd.temperatureCorrected - pd.wetBulbTemperature << ")" <<
                         ( pd.url().empty()?"":(" - "+pd.url()) ));

      }

      if( pd.temperatureCorrected != FLT_MAX &&
          symbol.symbol != oldSymbol.symbol ) {
         WEBFW_LOG_NOTICE( fixed << setprecision(0) << pd.time << " - " << setw(19) << oldSymbol.idname() <<
                         " (" << setw(2) << oldSymbol.idnumber() << ") -> " <<
                         setw(19) << symbol.idname() << " (" << setw(2) << symbol.idnumber() << ") "  << setprecision(1) <<
                         " T.HC: " << setw(5) << pd.temperatureCorrected << " T.WB: " << setw(5) << pd.wetBulbTemperature <<
                         ( pd.url().empty()?"":(" - " + pd.url())) );
      }
   } else {
      symbolMaker::stateMaker( symbol.symbol, pd.temperatureCorrected );
   }


/*
   if( oldSymbol.idnumber() != symbol.idnumber() ) {
      WEBFW_LOG_DEBUG( "SymbolGenerator::correctSymbol: Correct agregate: temp: " << temperature
                     << " old: " << oldSymbol.idnumber() << " (" << oldSymbol.idname()
                      << ") new: " << symbol.idnumber() << " (" << symbol.idname() << ")");
   }
*/


}


void
SymbolGenerator::
correctSymbol( SymbolHolder::Symbol &symbol,
               const LocationElem &data,
               const Precipitation &precip )
{
   correctSymbol( symbol, PartialData( data ), precip );
}

SymbolHolder*
SymbolGenerator::
computeSymbols( LocationData& data,
		        const std::string &provider,
	            int min, int max, int precipHours, bool withoutStateOfAgregate, std::string &error )
{
	//stringstream ost;
	symbolMaker sm;
	ostringstream smLog;
	boost::posix_time::ptime startAt;
	boost::posix_time::ptime pTime;
	boost::gregorian::date  date;
	boost::posix_time::time_duration time;
	boost::posix_time::ptime precipFromtime;
	miutil::miTime  mitime;
	vector<miSymbol> symbols;
	vector<miSymbol> tmpSymbols;
	map< int, map<miutil::miTime,float> > allParameters;
	vector<paramet>        parameters;
	vector<miutil::miTime> times;
	float                  val;

	WEBFW_USE_LOGGER( "symbols" );
	min--;
	
	if( min < 0 )
		min = 0;
	
	WEBFW_LOG_DEBUG("computeSymbols: " << provider << " min: " << min << " max: " << max
	             << " precipHours: " << precipHours
	             << " withoutStateOfAgregate: " << (withoutStateOfAgregate?"t":"f"));
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
		
		//Total cloud cover.
		val = elem.NN();
		
		if( val != FLT_MAX ) {
			//ost << " (25): " << val;	
			allParameters[25][mitime] = val;
		}
		
		//Thunder index
		val = elem.thunderProbability();

		//DEBUG (BEGIN)
		//if( min == 0 )
		//   val = 1;
		//DEBUG (END)
				
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
			
			//DEBUG (BEGIN)
			//if( min == 0 )
			//   val = 0.4;
			//DEBUG (END)

			WEBFW_LOG_DEBUG( "computeSymbols: hour: " << precipHours << " from: " << precipFromtime << " to: " << mitime << " lprovider: " << elem.lastUsedProvider() << " val: " << val );

			if( val != FLT_MAX ) {
				//ost << " (17): " << val;
				allParameters[17][mitime] = val;
			}
		}
		//ost << endl;
		
		//cerr << ost.str();
	}
	
#if 0
	//This code is here only to create a test data set.
	ostringstream foutname;
	foutname << "/home/borgem/projects/data_" << provider << "_min_" << min << "_max_" << max << ".dat";
	ofstream  fout( foutname.str().c_str() );
	if( fout.good() ) {
	   fout << "min: " << min << " max: " << max << endl;
	   for( map< int, map<miutil::miTime,float> >::const_iterator it = allParameters.begin();
	         it != allParameters.end(); ++it ) {
	      for( map<miutil::miTime,float>::const_iterator tit=it->second.begin();
	            tit != it->second.end(); ++tit ) {
	         fout << it->first << "," << tit->first << "," << tit->second << endl;
	      }
	   }
	   fout.close();
	}
#endif
	
	for( map< int, map<miutil::miTime,float> >::const_iterator it = allParameters.begin();
	     it != allParameters.end(); ++it ) {
		if( allParameters.count( it->first ) ) {
			paramet p;
			p.AddPara( it->first, allParameters[it->first], data.latitude() );
			parameters.push_back( p );
		}
	}

	int logLevel = WEBFW_GET_LOGLEVEL();

	if( logLevel == log4cpp::Priority::DEBUG)
	    sm.setLogger( &smLog );

	if( withoutStateOfAgregate )
		tmpSymbols = sm.computeWithoutStateOfAggregate( parameters, times, min, max, true );
	else
		tmpSymbols = sm.compute( parameters, times, min, max );

    if( logLevel == log4cpp::Priority::DEBUG) {
        WEBFW_LOG_DEBUG("symbolMaker: " << smLog.str() );
    }
   



//	for(vector<miSymbol>::iterator myIt = tmpSymbols.begin();
//	        myIt != tmpSymbols.end(); ++myIt ) {
//	    WEBFW_LOG_DEBUG( "Symbol: " << myIt->getTime() << " : " << myIt->customNumber()
//	                    << " (" << symbolidToName( myIt->customNumber() ) << ")" );
//	}

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
	   return new SymbolHolder(min, max, symbols, withoutStateOfAgregate );
   }
   catch ( ... ) {
   }
         
   return 0;
}

SymbolHolder*
SymbolGenerator::
computeSymbolsWithPuMet( LocationData& data,
                         const SymbolConf &symbolConf,
                         const std::string &provider,
                         bool withoutStateOfAgregate,
                         std::string &error )
{
   std::string myerror;

   SymbolHolder *sh = computeSymbols( data, provider,
                                      symbolConf.min(), symbolConf.max(), symbolConf.precipHours(),
                                      withoutStateOfAgregate,
                                      myerror );
   if( !sh ) {
      error += myerror;
      return 0;
   }

   return sh;
}

SymbolHolder*
SymbolGenerator::
getSymbolsFromData( LocationData& data,
                    const SymbolConf &symbolConf,
                    const std::string &provider,
                    std::string &error )
{
   boost::posix_time::ptime startAt;
   boost::posix_time::ptime fromTime;
   int symNumber;
   float prob;
   boost::posix_time::time_duration h;
   SymbolHolder *sh = new SymbolHolder( symbolConf.precipHours() );

   if( !sh ) {
      error += "NO MEM";
      return 0;
   }

   data.init( startAt, provider );

   while( data.hasNext() ) {
      LocationElem *elem = data.next();

      if( ! elem )
         continue;

      symNumber = elem->symbol( fromTime );

      if( symNumber == INT_MAX ) {
         //WEBFW_LOG_DEBUG("doSymbol: NO SYMBOL for fromtime: " << fromTime );
         continue;
      }

      h = elem->time() - fromTime;

      if( h.is_negative() ) {
         h.invert_sign();
      }

      if( h.hours() != sh->timespanInHours() ) {
         continue;
      }

      prob = elem->symbol_PROBABILITY( fromTime );

      sh->addSymbol( elem->time(), symNumber, elem->latitude(), true, prob );
   }

   return sh;
}


ProviderSymbolHolderList 
SymbolGenerator::
computeSymbols( LocationData& data, 
                const SymbolConfProvider &symbolConf,
                bool withoutStateOfAgregate,
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
	      if( itConf->min() != INT_MIN && itConf->max() != INT_MAX ) {
	         sh = computeSymbolsWithPuMet( data, *itConf, it->first, withoutStateOfAgregate, error );
	      } else {
	         sh = getSymbolsFromData( data, *itConf, it->first, error );
	      }

	      if( sh ) {
	         //If we allready have symbols for this provider and the timespan is equal we must merge the symbols.

	         ProviderSymbolHolderList::iterator itProvider = symbols.find( it->first );

	         if( itProvider != symbols.end() ) {
	            for( SymbolHolderList::iterator itSym = itProvider->second.begin();
	                  itSym != itProvider->second.end();
	                  ++itSym ) {
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

	         if( sh ) {
	            sh->provider = it->first;
	            symbols[it->first].push_back( boost::shared_ptr<SymbolHolder>( sh ) );
	         }
	      }
	   }
	}
	
	symbols.consistentCheck();

	return symbols;
}



}


