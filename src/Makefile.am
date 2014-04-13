SYSTEM     = x86-64_darwin
LIBFORMAT  = static_pic

CPLEXDIR      = /Users/evalentin/Applications/IBM/ILOG/CPLEX_Studio1251/cplex
CONCERTDIR    = /Users/evalentin/Applications/IBM/ILOG/CPLEX_Studio1251/concert
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)


AM_CPPFLAGS = -I../include -m64 -O -fPIC -fexceptions -DILO_LINUX -DNDEBUG -DIL_STD -fno-strict-aliasing -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)
AM_LDFLAGS = -DILO_LINUX -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -m64 -lm -lpthread -framework CoreFoundation -framework IOKit -fno-strict-aliasing

bin_PROGRAMS = schedtests
schedtests_SOURCES = schedulability.cpp analysis.cpp

