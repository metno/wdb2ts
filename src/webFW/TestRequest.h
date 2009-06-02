#ifndef __TESTREQUEST_H__
#define __TESTREQUEST_H__

#include <Request.h>

namespace webfw {

class TestRequest : public Request
{
   std::string urlPath_;
   std::string urlQuery_;
   
   public:
      TestRequest( Request::RequestType reqType,
                   const std::string &urlPath, const std::string &urlQuery );
      ~TestRequest();
      
      std::string urlPath()const ;
      std::string urlQuery()const;
};

}

#endif 
