TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#USR_CXXFLAGS=-I/usr/include/mono-2.0
#USR_CXXFLAGS=-clr:safe

#==================================================
# build a support library

USR_INCLUDES += -I..
LIB_INSTALLS += ../libdscs.a
LIB_INSTALLS += ../libdscs.so
BIN_INSTALLS += ../libdscs.so

LIBRARY_IOC += dscsAsyn

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install dbd into <top>/dbd
DBD += dscsAsynSupport.dbd

# specify all source files to be compiled and added to the library
dscsAsyn_SRCS += dscsAsyn.cpp
# dscs_LIBS_Linux += Wrapper
# ifeq (win, $(findstring win, $(T_A)))
# dscs_LIBS += CommsWrapper
# endif

dscsAsyn_LIBS += asyn
dscsAsyn_LIBS += dscs
dscsAsyn_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

