
#-----------------------------------------------------------------------------
# Documentation for WDB2TS
#-----------------------------------------------------------------------------

# Note: xmlto does not function properly with --searchpath, so in order
# to be able to use validator in our xml editor and get VPATH compiles
# to function properly, we need to perform this rather complicated
# copying of files when compiling.

# Docbook
man1_MANS = metno-wdb2ts.1 \
				metno-wdb2ts_locationforecast_handler.1\
				metno-wdb2ts_location_handler.1
				
metno-wdb2ts.1: doc/user/wdb2ts.man.xml 
	$(DOCBOOK) man $<

metno-wdb2ts_locationforecast_handler.1: doc/user/wdb2ts-locationforecast.man.xml  
	$(DOCBOOK) man $< 

metno-wdb2ts_location_handler.1: doc/user/wdb2ts-location.man.xml
	$(DOCBOOK) man $<


#EXTRA_DIST += doc/man/pgen-probability.xml
CLEANFILES += metno-wdb2ts.1 \
				  metno-wdb2ts_locationforecast_handler.1\
			     metno-wdb2ts_location_handler.1
			

DOCBOOK =					xmlto
DOCBOOK2HTML = 			$(DOCBOOK) html
DOCBOOK2PDF =				$(DOCBOOK) pdf
DOCBOOK2MAN = 				$(DOCBOOK) man 

DOC_WORKDIR= 				@builddir@/doc/work
DOC_INCWORKDIR=			@builddir@/doc/work/docbook
DOC_SRCWORKDIR= 			@builddir@/doc/work

DOCUMENTATION_AUX =	doc/docbook/wdb_entities.ent \
							doc/docbook/xinclude.mod

# Developer Documentation
DOCUMENTATION_SRC = doc/wdb2ts_user-manual.xml\
  						  doc/user/wdb2ts-manual.xml \
                    doc/user/wdb2ts.man.xml \
                    doc/user/wdb2ts-location.man.xml \
                    doc/user/wdb2ts-locationforecast.man.xml

DOCUMENTATION_DOCDIR =		@srcdir@/doc
DOCUMENTATION_HMTLDIR =		@builddir@/doc/html
DOCUMENTATION_PDFDIR =		@builddir@/doc/pdf
DOCUMENTATION_MANDIR =		@builddir@/doc/man

ENTITY_COMPILE=	sed s/__WDB_VERSION__/$(VERSION)/g @srcdir@/doc/docbook/wdb_entities.ent \
						| sed s¤__WDB_LIB_PATH__¤"$(LD_LIBRARY_PATH)"¤g \
						| sed s:__WDB_BUILDDIR__:@abs_builddir@:g \
						| sed s:__WDB_SRCDIR__:@srcdir@:g \
			 			| sed s:__WDB_BINDIR__:@bindir@:g \
			 			| sed s:__WDB_LIBDIR__:@libdir@:g \
			 			| sed s:__WDB_PKGLIBDIR__:$(libdir)/wdb:g \
			 			| sed s:__WDB_PKGDATADIR__:$(pkgdatadir):g \
			 			| sed s:__WDB_DATADIR__:$(wdbdatadir):g \
			 			| sed s:__WDB_SYSCONFDIR__:@sysconfdir@:g \
			 			| sed s:__WDB_LOCALSTATEDIR__:@localstatedir@:g \
			 			> $(DOC_INCWORKDIR)/wdb_entities.ent

COPY_DOCFILES=  mkdir -p $(DOC_WORKDIR); \
				mkdir -p $(DOC_INCWORKDIR); \
				mkdir -p $(DOC_SRCWORKDIR); \
				mkdir -p $(DOC_SRCWORKDIR)/user; \
				cp $(DOCUMENTATION_DOCDIR)/*.xml \
					$(DOC_SRCWORKDIR); \
				cp $(DOCUMENTATION_DOCDIR)/user/*.xml \
					$(DOC_SRCWORKDIR)/user; \
				cp @srcdir@/doc/docbook/xinclude.mod \
					$(DOC_INCWORKDIR)
					 			 
CLEAN_DOCFILES= rm -rf $(DOC_WORKDIR)


# HTML Documentation
#-----------------------------------------------------------------------------

#html-local:
#				@echo "Creating HTML documentation..."
#				@$(COPY_DOCFILES)
#				@$(ENTITY_COMPILE)
#				@echo "* WDB2TS Documentation"
#				@-rm -rf $(DOCUMENTATION_HMTLDIR)
#				@mkdir -p $(DOCUMENTATION_HMTLDIR)/wdb2ts_user-manual
#				@mkdir -p $(DOCUMENTATION_HMTLDIR)/wdb2ts
#				@mkdir -p $(DOCUMENTATION_HMTLDIR)/wdb2ts_location-handler
#				@mkdir -p $(DOCUMENTATION_HMTLDIR)/wdb2ts_locationforecast-handler
#				@$(DOCBOOK2HTML) \
#					-o $(DOCUMENTATION_HMTLDIR)/wdb2ts_user-manual \
#					$(DOC_SRCWORKDIR)/wdb2ts_user-manual.xml
#				@$(DOCBOOK2HTML) \
#					-o $(DOCUMENTATION_HMTLDIR)/wdb2ts \
#					$(DOC_SRCWORKDIR)/user/wdb2ts.man.xml
#				@$(DOCBOOK2HTML) \
#					-o $(DOCUMENTATION_HMTLDIR)/wdb2ts_location-handler \
#					$(DOC_SRCWORKDIR)/user/wdb2ts-location.man.xml
#				@$(DOCBOOK2HTML) \
#					-o $(DOCUMENTATION_HMTLDIR)/wdb2ts_locationforecast-handler \
#					$(DOC_SRCWORKDIR)/user/wdb2ts-locationforecast.man.xml
#				@$(CLEAN_DOCFILES)
#				@echo "* HTML documentation... done"


html-local:
				@echo "Creating HTML documentation..."
				@$(COPY_DOCFILES)
				@$(ENTITY_COMPILE)
				@echo "* WDB2TS Documentation"
				@-rm -rf $(DOCUMENTATION_HMTLDIR)
				@mkdir -p $(DOCUMENTATION_HMTLDIR)/wdb2ts-manual
				@$(DOCBOOK2HTML) \
					-o $(DOCUMENTATION_HMTLDIR)/wdb2ts-manual \
					$(DOC_SRCWORKDIR)/user/wdb2ts-manual.xml
				@$(CLEAN_DOCFILES)
				@echo "* HTML documentation... done"



# man Documentation
#-----------------------------------------------------------------------------

man-local:
				@echo "Creating man pages..."
				@$(COPY_DOCFILES)
				@$(ENTITY_COMPILE)
				@echo "* WDB2TS man pages"
				@-rm -rf $(DOCUMENTATION_MANDIR)
				@mkdir -p $(DOCUMENTATION_MANDIR)
				@$(DOCBOOK2MAN) \
					-o $(DOCUMENTATION_MANDIR) \
					$(DOC_SRCWORKDIR)/user/wdb2ts.man.xml
				@$(DOCBOOK2MAN) \
					-o $(DOCUMENTATION_MANDIR) \
					$(DOC_SRCWORKDIR)/user/wdb2ts-location.man.xml
				@$(DOCBOOK2MAN) \
					-o $(DOCUMENTATION_MANDIR) \
					$(DOC_SRCWORKDIR)/user/wdb2ts-locationforecast.man.xml
				@$(CLEAN_DOCFILES)
				@echo "* man pages... done"


# Cleaning
#-----------------------------------------------------------------------------

DOCCLEAN_HOOK =	rm -rf @builddir@/doc/html; \
						rm -rf @builddir@/doc/pdf; \
						rm -rf @builddir@/doc/man

EXTRA_DIST += 		$(DOCUMENTATION_SRC) \
						$(DOCUMENTATION_AUX) \
						doc/wdb2ts.mk

# Local Makefile Targets
#-----------------------------------------------------------------------------

doc/all: html-local man-local

doc/clean: clean
