ifdef I3_PORTS
ver	=	gsl-1.14
dir	=	-I$(I3_PORTS)/include/$(ver) -L$(I3_PORTS)/lib/$(ver)
else
dir	=
endif
gsl	=	$(dir) -lgsl -lgslcblas

gcpp	=	$(CXX) -O2 #--fast-math

all:
	$(gcpp) bfr.cxx -o bfr $(gsl)
