SHELL=/bin/bash
#ext = $(shell mexext)
# MATLAB = /Applications/MATLAB_R2016b.app
# MEXEXT = .mexmaci64
MEXARCH = mexmaci64
MEX  = /Applications/MATLAB_R2016b.app/bin/mex
# MEXARCH = mexmaci64
# MEXOPTS = -lmwblas -largeArrayDims
CPPFLAGS = -I/Users/yunyunni/Downloads/coda-2.17.3/ -I/Applications/MATLAB_R2016b.app/extern/include 
# matlab = /Applications/MATLAB_R2016b.app/bin/matlab
# matlabmexexecdir = ${pkglibdir}/matlab
# FILES = \
# 		yprime.c

COMPILE = $(MEX) $(CPPFLAGS) 

yprime.@MEXARCH@ : yprime.c 
		$(MEX) yprime.c
