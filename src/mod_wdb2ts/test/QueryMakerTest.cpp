/*
    wdb - weather and water data storage

    Copyright (C) 2008 met.no

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

/**
 * @addtogroup wdb2ts
 * @{
 * @addtogroup test
 * @{
 */
/**
 * @file
 * This file contains the UrlParamTest class implementation
 */

#include <iostream>

#include <QueryMakerTest.h>
#include <QueryMaker.h>



using namespace std;
using namespace wdb2ts;
using namespace wdb2ts::qmaker;
namespace pt=boost::posix_time;

namespace {
void printParamdefs( const ParamDefConfig &paramdef, const std::string &heading );
void printConf( Config *conf );
}

CPPUNIT_TEST_SUITE_REGISTRATION( QueryMakerTest );

QueryMakerTest::
QueryMakerTest()
	: testdir(TESTDIR_MOD_WDB2TS)
{

}

QueryMakerTest::
~QueryMakerTest()
{
	// NOOP
}

Config*
QueryMakerTest::
loadConfig( const std::string &file )
{
	ConfigParser parser( testdir );
	Config *conf;
	string confFile( testdir+"/"+file );
	cerr << "loadConfig: '" << confFile << "'" << endl;
	conf = parser.parseFile( confFile );

	cerr << "Messages: " << parser.getErrMsg() << endl << endl;
	return conf;
}


void
QueryMakerTest::
setUp()
{
	// NOOP
}

void
QueryMakerTest::
tearDown()
{
	// NOOP
}


//void
//QueryMakerTest::
//test()
//{
//	string confFile("testconf1-params.xml");
//	Config *conf = loadConfig( confFile );
//	string id;
//	ProviderList providers;
//	PtrProviderRefTimes refTimes( new ProviderRefTimeList() );
//	Level level( 600, 600, "height above ground", "m");
//
//    CPPUNIT_ASSERT_MESSAGE(string("Cant read configuration file '")+confFile+"'.",  conf != 0 );
//
//	(*refTimes)["hirlam"]=pt::time_from_string("2013-03-26 12:00:00");
//	(*refTimes)["ecmwf atmospheric"]=pt::time_from_string("2013-03-26 12:00:00");
//	providers.push_back( ProviderItem("hirlam") );
//
//	CPPUNIT_ASSERT_MESSAGE( "No configuration available.",  conf != 0 );
//
//	QueryMaker *qMaker = QueryMaker::create( conf->paramdef, 3 );
//
//	QuerysAndParamDefsPtr q = qMaker->getWdbReadQuerys( id, 10, 62, providers, level, refTimes );
//
//	cerr << *q << endl;
//
////	UrlParamDataProvider testParam;
////	testParam.decode("");
////	string wciParam = testParam.selectPart();
////	string expResult = "NULL";
////
////	CPPUNIT_ASSERT_EQUAL(  expResult, wciParam );
//}
//
//
//void
//QueryMakerTest::
//test2()
//{
//	string confFile("metno-wdb2ts-conf.xml");
//	string request("/proffecepsforecast");
//	Config *conf = loadConfig( confFile );
//	string id;
//	ProviderList providers;
//	PtrProviderRefTimes refTimes( new ProviderRefTimeList() );
//	Level level( 600, 600, "height above ground", "m");
//
//	CPPUNIT_ASSERT_MESSAGE(string("Cant read configuration file '")+confFile+"'.",  conf != 0 );
//
//	(*refTimes)["hirlam"]=pt::time_from_string("2013-03-26 12:00:00");
//	(*refTimes)["ecmwf atmospheric"]=pt::time_from_string("2013-03-26 12:00:00");
//	providers.push_back( ProviderItem("hirlam") );
//
//	CPPUNIT_ASSERT_MESSAGE( "No configuration available.",  conf != 0 );
//
//	RequestMap::iterator itReq = conf->requests.find( request );
//
//	for( RequestMap::iterator it = conf->requests.begin(); it != conf->requests.end(); ++it )
//		cerr << "Rquest Defined: '"<< it->first << "'" << endl;
//
//	CPPUNIT_ASSERT_MESSAGE(request+" - Not defined in file '" + confFile+ "'.",
//			               itReq != conf->requests.end() );
//
//	printParamdefs( itReq->second->requestDefault.paramdef, request );
//	QueryMaker *qMaker = QueryMaker::create( itReq->second->requestDefault.paramdef, 3 );
//
//	QuerysAndParamDefsPtr q = qMaker->getWdbReadQuerys( "default", 10, 62, providers, level, refTimes );
//
//	cerr << *q << endl;
//
////	UrlParamDataProvider testParam;
////	testParam.decode("");
////	string wciParam = testParam.selectPart();
////	string expResult = "NULL";
////
////	CPPUNIT_ASSERT_EQUAL(  expResult, wciParam );
//}


//void
//QueryMakerTest::testUrlParamDataProviderOne()
//{
//	UrlParamDataProvider testParam;
//	testParam.decode("hirlam");
//	string wciParam = testParam.selectPart();
//	string expResult = "ARRAY[ 'hirlam' ]";
//
//	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
//}
//
//void UrlParamTest::testUrlParamDataProviderMany()
//{
//	UrlParamDataProvider testParam;
//	testParam.decode("hirlam,eps,bracknell,washington");
//	string wciParam = testParam.selectPart();
//	string expResult = "ARRAY[ 'hirlam', 'eps', 'bracknell', 'washington' ]";
//
//	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
//}
//
//
//void UrlParamTest::testUrlParamTimeSpecEmpty()
//{
//	UrlParamTimeSpec testParam;
//	CPPUNIT_ASSERT_THROW ( testParam.decode(""), std::logic_error );
//	/*
//	UrlParamTimeSpec testParam;
//	testParam.decode("");
//	string wciParam = testParam.selectPart();
//	string expResult = "NULL";
//
//	CPPUNIT_ASSERT_EQUAL(  expResult, wciParam );
//	*/
//}
//
//void UrlParamTest::testUrlParamTimeSpecOne()
//{
//	UrlParamTimeSpec testParam;
//	testParam.decode("2008-05-17T12:00:00");
//	string wciParam = testParam.selectPart();
//	string expResult = "('2008-05-17T12:00:00', '2008-05-17T12:00:00', 'exact')";
//
//	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
//}
//
//void UrlParamTest::testUrlParamTimeSpecTwo()
//{
//	UrlParamTimeSpec testParam;
//	testParam.decode("2008-05-17T12:00:00,2008-05-17T23:00:00");
//	string wciParam = testParam.selectPart();
//	string expResult = "('2008-05-17T12:00:00', '2008-05-17T23:00:00', 'exact')";
//
//	CPPUNIT_ASSERT_EQUAL( expResult, wciParam );
//}



namespace {
void
printParamdefs( const ParamDefConfig &paramdef, const std::string &heading )
{
	cerr << endl << endl << "------------------  ParamDefs [" << heading << "]----------------------------------" << endl;

	for( ParamDefConfig::ParamDefs::const_iterator itId = paramdef.idParamDefs.begin();
			itId != paramdef.idParamDefs.end(); ++itId )
	{
		cerr << "[" << itId->first << "] : id" << endl;

		for( wdb2ts::ParamDefList::const_iterator pit = itId->second.begin();
				pit != itId->second.end(); ++pit )
		{
			cerr << "   [" << pit->first << "] " << endl;
			for( std::list<wdb2ts::ParamDef>::const_iterator it = pit->second.begin();
					it != pit->second.end();  ++it )
			{
				cerr << "      " <<  it->alias() << " : " << it->valueparametername()
					 << " (" << it->valueparameterunit() << ")";

				if( it->hasLevelparameters() )
				 cerr << " {" << it->levelparametername() << " (" << it->levelunitname()
					 << ") " << it->levelfrom() << " - " << it->levelto() << "}";
				else
					cerr << "  -- level undefined -- ";

				cerr  << endl;
			}
		}
	}
}

void printConf( Config *conf )
{
	if( conf ) {
		cerr << endl << endl << "*****************************************************************" << endl;
		for( wdb2ts::config::RequestMap::const_iterator it=conf->requests.begin();
			  it!=conf->requests.end();
			  ++it ) {
			cerr << *it->second << endl;
		}

		cerr << endl << endl << "------------------  Querys -----------------------------------" << endl;
		for( Config::QueryDefs::const_iterator it=conf->querys.begin();
		     it!=conf->querys.end();
		     ++it ) {
			cerr << "Query id: " << it->first << " Paralells: " << it->second.dbRequestsInParalells() << endl;
			for( Config::Query::const_iterator itQ=it->second.begin();
				  itQ!=it->second.end();
				  ++itQ )
				cerr << endl << "query[" << itQ->query() << "]" << endl;
			cerr << endl;
		}

		printParamdefs( conf->paramdef, "Global");

		cerr << "******************  WARNINGS  *************************************" << endl;
	}else
		cout << "NOT ok\n";
}

}

/**
 * @}
 *
 * @}
 */
