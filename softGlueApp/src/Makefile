TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
#DBD_INSTALLS += softGlueSupport.dbd
DBD += softGlueSupport.dbd

INC += drvIP_EP201.h

LIBRARY_IOC = softGlue
LIB_SRCS += drvIP_EP201.c devAsynSoftGlue.c
#LIB_SRCS += sampleCustomInterruptHandler.c

LIB_LIBS += asyn Ipac

LIB_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

