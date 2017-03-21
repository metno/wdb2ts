noinst_LTLIBRARIES += libwdb2ts.la \
					  mod_metno_wdb2ts.la

libwdb2ts_la_SOURCES= \
	src/mod_wdb2ts/transactor/WciTransactor.h \
	src/mod_wdb2ts/transactor/WciReadLocationForecast.h \
	src/mod_wdb2ts/transactor/WciReadLocationForecast.cpp \
   src/mod_wdb2ts/transactor/Topography.h \
   src/mod_wdb2ts/transactor/Topography.cpp \
   src/mod_wdb2ts/transactor/LocationPointRead.h \
   src/mod_wdb2ts/transactor/LocationPointRead.cpp \
   src/mod_wdb2ts/transactor/LocationPointMatrixData.h \
   src/mod_wdb2ts/transactor/LocationPointMatrixData.cpp \
   src/mod_wdb2ts/transactor/ProviderRefTime.h \
   src/mod_wdb2ts/transactor/ProviderRefTime.cpp \
   src/mod_wdb2ts/transactor/ResolveProviderGroups.h \
   src/mod_wdb2ts/transactor/ResolveProviderGroups.cpp \
   src/mod_wdb2ts/transactor/Version.h \
   src/mod_wdb2ts/transactor/Version.cpp \
   src/mod_wdb2ts/transactor/WciRead.cpp \
   src/mod_wdb2ts/transactor/WciRead.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GMLContext.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/LocationForecastGmlHandler.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/LocationForecastGmlHandler.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/EncodeLocationForecastGml.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/EncodeLocationForecastGml.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/EncodeLocationForecastGml2.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/EncodeLocationForecastGml2.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/EncodeLocationForecastGml3.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/EncodeLocationForecastGml3.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/XmlBase.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/XmlTag.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/WdbForecastsTag.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/WdbForecastTag.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/PointTag.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/WdbMomentTags.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/WdbMomentTags.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/TimePeriodTag.h \
   src/mod_wdb2ts/contenthandlers/LocationForecastGml/GML/InstantTimeTag.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/LocationForecastHandler.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/LocationForecastHandler.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecast/LocationForecastHandler2.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/LocationForecastHandler2.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecast/LocationForecastUpdateHandler.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/LocationForecastUpdateHandler.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast2.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast2.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast3.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast3.cpp \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast4.h \
   src/mod_wdb2ts/contenthandlers/LocationForecast/EncodeLocationForecast4.cpp \
	src/mod_wdb2ts/contenthandlers/Location/EncodeCSV.cpp \
   src/mod_wdb2ts/contenthandlers/Location/EncodeCSV.h \
   src/mod_wdb2ts/contenthandlers/Location/LocationHandler.cpp \
   src/mod_wdb2ts/contenthandlers/Location/LocationHandler.h\
   src/mod_wdb2ts/Mutex.h \
   src/mod_wdb2ts/Map.cpp \
   src/mod_wdb2ts/Map.h \
   src/mod_wdb2ts/MapLoader.h \
   src/mod_wdb2ts/MapLoader.cpp \
   src/mod_wdb2ts/matrix.h \
   src/mod_wdb2ts/matrix.cpp \
   src/mod_wdb2ts/ReadMapFile.h \
   src/mod_wdb2ts/ReadMapFile.cpp \
   src/mod_wdb2ts/DbManager.cpp \
   src/mod_wdb2ts/DbManager.h \
   src/mod_wdb2ts/LocationElem.h \
   src/mod_wdb2ts/LocationElem.cpp \
   src/mod_wdb2ts/LocationData.h \
   src/mod_wdb2ts/LocationData.cpp \
   src/mod_wdb2ts/Encode.cpp \
   src/mod_wdb2ts/Encode.h \
   src/mod_wdb2ts/configdata.h \
   src/mod_wdb2ts/configdata.cpp \
   src/mod_wdb2ts/ConfigUtils.h \
   src/mod_wdb2ts/ConfigUtils.cpp \
   src/mod_wdb2ts/ProviderGroups.h \
   src/mod_wdb2ts/ProviderGroups.cpp \
   src/mod_wdb2ts/ProviderGroupsResolve.h \
   src/mod_wdb2ts/ProviderGroupsResolve.cpp \
   src/mod_wdb2ts/ParamDef.h \
   src/mod_wdb2ts/ParamDef.cpp \
   src/mod_wdb2ts/PointDataHelper.cpp \
   src/mod_wdb2ts/PointDataHelper.h \
   src/mod_wdb2ts/Precipitation.cpp \
   src/mod_wdb2ts/Precipitation.h \
   src/mod_wdb2ts/WdbQueryHelper.h \
   src/mod_wdb2ts/WdbQueryHelper.cpp \
   src/mod_wdb2ts/preprocessdata.h \
   src/mod_wdb2ts/preprocessdata.cpp \
   src/mod_wdb2ts/ProviderList.h \
   src/mod_wdb2ts/ProviderList.cpp \
   src/mod_wdb2ts/ProviderListConfigure.h \
   src/mod_wdb2ts/ProviderListConfigure.cpp \
   src/mod_wdb2ts/ProviderReftimes.h \
   src/mod_wdb2ts/ProviderReftimes.cpp \
   src/mod_wdb2ts/UpdateProviderReftimes.h \
   src/mod_wdb2ts/UpdateProviderReftimes.cpp \
   src/mod_wdb2ts/wdb2TsApp.cpp \
   src/mod_wdb2ts/wdb2TsApp.h \
   src/mod_wdb2ts/wdb2TsExceptions.h \
   src/mod_wdb2ts/wdb2tsProfiling.h \
   src/mod_wdb2ts/HandlerBase.cpp \
   src/mod_wdb2ts/HandlerBase.h \
   src/mod_wdb2ts/RequestHandlerFactory.h \
   src/mod_wdb2ts/RequestHandlerFactory.cpp \
   src/mod_wdb2ts/NoteTag.h \
   src/mod_wdb2ts/INoteUpdateListener.h \
   src/mod_wdb2ts/NoteManager.h \
   src/mod_wdb2ts/NoteManager.cpp \
   src/mod_wdb2ts/NoteProviderReftime.h \
   src/mod_wdb2ts/NoteProviderReftime.cpp \
   src/mod_wdb2ts/NoteProviderList.h \
   src/mod_wdb2ts/NoteStringList.h \
   src/mod_wdb2ts/NoteString.h \
   src/mod_wdb2ts/SymbolGenerator.h \
   src/mod_wdb2ts/SymbolGenerator.cpp \
   src/mod_wdb2ts/SymbolHolder.h \
   src/mod_wdb2ts/SymbolHolder.cpp \
   src/mod_wdb2ts/SymbolConf.h \
   src/mod_wdb2ts/SymbolConf.cpp \
   src/mod_wdb2ts/SymbolConfConfigure.h \
   src/mod_wdb2ts/SymbolConfConfigure.cpp \
   src/mod_wdb2ts/MetaModelConf.h \
   src/mod_wdb2ts/MetaModelConf.cpp \
   src/mod_wdb2ts/SymbolContext.h \
   src/mod_wdb2ts/SymbolContext.cpp \
   src/mod_wdb2ts/ProjectionHelper.h\
   src/mod_wdb2ts/ProjectionHelper.cpp \
   src/mod_wdb2ts/ProjectionHelperConfigure.h \
   src/mod_wdb2ts/ProjectionHelperConfigure.cpp \
   src/mod_wdb2ts/probabilityCode.h \
   src/mod_wdb2ts/probabilityCode.cpp \
   src/mod_wdb2ts/WebQuery.h \
   src/mod_wdb2ts/WebQuery.cpp \
   src/mod_wdb2ts/PrecipitationConfig.h \
   src/mod_wdb2ts/PrecipitationConfig.cpp \
   src/mod_wdb2ts/LocationPoint.h \
   src/mod_wdb2ts/LocationPoint.cpp \
   src/mod_wdb2ts/TopoProvider.h \
   src/mod_wdb2ts/TopoProvider.cpp \
   src/mod_wdb2ts/NearestHeight.h \
   src/mod_wdb2ts/NearestHeight.cpp \
   src/mod_wdb2ts/NearestLand.h \
   src/mod_wdb2ts/NearestLand.cpp \
   src/mod_wdb2ts/wciHelper.h \
   src/mod_wdb2ts/wciHelper.cpp \
   src/mod_wdb2ts/WdbDataRequest.h \
   src/mod_wdb2ts/WdbDataRequest.cpp \
   src/mod_wdb2ts/WdbDataRequestCommand.cpp \
   src/mod_wdb2ts/WdbDataRequestCommand.h \
   src/mod_wdb2ts/RequestIterator.h \
   src/mod_wdb2ts/RequestIterator.cpp \
   src/mod_wdb2ts/QueryMaker.h \
   src/mod_wdb2ts/QueryMaker.cpp \
   src/mod_wdb2ts/WeatherSymbol.h \
   src/mod_wdb2ts/WeatherSymbol.cpp 
					  


mod_metno_wdb2ts_la_SOURCES= src/mod_wdb2ts/mod_metno_wdb2ts.cpp

mod_metno_wdb2ts_la_LDFLAGS= -rpath $(libexecdir) \
						     -export-dynamic      \
							 -module              \
							 -avoid-version
							 $(LDFLAGS)
								
mod_metno_wdb2ts_la_LIBADD= \
                        -lwebFW  \
                    	   -lWciWebQuery	\
                    	   -lwdb2tsconfigparser \
                    	   -lXML_locationforecast \
                    	   -lwdb2ts \
                    	   -lmiutil \
                    	   -ltuplecontainer \
                    	   -lpgconpool \
                    	   -lstatd \
                    	   $(putools_LIBS) \
								$(pqxx_LIBS) \
								$(BOOST_FILESYSTEM_LIB) \
								$(BOOST_REGEX_LIB) \
								$(BOOST_THREAD_LIB) \
								$(BOOST_SYSTEM_LIB) \
                    	   -lgfortran
                    	    

noinst_PROGRAMS+= TestWdb2Ts

TestWdb2Ts_SOURCES = src/mod_wdb2ts/testWdb2Ts.cpp

TestWdb2Ts_LDFLAGS= $(LDFLAGS)

TestWdb2Ts_LDADD=\
	-lwdb2ts \
	-lwebFW  \
	-lWciWebQuery	\
	-lwdb2tsconfigparser \
	-lXML_locationforecast \
	-lmiutil \
	-ltuplecontainer \
	-lpgconpool \
	-lstatd \
	$(pumet_LIBS) \
	$(putools_LIBS) \
	$(pqxx_LIBS) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_SYSTEM_LIB) \
	-lgfortran 
					


EXTRA_DIST+= src/mod_wdb2ts/wdb2ts.mk   \
			 src/mod_wdb2ts/Makefile.am \
			 src/mod_wdb2ts/Makefile.in

DISTCLEANFILES +=	src/mod_wdb2ts/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/mod_wdb2ts/all: 

src/mod_wdb2ts/clean: clean
