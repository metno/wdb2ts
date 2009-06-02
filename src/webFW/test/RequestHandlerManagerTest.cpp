#include <cppunit/config/SourcePrefix.h>
#include <RequestHandlerManagerTest.h>
#include <TestRequestHandler.h>
#include <exception.h>
#include <CErrLogger.h>
#include <DefaultResponse.h>
#include <TestRequest.h>
#include <TestApp.h>

CPPUNIT_TEST_SUITE_REGISTRATION( RequestHandlerManagerTest );

using namespace std;
using namespace webfw;

void
RequestHandlerManagerTest::
setUp()
{
	// NOOP
}

void
RequestHandlerManagerTest::
tearDown() 
{
	// NOOP
}
      
void 
RequestHandlerManagerTest::
testDecodePath()
{
   int major, minor;
   std::string newPath;
      
   std::string path1( "/met/no/1.2" ); 
   std::string path2( "/met/no/1" );
   std::string path3( "/met/no/1.2.3" ); //Error
   std::string path4( "/met/no/" );
   std::string path5( "/met/no" );
   std::string path6( "met/no" );
   std::string path7( "/met" );
   std::string path8( "met" );
   std::string path9( "" );
   std::string path10( "/" );
   std::string path11( "1.2" );
   std::string path12( "/1.2" );
   
   RequestHandlerManager::decodePath( path1, newPath, major, minor ); 
   CPPUNIT_ASSERT_MESSAGE( "path1", 
                           major==1 && minor==2 && newPath=="/met/no" );
        
   RequestHandlerManager::decodePath( path2, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path2",
                           major==1 && minor==0 && newPath=="/met/no" );

   CPPUNIT_ASSERT_THROW_MESSAGE( "path3",
         RequestHandlerManager::decodePath( path3, newPath, major, minor ),
         InvalidPath
   );
       
   RequestHandlerManager::decodePath( path4, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path4", major==0 && minor==0 && newPath=="/met/no" );

   RequestHandlerManager::decodePath( path5, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path5", major==0 && minor==0 && newPath=="/met/no" );
   
   RequestHandlerManager::decodePath( path6, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path6", major==0 && minor==0 && newPath=="/met/no" );
   
   RequestHandlerManager::decodePath( path7, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path7", major==0 && minor==0 && newPath=="/met" );
   
   RequestHandlerManager::decodePath( path8, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path8", major==0 && minor==0 && newPath=="/met" );
   
   RequestHandlerManager::decodePath( path9, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path9", major==0 && minor==0 && newPath=="/" );
   
   RequestHandlerManager::decodePath( path10, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path10", major==0 && minor==0 && newPath=="/" );
   
   RequestHandlerManager::decodePath( path11, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path11", major==1 && minor==2 && newPath=="/" );
   
   RequestHandlerManager::decodePath( path12, newPath, major, minor );
   CPPUNIT_ASSERT_MESSAGE( "path12", major==1 && minor==2 && newPath=="/" );
}



void 
RequestHandlerManagerTest::
testFindRequestHandlerPath()
{
   using namespace std;
   
   DefaultRequestHandlerManager reqHandlerMgr;
   RequestHandlerPtr  reqHandler;
   string path1("/met/no");
   
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 1, "Test01"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 2, "Test02"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 1, 2, "Test12"),
                                    path1 );                                         
                                         
   
   CPPUNIT_ASSERT_MESSAGE( "T 0.1",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/0.1" )) 
                           && string(reqHandler->name())== "Test01" );

   
   CPPUNIT_ASSERT_MESSAGE( "T 0.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/0.2" )) 
                           && string(reqHandler->name())== "Test02" );
   
   
   CPPUNIT_ASSERT_MESSAGE( "T 1.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/1.2" )) 
                           && string(reqHandler->name())== "Test12" );
 
   
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )),
                                 NotFound );
 
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/3.4" )),
                                 NotFound );
 
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( "/my/path" )),
                                 NotFound );

 
   CPPUNIT_ASSERT_THROW_MESSAGE( "T InvalidPath", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/1.V" )),
                                 InvalidPath );
 
 
   reqHandlerMgr.setDefaultRequestHandler( path1, 0, 2 );
   CPPUNIT_ASSERT_MESSAGE( "T 0.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )) 
                           && string(reqHandler->name())== "Test02" );
   
   CPPUNIT_ASSERT_MESSAGE( "T 1.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/1.2" )) 
                           && string(reqHandler->name())== "Test12" );
}

void 
RequestHandlerManagerTest::
testDefaultRequestHandlerPath()
{
   
   DefaultRequestHandlerManager reqHandlerMgr;
   RequestHandlerPtr  reqHandler;
   string path1("/met/no");
   
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 1, "Test01"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 2, "Test02"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 1, 2, "Test12"),
                                    path1 );                                         
   
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )),
                                 NotFound );
    
   reqHandlerMgr.setDefaultRequestHandler( path1, 0, 2 );
   CPPUNIT_ASSERT_MESSAGE( "T 0.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )) 
                           && string(reqHandler->name())== "Test02" );
}

void 
RequestHandlerManagerTest::
testRemoveRequestHandlerPath()
{
   DefaultRequestHandlerManager reqHandlerMgr;
   RequestHandlerPtr  reqHandler;
   string path1("/met/no");
   
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 1, "Test01"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 2, "Test02"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 1, 2, "Test12"),
                                    path1 );                                         


   CPPUNIT_ASSERT_MESSAGE( "T 0.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/0.2" )) 
                           && string(reqHandler->name())== "Test02" );
   
   reqHandlerMgr.removeRequestHandler( path1, 0, 2 );
   
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/0.2" )),
                                 NotFound );
}


void 
RequestHandlerManagerTest::
testRemoveDefaultRequestHandlerPath()
{
   DefaultRequestHandlerManager reqHandlerMgr;
   RequestHandlerPtr  reqHandler;
   string path1("/met/no");
   
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 1, "Test01"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 2, "Test02"),
                                    path1 );
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 1, 2, "Test12"),
                                    path1 );                                         

   CPPUNIT_ASSERT_MESSAGE( "T 0.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/0.2" )) 
                           && string(reqHandler->name())== "Test02" );

    
   reqHandlerMgr.setDefaultRequestHandler( path1, 0, 2 );
   CPPUNIT_ASSERT_MESSAGE( "T 0.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )) 
                           && string(reqHandler->name())== "Test02" );
   
   reqHandlerMgr.removeRequestHandler( path1, 0, 2 );
   
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/0.2" )),
                                 NotFound );
   
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )),
                                 NotFound );

   reqHandlerMgr.setDefaultRequestHandler( path1, 1, 2 );
   CPPUNIT_ASSERT_MESSAGE( "T 1.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )) 
                           && string(reqHandler->name())== "Test12" );

   reqHandlerMgr.removeRequestHandler( path1, 0, 0 );
   
   CPPUNIT_ASSERT_THROW_MESSAGE( "T NotFound", 
                                 (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1 )),
                                 NotFound );
  
   CPPUNIT_ASSERT_MESSAGE( "T 1.2",
                           (reqHandler = reqHandlerMgr.findRequestHandlerPath( path1+"/1.2" )) 
                           && string(reqHandler->name())== "Test12" );
  

}

void 
RequestHandlerManagerTest::
testRequestHandler()
{
   TestApp *app=TestApp::app();
   CErrLogger logger;
   DefaultResponse response;
   TestRequest req( Request::GET, "/met/no/location", "lon=10.0;lat=60.9" );
   
   app->init( logger, 0 );
   
   cerr << "\nAPP name: " << app->moduleName() << endl;
   
   app->dispatch( req, response, logger );
   
   cerr << "ErrDoc: <" << response.errorDoc() << ">\n";
   cerr << "ReqHandler: <" << response.aboutRequestHandler() << ">\n"; 
   cerr << "Response: <" << response.content() << ">\n";
   
   
   
}


