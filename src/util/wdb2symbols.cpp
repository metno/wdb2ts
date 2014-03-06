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
#include <float.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <boost/assign.hpp>
#include "../webFW/Logger4cpp.h"
#include "../miutil/ptimeutil.h"
#include "../mod_wdb2ts/configparser/ConfigParser.h"
#include "../mod_wdb2ts/PointDataHelper.h"
#include "../mod_wdb2ts/SymbolGenerator.h"
#include "../tuple_container/PqTupleContainer.h"
#include "../miutil/gettimeofday.h"
#include "../tuple_container/CSV.h"
#include "../mod_wdb2ts/WeatherSymbolDataBuffer.h"
#include "../mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/MomentTags1.h"

//./wdb2cvs -p "arome_2500m" -p "arome_2500m_temperature_corrected" --port 5432 -h staging-wdb-arome -r 2014-02-07T12:00:00
using namespace std;
namespace po = boost::program_options;
namespace pt = boost::posix_time;
namespace tc = miutil::container;
namespace wc = wdb2ts::config;



class Options
{
	po::options_description desc;
	vector<string> provider;
	string sLatitude;
	string sLongitude;
	string reftime;
	string validFrom;
	string validTo;
	string filename;
	bool help;


	pt::ptime s2time( const std::string &t )const{
		if( t.empty() )
			return pt::ptime();

		return miutil::ptimeFromIsoString( t );
	}

	/**
	 * Convert a location element latitude/longitude
	 * to decimal degree.
	 *
	 * Allowed formats is decimal degree with the point
	 * used defined by the local or in format the format
	 * [+/-]grad_minute_second[W|E|N|S]
	 *
	 * If both a [-] and W or S is given the W and S is ignored.
	 *
	 */
	float s2pos( const std::string &pos_ ) const{
		float ret=FLT_MIN;
		string pos( pos_ );
		char ch;
		vector<string> elements;
		int sign=1;

		if( pos.empty() )
			return ret;

		ch = pos[ pos.length()-1 ];

		if( ! isdigit( ch ) ) {
			if( ch == 'S' || ch == 's' || ch=='W' || ch=='w' )
				sign = -1;
			pos = pos.substr(0, pos.length()-1);
		}

		elements.reserve( 3 );
		boost::split( elements, pos, boost::is_any_of( "_" ), boost::algorithm::token_compress_on );

		if( elements.empty() )
			return ret;

		ret = boost::lexical_cast<float>( elements[0] );

		if( elements.size() > 1 && !elements[1].empty())
			ret += boost::lexical_cast<float>( elements[1] )/60;

		if( elements.size() > 2 && !elements[2].empty() )
			ret += boost::lexical_cast<float>( elements[2] )/3600;

		if( ret > 0 ) {
			ret *= sign;
		}
		return ret;
	}

public:
	Options()
		:desc("wdb2symbols options"), help( false ) {

		desc.add_options()
			("help", "Show this help screen and exit.")
			("provider,p", po::value< vector<string> >( &provider), "Data provider. Can be specified multiple times.")
			("reftime,r", po::value<string>(&reftime), "Reference time.")
			("long", po::value<string>(&sLongitude)->default_value( "10.7231" ), "Longitude. Decimal degree or GG_MM_SS[N|S|E|W]")
			("lat", po::value<string>(&sLatitude)->default_value( "59.9406" ), "Latitude. Decimal degree or [-]GG_MM_SS[N|S|E|W]")
			("validfrom,f", po::value<string>(&validFrom), "Valid time from.")
			("validto,t", po::value<string>(&validTo), "Valid time to.")
			("file", po::value<string>(&filename)->default_value("wdb-data-{reftime}.cvs"), "Name of the CVS output file.")
			;
	}

	bool getHelp() const {
		return help;
	}

	void
	parse( int argn, char *argv[] )
	{
		po::positional_options_description p;
		p.add("provider", -1);

		po::variables_map vm;
		po::store(po::command_line_parser( argn, argv).
		          options(desc).positional(p).run(), vm);
		po::notify(vm);

		if( vm.count("help") )
			help = true;
		else
			help = false;
	}

	void use( int exitStatus )const {
		desc.print( cerr );
		exit( exitStatus );
	}

	const vector<string>& getProvider() const {
		return provider;
	}

	boost::posix_time::ptime getReftime() const {
		return s2time( reftime );
	}

	float getLatitude() const {
		float lat = s2pos( sLatitude );
		if( lat<-90 || lat>90)
			return FLT_MIN;
		return lat;
	}

	float getLongitude() const {
		float lon = s2pos( sLongitude );
		if( lon<-180 || lon>180)
			return FLT_MIN;
		return lon;
	}

	boost::posix_time::ptime getValidFrom() const {
		return s2time( validFrom );
	}

	const boost::posix_time::ptime getValidTo() const {
		return s2time( validTo );
	}

	std::string getFilename() const {
		if( filename == "wdb-data-{reftime}.cvs" ) {
			if( reftime.empty() )
				return "wdb-data.cvs";
			else
				return "wdb-data-"+reftime +".cvs";
		} else {
			return filename;
		}
	}

	void validate()
	{
		if( provider.empty() ) {
			cerr << "\n --- At least one provider must be specified.\n" << endl;
			use( 1 );
		}

		if( getReftime().is_special() &&
			( getValidFrom().is_special() || getValidTo().is_special() ) )
		{
			cerr << "\n --- Must give reference time or validfrom/validto.\n" << endl;
			use( 1 );
		}
	}

	friend std::ostream& operator<<( std::ostream &o, const Options &opt );
};


std::ostream&
operator<<( std::ostream &o, const Options &opt )
{
	try {
		o << "Symbols: "  << endl;
		o << "  Location (long lat): " << opt.getLongitude() << " " << opt.getLatitude() << endl;
		o << "  Referencetime: " << opt.getReftime() << " Valid: " << opt.getValidFrom() << " - " << opt.getValidTo() << endl;
		o << "  Provider:";

		BOOST_FOREACH( const string &provider, opt.getProvider() ){
			o << " '" << provider <<"'";
		}
		o << endl;
	}
	catch( const boost::bad_lexical_cast &ex ) {
		o << "Invalid value (bad_cast): " << ex.what() << endl;
	}
	catch( const std::exception &ex ) {
		o << "Invalid value: " << ex.what() << endl;
	}

	return o;
}




struct Data {
	float value;
	string dataprovidername;
	string placename;
	pt::ptime referencetime;
	pt::ptime validtimefrom;
	pt::ptime validtimeto;
	string valueparametername;
	string valueparameterunit;
	string levelparametername;
	string levelunitname;
	int levelfrom;
	int levelto;
	int dataversion;
	string point;

	Data():
		value( FLT_MIN), levelfrom( INT_MIN ), levelto( INT_MIN ), dataversion(INT_MIN)
	{}

	Data( pqxx::result::const_iterator it ):
		value( it->at(0).as<float>( FLT_MIN ) ),
		dataprovidername( it->at(1).c_str() ),
		placename( it->at(2).c_str() ),
		referencetime( miutil::ptimeFromIsoString( it->at(3).c_str() ) ),
		validtimefrom( miutil::ptimeFromIsoString( it->at(4).c_str() ) ),
		validtimeto( miutil::ptimeFromIsoString( it->at(5).c_str() ) ),
		valueparametername( it->at(6).c_str() ),
		valueparameterunit( it->at(7).c_str() ),
		levelparametername( it->at(8).c_str() ),
	    levelunitname( it->at(9).c_str() ),
		levelfrom( it->at(10).as<int>( INT_MIN ) ),
		levelto( it->at(11).as<int>( INT_MIN ) ),
		dataversion( it->at(12).as<int>( INT_MIN ) ),
		point( it->at(13).c_str() )
	{
	}

	Data( tc::ITuple &it ):
			value( it.at(0).as<float>( FLT_MIN ) ),
			dataprovidername( it.at(1).c_str() ),
			placename( it.at(2).c_str() ),
			referencetime( it.at(3).as<pt::ptime>() ),
			validtimefrom( it.at(4).as<pt::ptime>() ),
			validtimeto( miutil::ptimeFromIsoString( it.at(5).c_str() ) ),
			valueparametername( it.at(6).c_str() ),
			valueparameterunit( it.at(7).c_str() ),
			levelparametername( it.at(8).c_str() ),
		    levelunitname( it.at(9).c_str() ),
			levelfrom( it.at(10).as<int>( FLT_MIN ) ),
			levelto( it.at(11).as<int>( FLT_MIN ) ),
			dataversion( it.at(12).as<int>( FLT_MIN ) ),
			point( it.at(13).c_str() )
		{
		}


	friend std::ostream &operator<<( std::ostream &o, const Data &d );
};
std::ostream&
operator<<( std::ostream &o, const Data &d )
{
	o << d.value << ", "
      << d.dataprovidername << ", "
      << d.placename << ", "
      << d.referencetime << ", "
      << d.validtimefrom << ", "
      << d.validtimeto << ", "
      << d.valueparametername << ", "
      << d.valueparameterunit << ", "
      << d.levelparametername << ", "
      << d.levelunitname << ", "
      << d.levelfrom << ", "
      << d.levelto << ", "
      << d.dataversion << ", "
      << d.point;
	return o;
}


miutil::container::SimpleTupleContainer*
loadData( const std::string &file )
{
	ifstream fin;

	fin.open( file.c_str() );

	if( ! fin.is_open() ) {
		cerr << "Can't open the file <" << file << "> for reading!" << endl;
		return 0;
	}

	return readCSV( fin );
}

/**
 * Generate configuration information from the dataset.
 */
void
initFromData( wdb2ts::ProviderList &providerList, wdb2ts::ProviderRefTimeList &reftimes, wdb2ts::SymbolConfProvider &symbolConf, const tc::ITupleContainer &con )
{
	typedef map<string, pt::ptime> RefTimes;
	struct Hours{
		map<string, pt::ptime> prevTime;
		map<string, map<int, int> > hours;
		typedef map<string, map<int, int> >::value_type ProviderHoursCount;
		typedef map<int, int>::value_type HoursCount;
		int diffHours( const pt::ptime &t, const string &provider ) {
			pt::ptime pt = prevTime[provider];
			if( pt.is_special() ) {
				prevTime[provider] = t;
				return 0;
			}

			if( pt == t )
				return 0;

			prevTime[provider] = t;
			return ( t - pt ).hours();
		}
		void operator()( const pt::ptime &t, const string &provider ) {
			hours[provider][diffHours(t, provider)]++;
		}
	} hours;
	RefTimes providerReftimes;

	for( tc::IIteratorPtr it = con.iteratorPtr(); it->hasNext(); ) {
		tc::ITuple &row=it->next();
		wdb2ts::ProviderItem provider=wdb2ts::ProviderItem::decodeFromWdb( row.at("dataprovidername").c_str(), row.at("placename").c_str() );
		providerList.addProvider( provider );

		if( row.at("validtimefrom").as<pt::ptime>() == row.at("validtimeto").as<pt::ptime>() )
			hours( row.at("validtimefrom").as<pt::ptime>(), provider.providerWithPlacename() );

		providerReftimes[ provider.providerWithPlacename() ] = row.at("referencetime").as<pt::ptime>();
	}

	BOOST_FOREACH( Hours::ProviderHoursCount &provider, hours.hours ){
		BOOST_FOREACH( Hours::HoursCount &val, provider.second ) {
			if( val.first == 1 )
				symbolConf.add( provider.first, boost::assign::list_of(wdb2ts::SymbolConf(0,0,1))(wdb2ts::SymbolConf(2,0,1))
						(wdb2ts::SymbolConf(3,0,1))(wdb2ts::SymbolConf(6,0,1)) );
			else if( val.first == 3 )
				symbolConf.add(provider.first, boost::assign::list_of(wdb2ts::SymbolConf(3,0,3))(wdb2ts::SymbolConf(6,0,6)) );
			else if( val.first == 6 )
				symbolConf.add(provider.first, boost::assign::list_of(wdb2ts::SymbolConf(6,0,6)) );
		}
	}

	BOOST_FOREACH( RefTimes::value_type &v, providerReftimes ){
		reftimes[v.first]= wdb2ts::ProviderTimes( v.second );
	}
}

void
setTemperatureCorrected( wdb2ts::LocationData  &theData,
		                 const std::string &temperatureCorrectedProvider )
{
	string provider( temperatureCorrectedProvider );
	theData.init( pt::ptime(), temperatureCorrectedProvider );

	bool tryHard = provider.empty();

	while( theData.hasNext() ) {
		float val;
		wdb2ts::LocationElem &elem = *theData.next();
		val = elem.T2M_NO_ADIABATIC_HIGHT_CORRECTION( tryHard );
		if( val != FLT_MAX ) {
			tryHard = false;
			if( provider.empty() )
				provider = elem.lastUsedProvider();

			elem.temperatureCorrected( val, provider, true );
		}
	}
}

void
setLogLevel()
{
	{	WEBFW_USE_LOGGER( "symbols" );
		WEBFW_SET_LOGLEVEL( log4cpp::Priority::ERROR ); }
	{	WEBFW_USE_LOGGER( "main" );
		WEBFW_SET_LOGLEVEL( log4cpp::Priority::ERROR ); }
	{	WEBFW_USE_LOGGER( "encode" );
		WEBFW_SET_LOGLEVEL( log4cpp::Priority::ERROR ); }
}


int
main( int argn, char *argv[] )
{
	setLogLevel();
	string file("wdb-data-2014-02-12T00:00:00.cvs");
	wdb2ts::SymbolGenerator symbolGenerator;
	Options opt;

	symbolGenerator.readConf( string(SRCDIR) + "/etc/qbSymbols.def.in");

	opt.parse( argn, argv );

	if( opt.getHelp() )
		opt.use( 0 );

	opt.validate();

	cerr << opt << endl;

	wc::ConfigParser parser;
	wc::Config *config=parser.parseFile( "metno-wdb2ts-conf.xml");

	if( ! config ) {
		cerr << "Failed to parser parameter definitions." << endl;
		return 1;
	}

	cerr <<  config->paramdef << endl << endl << endl;
	list<string> providers;

	copy( opt.getProvider().begin(), opt.getProvider().end(), back_inserter( providers) );

	wdb2ts::ParamDefList parDefs = config->paramdef.getParamDefs( providers );

	tc::SimpleTupleContainer *wdbData = loadData( file );

	if( ! wdbData ) {
		cerr << "Failed to load wdb-data from file <" << file << ">." << endl;
		return 1;
	}


//	Data d;
//	int n=0;
//	for( boost::shared_ptr<tc::IIterator> ri( wdbData->iterator() ); ri->hasNext() /*&& n<10*/;  ++n ) {
//		d = Data( ri->next() );
//		cerr <<  d << endl;
//	}

	wdb2ts::ProviderList providerList;
	wdb2ts::ProviderRefTimeList refTimes;
	wdb2ts::SymbolConfProvider symbolConf;
	wdb2ts::LocationPointData locationpointData;
	wdb2ts::TopoProviderMap modelTopoProviders;
	list<string> topoProviders;


	initFromData( providerList, refTimes, symbolConf, *wdbData );









	cerr << "Configuration: " << endl;
	cerr << "  providers: "<< endl;
	BOOST_FOREACH( wdb2ts::ProviderItem &provider, providerList )
		cerr << "    "   << provider.providerWithPlacename() << endl;
	cerr << "  reftimes: " << endl;
	BOOST_FOREACH( wdb2ts::ProviderRefTimeList::value_type &v, refTimes )
		cerr << "    " << v.first << ":  " << v.second.refTime  << endl;
	cerr << "  SymbolConf: " << endl;
	for( wdb2ts::SymbolConfProvider::const_iterator it = symbolConf.begin();
		it != symbolConf.end(); ++it ){
		cerr << "    " << it->first << endl;
		for( wdb2ts::SymbolConfList::const_iterator cit = it->second.begin();
				cit != it->second.end(); ++cit )
			cerr << "      " << *cit << endl;
	}

 	wdb2ts::decodePData( parDefs, providerList, refTimes, *wdbData, false, locationpointData );

	if( locationpointData.empty() ) {
		cerr << "--- NO data decoded from file <" << file << ">.\n";
		return 1;
	}

	string error;
	wdb2ts::LocationData  theData( locationpointData.begin()->second,
								   opt.getLongitude(), opt.getLatitude(), 10,
								   providerList, modelTopoProviders, topoProviders );

	setTemperatureCorrected( theData, "" );

	ostringstream xml;
	wdb2ts::WeatherSymbolDataBuffer dataBuffer( 6 );
	wdb2ts::WeatherSymbolDataBuffer::slice_iterator slice;
	int timestep;
	float precip, precipMin, precipMax, precipProb;
	boost::posix_time::ptime precipFrom;
	theData.init( pt::ptime() );

	while( theData.hasNext() ) {
		float val;
		wdb2ts::SymbolDataElement symData;
		wdb2ts::LocationElem &elem = *theData.next();

		xml.str("");
		wdb2ts::MomentTags1 moment( elem, symData );
		moment.output( xml, "      " );
		if( elem.PRECIP_MIN_MAX_MEAN( 3, precipFrom, precipMin, precipMax, precip, precipProb, false ) ) {
			symData.precipitation = precip;
			symData.maxPrecipitation = precipMax;
			symData.minPrecipitation = precipMin;
			symData.from = precipFrom;
		} else {
			cerr << "  WARNING: NO PRECIP DATA: " << elem.time() << endl;
		}

		dataBuffer.add( elem.time(), symData );
		symData = wdb2ts::computeWeatherSymbol( dataBuffer, 3 );
//		//cerr << elem.time() << " " << symData << endl;
//		cerr << " ------ " << elem.time() << " ---------------" << endl;
//		cerr << xml.str();
//		slice = dataBuffer.slice( 3, timestep);
//		cerr << " ------ " << elem.time() << " (" << timestep << ") ---------------" << endl;
//		dataBuffer.print( cerr, slice ) << endl;
//		//cerr << dataBuffer << endl;
	}


//	wdb2ts::symbol::SymbolDataContainer baseData;
//	//wdb2ts::symbol::SymbolDataContainer symbolData;
//	wdb2ts::symbol::SymbolContainer symbolData;
//	BOOST_FOREACH(wdb2ts::SymbolConfProvider::value_type &v, symbolConf ) {
//		cerr << "================== BEGIN (" << v.first << ") =================== \n";
//		BOOST_FOREACH( wdb2ts::SymbolConf &sv, v.second ) {
//			wdb2ts::symbol::loadBaseData( baseData, theData, v.first, sv.precipHours() );
//
//			BOOST_FOREACH(wdb2ts::symbol::SymbolDataContainer::value_type &v, baseData ) {
//				cerr << v.second.from << " - "  << v.first << ": " << v.second << endl;
//			}
//
//			symbolData.clear();
//			cerr << "\n ----- compute 1 (begin) --------------------\n";
//			wdb2ts::symbol::computeSymbolData( baseData, symbolData, 1 );
//			cerr << "\n ----- compute 1 (end) --------------------\n";
//
//			cerr << "\n ----- compute 2 (begin) --------------------\n";
//			wdb2ts::symbol::computeSymbolData( baseData, symbolData, 2 );
//			cerr << "\n ----- compute 1 (end) --------------------\n";
//
//			cerr << "\n ----- compute 3 (begin) --------------------\n";
//			wdb2ts::symbol::computeSymbolData( baseData, symbolData, 3 );
//			cerr << "\n ----- compute 3 (end) --------------------\n";
//
//
//			cerr << "\n ----- compute 6 (begin) --------------------\n";
//			wdb2ts::symbol::computeSymbolData( baseData, symbolData, 6 );
//			cerr << "\n ----- compute 6 (end) --------------------\n";
//
//			cerr << "\n ---- SYMBOL DATA  (" << sv.precipHours() << ") ---- \n";
//			BOOST_FOREACH(wdb2ts::symbol::SymbolContainer::value_type &v, symbolData ) {
//				BOOST_FOREACH( wdb2ts::symbol::SymbolContainerElement::value_type & sym, v.second )  {
//					cerr << sym.first << " - " << v.first << "  #" << (v.first-sym.first).hours() << " (" << sym.second.count <<") " << sym.second.symbolData << endl;
//				}
//				cerr << "   ---------------------------------------------------  \n";
//			}
//
//		}
//
//		cerr << "================== END (" << v.first << ") =================== \n";
//	}


//	wdb2ts::ProviderSymbolHolderList symbols;
//
//	symbols = wdb2ts::SymbolGenerator::computeSymbols ( theData, symbolConf, true, error );
//
//	BOOST_FOREACH( wdb2ts::ProviderSymbolHolderList::value_type &val, symbols ) {
//		BOOST_FOREACH( wdb2ts::SymbolHolderPtr symbol, val.second ) {
//			cerr << *symbol << endl;
//		}
//	}
}
