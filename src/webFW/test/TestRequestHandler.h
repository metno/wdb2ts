#ifndef __TESTREQUESTHANDLER_H__
#define __TESTREQUESTHANDLER_H__

#include <string>
#include <RequestHandler.h>

class TestRequestHandler : public webfw::RequestHandler
{
   std::string name_;
   
   public:
      TestRequestHandler( int major, int minor, const char *name ):
         webfw::RequestHandler( major, minor ), name_( name ) {}       
   
      const char *name()const { return name_.c_str(); };
      void get( webfw::Request &req, 
                webfw::Response &response, 
                webfw::Logger &logger )
                { response.out() << "Test::get";};
      void post( webfw::Request &req, 
                 webfw::Response &response,
                 webfw::Logger &logger )
                 { response.out() << "Test::post";  };
      void put( webfw::Request &req, 
                webfw::Response &response,
                webfw::Logger &logger )
                { response.out() << "Test::put"; };
      void del( webfw::Request &req, 
                webfw::Response &response,
                webfw::Logger &logger )
                { response.out() << "Test::del"; };
};


#endif
