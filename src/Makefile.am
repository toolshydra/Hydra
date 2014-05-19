SYSTEM     = x86-64_darwin
LIBFORMAT  = static_pic

CPLEXDIR      = /Users/evalentin/Applications/IBM/ILOG/CPLEX_Studio1251/cplex
CONCERTDIR    = /Users/evalentin/Applications/IBM/ILOG/CPLEX_Studio1251/concert
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

AKAROA2DIR    = /Users/evalentin/local/akaroa/
AKAROA2INCDIR   = $(AKAROA2DIR)/include
AKAROA2LIBDIR   = $(AKAROA2DIR)/lib

AM_CPPFLAGS = -I$(AKAROA2INCDIR) -I../include -m64 -O -fPIC -fexceptions -DILO_LINUX -DNDEBUG -DIL_STD -fno-strict-aliasing -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)
AM_LDFLAGS = -L$(AKAROA2LIBDIR) -lakaroa -largs -lfl -lm -DILO_LINUX -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -m64 -lm -lpthread -framework CoreFoundation -framework IOKit -fno-strict-aliasing

bin_PROGRAMS = schedtests pseudosim
schedtests_SOURCES = schedulability.cpp analysis.cpp
pseudosim_SOURCES = pseudosim.cpp analysis.cpp

