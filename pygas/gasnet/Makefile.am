include @GASNET_CONDUIT_INCLUDE_FILE@

pyexec_LTLIBRARIES = gasnet.la
gasnet_la_SOURCES = gasnet.c
gasnet_la_CPPFLAGS = $(PYTHON_CPPFLAGS) $(GASNET_CPPFLAGS)
gasnet_la_LDFLAGS = $(PYTHON_LDFLAGS) -avoid-version -module $(GASNET_LDFLAGS)
gasnet_la_LIBADD = $(GASNET_LIBS)
