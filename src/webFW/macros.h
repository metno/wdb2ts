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

#include <trimstr.h>
#include <boost/thread.hpp>

#define MISP_DECLARE_APP( AppClass ) \
   private:                          \
      static boost::mutex mutex##AppClass; \
      static AppClass *app##AppClass;\
   public:                           \
      static AppClass* app();        \
   private:                          
   


#define MISP_IMPL_APP( AppClass )         \
   AppClass *AppClass::app##AppClass = 0; \
   boost::mutex AppClass::mutex##AppClass; \
                                          \
AppClass*                                 \
AppClass::                                \
app()                                     \
{                                         \
   boost::mutex::scoped_lock lock(mutex##AppClass); \
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
                                                   \
struct Name##_conf {                               \
	const char *confpath;                           \
	const char *logpath;                            \
	const char *tmppath;                            \
};                                                 \
                                                   \
struct Name##_app_conf {                           \
   apr_pool_t *pool; /*Holds only a pointer to the App instance.*/  \
};                                                 \
                                                   \
static const char*                                 \
Name##_set_confpath( cmd_parms *cmd, void *dummy, const char *arg );  \
	                                                                   \
static const char*                                                    \
Name##_set_logpath( cmd_parms *cmd, void *dummy, const char *arg );   \
	                                                                   \
static const char*                                                    \
Name##_set_tmppath( cmd_parms *cmd, void *dummy, const char *arg );   \
                                                                      \
static int                                         \
Name##_handler( request_rec *r );                  \
	                                                \
static void                                        \
Name##_register_hooks( apr_pool_t *p );            \
                                                   \
static const command_rec Name##_cmds[] = {         \
	AP_INIT_TAKE1( #Name "_confpath",               \
	               (const char* (*)())Name##_set_confpath, \
	               NULL,                                   \
	               RSRC_CONF,                              \
				   #Name "_confpath, where is the configuration files located. Default is undefined or compiled in." \
				 ),                                \
    AP_INIT_TAKE1( #Name "_logpath",              \
	               (const char* (*)())Name##_set_logpath, \
	               NULL,                                   \
	               RSRC_CONF,                              \
				   #Name "_logpath, where shall the log files be written. Default is undefined or compiled in." \
                 ),                                \
    AP_INIT_TAKE1( #Name "_tmppath",              \
	               (const char* (*)())Name##_set_tmppath, \
	               NULL,                                   \
	               RSRC_CONF,                              \
				   #Name "_tmppath, where shall the temporary files be written. Default is \"/tmp\" in." \
				 ),                                \
	             { NULL }                          \
};                                                 \
                                                   \
static void*                                       \
Name##_create_srv_conf(apr_pool_t* pool, server_rec* svr) \
{                                                         \
	Name##_conf *conf= (Name##_conf*) apr_pcalloc( pool, sizeof( Name##_conf ) ); \
	conf->confpath = apr_pstrdup (pool, "");              \
	conf->logpath = apr_pstrdup (pool, "");               \
	conf->tmppath = apr_pstrdup (pool, "");               \
	return conf;                                          \
}                                                         \
                                                          \
/* Dispatch list for API hooks */                                  \
module AP_MODULE_DECLARE_DATA Name ## _module = {                  \
    STANDARD20_MODULE_STUFF,                                       \
    NULL,                   /* create per-dir    config structures */ \
    NULL,                   /* merge  per-dir    config structures */ \
    Name##_create_srv_conf, /*create per-server config structures */ \
    NULL,                   /* merge  per-server config structures */ \
    Name##_cmds,            /*  table of config file commands*/ \
    Name##_register_hooks   /* register hooks                      */ \
};                                                                  \
                                                                   \
int                                         \
Name##_handler(request_rec *r)                     \
{                                                  \
   std::ostringstream ost;                         \
   time_t             requestTime;                 \
  /* double tstart=miutil::gettimeofday(); */      \
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
   server_rec *s = r->server;                      \
   Name##_conf *conf=(Name##_conf*)ap_get_module_config( s->module_config, \
                                                         &Name##_module);  \
   myApp->setPathsFromConffile( conf->confpath, conf->logpath, conf->tmppath ); \
                                                   \
   MARK_ID_MI_PROFILE("dispatch");                 \
   myApp->dispatch( request, response, logger );   \
   MARK_ID_MI_PROFILE("dispatch");                 \
                                                   \
   std::string error( response.errorDoc() );       \
   if( ! error.empty() ) {                         \
      logger.error( error );                       \
   }                                               \
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
            ost << "Path NOT found: '" << request.urlPath() << "'";\
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
      case webfw::Response::CONFIG_ERROR:                \
		  ost << error;                                  \
         return HTTP_INTERNAL_SERVER_ERROR;              \
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
apr_status_t                                                     \
Name##_app_onshutdown( void *p )                                 \
{                                                                \
   AppClass *app = static_cast<AppClass*>( p );                  \
   app->onShutdown();                                            \
   return  APR_SUCCESS;                                          \
}                                                                \
/**                                                                \
 * This function register our module with the Apache core. It also \
 * initialize the ProgApp to be used as a singleton.               \
 */                                                                \
void                                                               \
Name##_child_init( apr_pool_t *pchild, server_rec *s )             \
{                                                                  \
   std::cerr << "++ AppInit: child_init: " << AppClass::app()->moduleName()    \
             << " hook: called!\n";         \
   /* Initialize as a singleton before it is used in any threads.  \
    * Also call the apps initAction.                               \
    */                                                             \
   webfw::ApacheLogger logger( pchild );                           \
   MISP_CREATE_AND_INIT_APP( AppClass,                             \
                             logger,                               \
                             new webfw::ApacheAbortHandlerManager() );      \
  apr_pool_cleanup_register( pchild, static_cast<void*>(AppClass::app()),   \
                             Name##_app_onshutdown, apr_pool_cleanup_null ); \
}                                                                  \
                                                                   \
                                                                   \
/**                                                                \
 * This function register our module with the Apache core. It also \
 * initialize the ProgApp to be used as a singleton.               \
 */                                                                \
void                                                        \
Name##_register_hooks(apr_pool_t *p)                               \
{                                                                  \
   /* Initialize as a singleton before it is used in any threads.  \
    * Also call the apps initAction.                               \
    */                                                             \
    std::cerr << "-- AppInit: register_hooks: " << AppClass::app()->moduleName() << std::endl;    \
/*   webfw::ApacheLogger logger( p );                                \
   MISP_CREATE_AND_INIT_APP( AppClass,                             \
                             logger,                               \
                             new webfw::ApacheAbortHandlerManager() );\
                             << "_register_hooks: called!\n";         \
*/                                                                   \
   ap_hook_child_init(Name ## _child_init, NULL, NULL, APR_HOOK_MIDDLE); \
   ap_hook_handler(Name ## _handler, NULL, NULL, APR_HOOK_MIDDLE); \
}                                                                  \
                                                                   \
                                                                   \
const char*                                        \
Name##_set_confpath( cmd_parms *cmd, void *dummy, const char *arg) \
{                                                                      \
	server_rec      *s = cmd->server;                                  \
	Name##_conf *conf = (Name##_conf*) ap_get_module_config ( s->module_config,   \
			                                                  &Name##_module    );\
                                                                           \
    std::string buf( arg );                                            \
    miutil::trimstr( buf );                                            \
    conf->confpath = apr_pstrdup( cmd->pool, buf.c_str() );            \
                                                                       \
    return NULL;                                                       \
}                                                                      \
                                                                       \
const char*                                                            \
Name##_set_logpath( cmd_parms *cmd, void *dummy, const char *arg) \
{                                                                      \
   	server_rec      *s = cmd->server;                                  \
   	Name##_conf *conf = (Name##_conf*) ap_get_module_config ( s->module_config,   \
                                                              &Name##_module    );\
                                                                       \
   	std::string buf( arg );                                          \
   	miutil::trimstr( buf );                                          \
   	conf->logpath = apr_pstrdup ( cmd->pool, buf.c_str() );          \
   	                                                                 \
   	return NULL;                                                     \
}                                                                      \
	                                                                    \
const char*                                                            \
Name##_set_tmppath( cmd_parms *cmd, void *dummy, const char *arg) \
{                                                                      \
   	server_rec      *s = cmd->server;                                  \
   	Name##_conf *conf = (Name##_conf*) ap_get_module_config ( s->module_config,   \
                                                              &Name##_module    );\
                                                                       \
   	std::string buf( arg );                                            \
   	miutil::trimstr( buf );                                            \
   	conf->tmppath = apr_pstrdup (cmd->pool, buf.c_str() );                  \
   	                                                                   \
   	return NULL;                                                       \
}

