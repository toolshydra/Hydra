SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio1261/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio1261/concert
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

AKAROA2DIR    = /opt/akaroa2
AKAROA2INCDIR   = $(AKAROA2DIR)/include
AKAROA2LIBDIR   = $(AKAROA2DIR)/lib


AM_CPPFLAGS = -O0 -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -I../../include -I$(CONCERTINCDIR) -I$(CPLEXINCDIR) -I$(AKAROA2INCDIR)

bin_PROGRAMS = geneticmgap geneticmgap_rm_util geneticmgap_rm_resp

geneticmgap_SOURCES = fitnesscalcpga.cpp geneticalgorithm.cpp individuals.cpp main.cpp population.cpp ../analysis.cpp
geneticmgap_LDADD = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread
geneticmgap_rm_util_SOURCES = fitnesscalcpga_rmutil.cpp geneticalgorithm.cpp individuals.cpp main.cpp population.cpp ../analysis.cpp
geneticmgap_rm_util_LDADD = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread
geneticmgap_rm_resp_SOURCES = fitnesscalcpga_rmresp.cpp geneticalgorithm.cpp individuals.cpp main.cpp population.cpp ../analysis.cpp
geneticmgap_rm_resp_LDADD = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread
