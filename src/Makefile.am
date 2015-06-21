SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio1251/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio1251/concert
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

AKAROA2DIR    = /usr/local/akaroa
AKAROA2INCDIR   = $(AKAROA2DIR)/include
AKAROA2LIBDIR   = $(AKAROA2DIR)/lib


AM_CPPFLAGS = -O0 -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -I../include -I$(CONCERTINCDIR) -I$(CPLEXINCDIR) -I$(AKAROA2INCDIR)

bin_PROGRAMS = schedtests solver_mgap_rm_enrico solver_mgap_edf pseudosim hydra_solver
schedtests_SOURCES = schedulability.cpp analysis.cpp
schedtests_LDADD = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread
solver_mgap_rm_enrico_SOURCES = solver_mgap_rm_enrico.cpp analysis.cpp
solver_mgap_rm_enrico_LDADD = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread
solver_mgap_edf_SOURCES = solver_mgap_edf.cpp analysis.cpp
solver_mgap_edf_LDADD = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread
pseudosim_SOURCES = pseudosim.cpp analysis.cpp
pseudosim_LDADD = -L$(AKAROA2LIBDIR) -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread -lakaroa -largs -lfl
hydra_solver_SOURCES = hydra_solver.cpp analysis.cpp
hydra_solver_LDADD = -L$(AKAROA2LIBDIR) -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lconcert -lilocplex -lcplex -lm -lpthread -lakaroa -largs -lfl





