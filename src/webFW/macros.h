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



#define MISP_DECLARE_APP( AppClass ) \
   private:                          \
      static AppClass *app##AppClass;\
   public:                           \
      static AppClass* app();        \
   private:                          
   


#define MISP_IMPL_APP( AppClass )         \
   AppClass *AppClass::app##AppClass = 0; \
                                          \
AppClass*                                 \
AppClass::                                \
app()                                     \
{                                         \
   if( ! app##AppClass )                  \
      app##AppClass = new AppClass();     \
                                          \
   return app##AppClass;                  \
}

//static void printProfileData();

#ifdef __MI_PROFILE__ 
#define PRINT_PROFILE_DATA                                     \
{                                                              \
   miutil::Timer *timer=MI_PROFILE_PTR;                        \
                                                               \
   double runtime=timer->getDiff();                            \
   cerr << "runtime:             " << runtime << endl;         \
   cerr << "dispath:             " << (timer->getTimer("dispatch")/runtime)*100 << " % of runtime!" << endl;\
   cerr << "Outputdata (APACHE): " << (timer->getTimer("net")/runtime)*100 << " % of runtime!" << endl;\
}
#else 
#define PRINT_PROFILE_DATA
#endif

       

#define MISP_CREATE_AND_INIT_APP( AppClass, Logger, IAbortManager )   \
AppClass::app()->init( Logger, IAbortManager )

#define MISP_IMPL_APACHE_HANDLER( Name, AppClass ) \
static int                                         \
Name##_handler(request_rec *r)                     \
{                                                  \
   std::ostringstream ost;                         \
   time_t             requestTime;                 \
  /* double tstart=miutil::gettimeofday(); */      \
   int returnType=OK;                              \
                                                   \
   time( &requestTime );                           \
                                                   \
   if (strcmp(r->handler, #Name )) {               \
      /*This request is not for us.*/              \
      return DECLINED;                             \
   }                                               \
                                                   \
   if (r->header_only) {                           \
      ap_set_content_type ( r, "text/plain" );     \
      /*r->content_type = "text/xml";*/            \
      return OK;                                   \
   }                                               \
                                                   \
   r->allowed = (AP_METHOD_BIT << M_GET);          \
                                                   \
   if(r->method_number!=M_GET){                    \
    /*At the momment: only GET request is handled by the module.*/ \
     return DECLINED;  /*Method not allowed*/      \
   }                                               \
                                                   \
   INIT_MI_PROFILE(100);                           \
   USE_MI_PROFILE;                                 \
   MARK_ID_MI_PROFILE("main");                     \
                                                   \
   webfw::ApacheRequest   request( r );            \
   webfw::ApacheResponse  response( r );           \
   webfw::ApacheLogger    logger( r );             \
   AppClass*   myApp = AppClass::app();            \
                                                   \
	ap_set_content_type ( r, "text/plain" );        \
	                                                \
   if( ! myApp )  {                                \
      /*r->content_type = "text/xml"; */           \
      ap_log_error(APLOG_MARK, APLOG_ERR, 0,r->server, "Internal server error (App == 0)!");\
      return HTTP_INTERNAL_SERVER_ERROR;           \
   }                                               \
                                                   \
   MARK_ID_MI_PROFILE("dispatch");                 \
   myApp->dispatch( request, response, logger );   \
   MARK_ID_MI_PROFILE("dispatch");                 \
                                                   \
   std::string error( response.errorDoc() );       \
   ost.str("");                                    \
   /*std::cerr << "Error: " << error << "\n"*/;    \
                                                   \
   switch (response.status()) {                    \
      case webfw::Response::PUT_NOT_SUPPORTED:     \
      case webfw::Response::GET_NOT_SUPPORTED:     \
      case webfw::Response::POST_NOT_SUPPORTED:    \
      case webfw::Response::DELETE_NOT_SUPPORTED:  \
      case webfw::Response::NOT_SUPPORTED:         \
         ost << r->method << ": Not supported!";         \
         logger.error( ost.str() );                      \
         return DECLINED;                                \
      case webfw::Response::NOT_FOUND:                   \
         if( error.empty() )                             \
            ost << "Path NOT found: " << request.urlPath();\
         else                                            \
            ost << error;                                \
                                                         \
         logger.info( ost.str() );                       \
         return HTTP_NOT_FOUND;                          \
      case webfw::Response::NO_DATA:                     \
         if( error.empty() )                             \
            ost << "NO DATA: " << request.urlPath();     \
         else                                            \
            ost << error;                                \
         return HTTP_NOT_FOUND;                          \
      case webfw::Response::INVALID_PATH:                \
         if( error.empty() )                             \
            ost << "NO DATA: " << request.urlPath();     \
         else                                            \
            ost << error;                                \
         return HTTP_BAD_REQUEST;                        \
      case webfw::Response::INVALID_QUERY:               \
         if( error.empty() )                             \
            ost << "NO DATA: " << request.urlQuery();    \
         else                                            \
            ost << error;                                \
         return HTTP_BAD_REQUEST;                        \
      case webfw::Response::INTERNAL_ERROR:              \
         if( error.empty() )                             \
            ost << "Unknown INTERNAL ERROR" << request.urlPath();\
         else                                            \
            ost << error;                                \
         return HTTP_INTERNAL_SERVER_ERROR;              \
      case webfw::Response::SERVICE_UNAVAILABLE:        \
         if( error.empty() )                             \
            ost << "SERVICE_UNAVAILABLE" << request.urlPath();\
         else                                            \
            ost << error;                                \
         return HTTP_SERVICE_UNAVAILABLE;                \
      case webfw::Response::NO_ERROR:                    \
         /*Nothing to do here.*/                         \
         break;                                          \
      default:                                           \
         if( error.empty() )                             \
            ost << "Unknown Response from the request handler!" << request.urlPath();\
         else                                            \
            ost << error;                                \
         return HTTP_INTERNAL_SERVER_ERROR;              \
   }                                                     \
                                                                 \
   MARK_ID_MI_PROFILE( "net" );                                  \
   try{                                                          \
      response.flush();                                          \
   }                                                             \
   catch( ... ) {                                                \
      /* Do nothing */                                           \
   }                                                             \
   MARK_ID_MI_PROFILE( "net" );                                  \
                                                                 \
   MARK_ID_MI_PROFILE("main");                                   \
   PRINT_MI_PROFILE;                                             \
   PRINT_PROFILE_DATA;                                           \
                                                                 \
   return OK;                                                    \
}                                                                \
                                                                 \
                                                                 \
/**                                                                \
 * This function register our module with the Apache core. It also \
 * initialize the ProgApp to be used as a singleton.               \
 */                                                                \
static void                                                        \
Name##_register_hooks(apr_pool_t *p)                               \
{                                                                  \
   /* Initialize as a singleton before it is used in any threads.  \
    * Also call the apps initAction.                               \
    */                                                             \
   webfw::ApacheLogger logger( p );                                \
   MISP_CREATE_AND_INIT_APP( AppClass,                             \
                             logger,                               \
                             new webfw::ApacheAbortHandlerManager() );\
   std::cerr << "-- AppInit: " << AppClass::app()->moduleName()    \
                          << "_register_hooks: called!\n";         \
   /*ap_log_perror(APLOG_MARK, APLOG_DEBUG, 0,p,                    \
         std::string( std::string(AppClass::app()->moduleName()) + \
                      "_register_hooks: called").c_str() );   */     \
                                                                   \
   ap_hook_handler(Name ## _handler, NULL, NULL, APR_HOOK_MIDDLE); \
}                                                                  \
                                                                   \
/* Dispatch list for API hooks */                                  \
module AP_MODULE_DECLARE_DATA Name ## _module = {                  \
    STANDARD20_MODULE_STUFF,                                       \
    NULL,                  /* create per-dir    config structures */ \
    NULL,                  /* merge  per-dir    config structures */ \
    NULL,    /*mod_ts2xml_create_srv_conf,  create per-server config structures */ \
    NULL,                  /* merge  per-server config structures */ \
    NULL,    /*  mod_ts2xml_cmds,                  table of config file commands*/ \
    Name##_register_hooks  /* register hooks                      */ \
}
