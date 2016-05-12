# Makefile for diWdb Library
# BM 2007-06-17

LIBOBJ= testSetupParser.o

LIBSRC=$(LIBOBJ:.o=.cc)

BINS=testSetupParser
$(INCLUDE)
CXXFLAGS = -g

INCLUDE=-I/disk1/local/include -I/disk1/local/include/puTools

LIB= -L../lib -ldiWdb  -L/disk1/local/lib -lboost_date_time-mt -lpuTools

VPATH=$(LIBDIR)

.SUFFIXES: .cc .o

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

all: $(BINS)

testSetupParser: testSetupParser.o
	$(CXX) $(CXXLDFLAGS) -o $@ $< -L../lib -ldiWdb

testMiTime2Ptime: testMiTime2Ptime.o
	$(CXX) $(CXXLDFLAGS) -o $@ $< $(LIB)


clean:
	rm -f *.o
	rm -f core.*
	rm -f $(BINS)

veryclean: clean

depends:
	rm -f $(DEPENDSFILE)
	echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CCFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)

$(LIBDIR)/libdiWdb.a: $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $(LIBOBJ)

include $(DEPENDSFILE)
