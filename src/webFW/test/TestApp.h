#ifndef __TESTAPP_H__
#define __TESTAPP_H__

#include <macros.h>
#include <App.h>

class TestApp: 
   public webfw::App
{
   MISP_DECLARE_APP( TestApp );
   
   protected:
      virtual void initAction( webfw::RequestHandlerManager& reqHandlerMgr,
                               webfw::Logger &logger );
   
   
   public:
      TestApp();
      ~TestApp();
      
      virtual char *moduleName()const { return "TestApp"; }    
};
#endif 

