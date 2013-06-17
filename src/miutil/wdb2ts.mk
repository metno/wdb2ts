
noinst_LTLIBRARIES+=		libmiutil.la

libmiutil_la_SOURCES= src/miutil/Dir.cpp \
					  src/miutil/Dir.h \
					  src/miutil/copyfile.cpp \
					  src/miutil/copyfile.h \
		              src/miutil/gettimeofday.cpp \
		              src/miutil/gettimeofday.h \
		              src/miutil/mkdir.cpp \
		              src/miutil/mkdir.h \
		              src/miutil/ptimeutil.cpp \
		              src/miutil/ptimeutil.h \
		              src/miutil/replace.cpp \
		              src/miutil/replace.h \
		              src/miutil/trimstr.cpp \
		              src/miutil/trimstr.h \
		              src/miutil/readfile.cpp \
		              src/miutil/readfile.h \
		              src/miutil/splitstr.cpp \
		              src/miutil/splitstr.h \
		              src/miutil/compresspace.h \
		              src/miutil/compresspace.cc \
		              src/miutil/File.cpp \
		              src/miutil/File.h \
		              src/miutil/Timer.cpp \
		              src/miutil/Timer.h \
		              src/miutil/profiling.h \
		              src/miutil/Indent.h \
		              src/miutil/stlContainerUtil.h  \
		              src/miutil/SAXParser.h \
		              src/miutil/SAXParser.cc \
		              src/miutil/Value.h \
		              src/miutil/Value.cc \
		              src/miutil/FileLockHelper.h \
		              src/miutil/FileLockHelper.cpp \
		              src/miutil/StreamReplace.h \
		              src/miutil/StreamReplace.cpp \
		              src/miutil/TempFileStream.h \
		              src/miutil/TempFileStream.cpp \
		              src/miutil/queuemt.h \
		              src/miutil/msleep.h \
		              src/miutil/msleep.cpp \
		              src/miutil/metfunctions.h \
		              src/miutil/metfunctions.cpp \
		              src/miutil/ShapeReader.h \
		              src/miutil/ShapeReader.cpp

EXTRA_DIST+= src/miutil/wdb2ts.mk \
				 src/miutil/Makefile.am \
				 src/miutil/Makefile.in

DISTCLEANFILES +=	src/miutil/Makefile


# Local Makefile Targets
#-----------------------------------------------------------------------------

src/miutil/all: 

src/miutil/clean: clean
                     