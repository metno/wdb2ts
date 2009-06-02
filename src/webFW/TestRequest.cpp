#include <TestRequest.h>

webfw::
TestRequest::
TestRequest( Request::RequestType reqType, 
             const std::string &urlPath, const std::string &urlQuery )
   : Request( reqType ), urlPath_( urlPath ), urlQuery_( urlQuery )
{
}

webfw::
TestRequest::
~TestRequest()
{
}
      
std::string 
webfw::
TestRequest::
urlPath()const 
{
   return urlPath_;
}

std::string 
webfw::
TestRequest::
urlQuery()const
{
   return urlQuery_;
}
