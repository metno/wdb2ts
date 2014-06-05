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
#include "../miutil/ptimeutil.h"
#include "../mod_wdb2ts/configparser/ConfigParser.h"
#include "../tuple_container/PqTupleContainer.h"
#include "../miutil/gettimeofday.h"
#include "../tuple_container/CSV.h"

//./wdb2cvs -p "arome_2500m" -p "arome_2500m_temperature_corrected" --port 5432 -h staging-wdb-arome -r 2014-02-07T12:00:00
using namespace std;
namespace po = boost::program_options;
namespace pt = boost::posix_time;
namespace tc = miutil::container;
namespace wc = wdb2ts::config;

const char *RetValues=
		"value, dataprovidername, placename, referencetime, validtimefrom, validtimeto, "
		"valueparametername, valueparameterunit, levelparametername, "
		"levelunitname, levelfrom, levelto, dataversion, astext(placegeometry) as point";


class Options
{
	po::options_description desc;
	vector<string> provider;
	string sLatitude;
	string sLongitude;
	string reftime;
	string validFrom;
	string validTo;
	string wdbUser;
	int dbPort;
	string dbHost;
	string dbUser;
	string dbName;
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
		:desc("wdb2cvs options"), dbPort( 5432 ), help( false ) {

		desc.add_options()
			("help", "Show this help screen and exit.")
			("provider,p", po::value< vector<string> >( &provider), "Data provider. Can be specified multiple times.")
			("reftime,r", po::value<string>(&reftime), "Reference time.")
			("long", po::value<string>(&sLongitude)->default_value( "10.7231" ), "Longitude. Decimal degree or GG_MM_SS[N|S|E|W]")
			("lat", po::value<string>(&sLatitude)->default_value( "59.9406" ), "Latitude. Decimal degree or [-]GG_MM_SS[N|S|E|W]")
			("validfrom,f", po::value<string>(&validFrom), "Valid time from.")
			("validto,t", po::value<string>(&validTo), "Valid time to.")
			("wdbuser", po::value<string>(&wdbUser)->default_value( "wdb" ), "WDB user name.")
			("port", po::value<int>(&dbPort)->default_value(5432), "postgres port.")
			("host,h", po::value<string>(&dbHost), "Postgres host name.")
			("user,u", po::value<string>(&dbUser)->default_value("wdb"), "postgres user name.")
			("dbname,d", po::value<string>(&dbName)->default_value("wdb"), "Database name.")
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

	const string& getDbHost() const {
		return dbHost;
	}

	const string& getDbName() const {
		return dbName;
	}

	int getDbPort() const {
		return dbPort;
	}

	const string& getDbUser() const {
		return dbUser;
	}
	const string& getWdbUser() const {
		return wdbUser;
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
		o << "WDB: (" <<  opt.getWdbUser() << ")" << endl;
		o << "  Location (long lat): " << opt.getLongitude() << " " << opt.getLatitude() << endl;
		o << "  Referencetime: " << opt.getReftime() << " Valid: " << opt.getValidFrom() << " - " << opt.getValidTo() << endl;
		o << "  Provider:";

		BOOST_FOREACH( const string &provider, opt.getProvider() ){
			o << " '" << provider <<"'";
		}
		o << endl;

		o << "Login information:" << endl;
		o << "  Host: " << opt.getDbHost() << endl
		  << "  Port: " << opt.getDbPort() << endl
		  << "  User: " <<opt.getDbUser() << endl
		  << "    DB: " << opt.getDbName() << endl << endl;
	}
	catch( const boost::bad_lexical_cast &ex ) {
		o << "Invalid value (bad_cast): " << ex.what() << endl;
	}
	catch( const std::exception &ex ) {
		o << "Invalid value: " << ex.what() << endl;
	}

	return o;
}

std::string
createWdbGetQuery( const Options &opt )
{
	ostringstream q;

	q << "SELECT " << RetValues << " FROM ";
	vector<string>::const_iterator it = opt.getProvider().begin();

	q << "wci.read(ARRAY['" << *it << "'";
	for( ++it; it != opt.getProvider().end(); ++it )
		q << ",'" << *it << "'";
	q << "],";

	q << "'nearest POINT( " << opt.getLongitude() << " " << opt.getLatitude()<<" )',";

	if( opt.getReftime().is_special() )	q << "NULL,";
	else q << "'exact " << miutil::isotimeString( opt.getReftime(), false, true ) <<"',";

	if( opt.getValidFrom().is_special() || opt.getValidTo().is_special() )
		q << "NULL,";
	else if( opt.getValidFrom() == opt.getValidTo() )
		q << "'exact " << miutil::isotimeString( opt.getValidFrom(), false, true ) << "',";
	else
		q << "'inside " << miutil::isotimeString( opt.getValidFrom(), false, true ) << " TO "
		  << miutil::isotimeString( opt.getValidTo(), false, true ) << "',";

	q << "NULL, "; //parameter
	q << "NULL,"; //LevelSpec
	q << "array[-1],"; //Dataversion
	q << "NULL::wci.returnfloat )";
	q << " ORDER BY dataprovidername, referencetime, validtimefrom, validtimeto, valueparametername";

	return q.str();
}


void
getWdbData( const Options &opt )
{
	using namespace pqxx;

	ostringstream ost;

	ost << "host=" << opt.getDbHost() << " port=" << opt.getDbPort()
	   << " user=" << opt.getDbUser() << " dbname=" << opt.getDbName();

	connection con( ost.str() );

	if( ! con.is_open() ) {
		cerr << "Failed to connect to DB at: " << ost.str() << "." << endl;
		return;
	}

	cerr << "Connected to DB at: " << ost.str() << "." << endl;

	try {
		work w( con, "WDB" );
		ost.str("");

		ost << "SELECT wci.begin('" << opt.getWdbUser() << "');";
		w.exec( ost.str() );
		result r = w.exec( createWdbGetQuery( opt ) );

		for( result::const_iterator ri = r.begin(); ri != r.end(); ++ri ) {
			result::tuple::const_iterator ti=ri->begin();
			cerr << *ti;
			for( ++ti ; ti != ri->end(); ++ti  ) {
				cerr << ", " << *ti;
			}
			cerr  << endl;
		}

	}
	catch( const std::exception &ex ) {
		cerr << "DB ERROR: " << ex.what() << "." << endl;
	}


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

double
testPqxx( const Options &opt, int nLoops )
{
	using namespace pqxx;

	ostringstream ost;
	double tStart, tStop;
	ost << "host=" << opt.getDbHost() << " port=" << opt.getDbPort()
	   << " user=" << opt.getDbUser() << " dbname=" << opt.getDbName();

	connection con( ost.str() );

	if( ! con.is_open() ) {
		cerr << "Failed to connect to DB at: " << ost.str() << "." << endl;
		return DBL_MIN;
	}

	cerr << "Connected to DB at: " << ost.str() << "." << endl;

	try {
		work w( con, "WDB" );
		ost.str("");

		ost << "SELECT wci.begin('" << opt.getWdbUser() << "');";

		w.exec( ost.str() );

		string q = createWdbGetQuery( opt );

		Data d;
		result r = w.exec( q );
		tStart = miutil::gettimeofday();

		for( int i=0; i < nLoops; ++i ) {
			for( result::const_iterator ri = r.begin(); ri != r.end() ; ++ri ) {
				d = Data( ri );
				//cerr << d << endl;
			}
		}

		tStop = miutil::gettimeofday();
	}
	catch( const std::exception &ex ) {
		cerr << "DB ERROR: " << ex.what() << "." << endl;
		return DBL_MIN;
	}

	return tStop - tStart;

}


double
testPqTuple( const Options &opt, int nLoops )
{
	using namespace pqxx;

	ostringstream ost;
	double tStart, tStop;

	ost << "host=" << opt.getDbHost() << " port=" << opt.getDbPort()
	   << " user=" << opt.getDbUser() << " dbname=" << opt.getDbName();

	connection con( ost.str() );

	if( ! con.is_open() ) {
		cerr << "Failed to connect to DB at: " << ost.str() << "." << endl;
		return DBL_MIN;
	}

	cerr << "Connected to DB at: " << ost.str() << "." << endl;

	try {
		work w( con, "WDB" );
		ost.str("");

		ost << "SELECT wci.begin('" << opt.getWdbUser() << "');";

		w.exec( ost.str() );

		string q = createWdbGetQuery( opt );

		Data d;
		result r = w.exec( q );
		tc::PqContainer pg( r, &w );
		int n;
		n=0;
		tStart = miutil::gettimeofday();

		for( int i=0; i < nLoops; ++i ) {
			for( boost::shared_ptr<tc::IIterator> ri( pg.iterator() ); ri->hasNext() /*&& n<10*/;  ++n ) {
				d = Data( ri->next() );
				//	cerr <<  d << endl;
			}
		}
		tStop = miutil::gettimeofday();
	}
	catch( const std::exception &ex ) {
		cerr << "DB ERROR: " << ex.what() << "." << endl;
		return DBL_MIN;
	}

	return tStop - tStart;
}

bool
writeCVS(  const Options &opt )
{
	using namespace pqxx;

	ostringstream ost;
	string filename = opt.getFilename();
	ofstream fout( filename.c_str() );

	if( ! fout.is_open() ) {
		cerr << "Cant open output file <" << filename << ">." << endl;
		return false;
	}

	ost << "host=" << opt.getDbHost() << " port=" << opt.getDbPort()
		<< " user=" << opt.getDbUser() << " dbname=" << opt.getDbName();

	connection con( ost.str() );

	if( ! con.is_open() ) {
		cerr << "Failed to connect to DB at: " << ost.str() << "." << endl;
		return false;
	}

	cerr << "Connected to DB at: " << ost.str() << "." << endl;

	try {
		work w( con, "WDB" );
		ost.str("");

		ost << "SELECT wci.begin('" << opt.getWdbUser() << "');";

		w.exec( ost.str() );

		string q = createWdbGetQuery( opt );

		result r = w.exec( q );
		tc::PqContainer pg( r, &w );

		if( ! writeCSV( fout, pg ) ) {
			cerr << "Failed to write DB data to file <" << filename << ">." << endl;
			fout.close();
			return false;
		}
		fout.close();
		cerr << "DB data written to file <" << filename << ">." << endl;
		return true;
	}
	catch( const std::exception &ex ) {
		cerr << "DB ERROR: " << ex.what() << "." << endl;
		return false;
	}

}




/*
"value, dataprovidername, placename, referencetime, validtimefrom, validtimeto, "
		"valueparametername, valueparameterunit, levelparametername, "
		"levelunitname, levelfrom, levelto, dataversion, astext(placegeometry) as point";
*/

int
main( int argn, char *argv[] )
{
	Options opt;
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


	cerr << parDefs << endl;

	if( writeCVS( opt ) )
		return 0;
	else
		return 1;


	int loops=1000;

	double t1 = testPqxx( opt, loops );
	double t2 = testPqTuple( opt, loops );

	double percent;

	if( t1 > t2 )
		percent = ((t1-t2)/t1)*100;
	else
		percent = ((t2-t1)/t2)*100;

	cerr << "pqxx: " << t1 << "  pqtuple: " << t2 << "  dif: " << t2-t1  << " ( " << percent << "% )"<< endl;
	return 0;
	getWdbData( opt );
}
