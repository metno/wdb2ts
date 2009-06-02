#include <iostream>
#include <TestApp.h>
#include <TestRequestHandler.h>
 
using namespace std;
 

MISP_IMPL_APP( TestApp );
   
TestApp::
TestApp()
   : webfw::App()
{
}

void   
TestApp::
initAction( webfw::RequestHandlerManager&  reqHandlerMgr,
            webfw::Logger &logger )
{
   cerr << " -- TestApp::initAction\n";
   reqHandlerMgr.addRequestHandler( new TestRequestHandler( 0, 1, "TestHandler" ), 
                                    "/location" );
   reqHandlerMgr.setDefaultRequestHandler( "/location", 0, 1 );
}

