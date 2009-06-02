noinst_LTLIBRARIES +=   libXML_locationforecast.la 

libXML_locationforecast_la_SOURCES= \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/IXmlTemplate.h \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/LocationTag.h \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/MomentTags.h \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/MomentTags.cpp \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/ProductTag.h \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/PrecipitationTags.h \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/PrecipitationTags.cpp \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/PrecipitationPercentileTags.h \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/PrecipitationPercentileTags.cpp \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/TimeTag.h \
                       src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/WeatherdataTag.h 
	
EXTRA_DIST+= src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/wdb2ts.mk \
				 src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/Makefile.am \
				 src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/Makefile.in

DISTCLEANFILES +=	src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/all: 

src/mod_wdb2ts/contenthandlers/LocationForecast/XML_locationforecast/clean: clean
	