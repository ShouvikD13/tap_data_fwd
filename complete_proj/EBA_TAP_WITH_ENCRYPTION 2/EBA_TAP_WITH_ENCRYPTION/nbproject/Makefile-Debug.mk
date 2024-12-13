#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Config.o \
	${OBJECTDIR}/Tap_interface.o \
	${OBJECTDIR}/UserPipe.o \
	${OBJECTDIR}/exg_comm.o \
	${OBJECTDIR}/exg_data.o \
	${OBJECTDIR}/gateway_comm.o \
	${OBJECTDIR}/genLogger.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/md5.o \
	${OBJECTDIR}/nse_xchng_mgs.o \
	${OBJECTDIR}/pipe_handler.o \
	${OBJECTDIR}/signal_handlers.o \
	${OBJECTDIR}/slidingWindow.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m32
CXXFLAGS=-m32

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/home/ebaadm/boost_1_57_0_32/boost_1_57_0/stage/lib -Wl,-rpath,'/home/ebaadm/boost_1_57_0_32/boost_1_57_0/stage/lib' -lboost_log_setup -lboost_log -lboost_system -lboost_thread -lpthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/eba_tap

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/eba_tap: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/eba_tap ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Config.o: Config.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Config.o Config.cpp

${OBJECTDIR}/Tap_interface.o: Tap_interface.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Tap_interface.o Tap_interface.cpp

${OBJECTDIR}/UserPipe.o: UserPipe.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UserPipe.o UserPipe.cpp

${OBJECTDIR}/exg_comm.o: exg_comm.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/exg_comm.o exg_comm.cpp

${OBJECTDIR}/exg_data.o: exg_data.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/exg_data.o exg_data.cpp

${OBJECTDIR}/gateway_comm.o: gateway_comm.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/gateway_comm.o gateway_comm.cpp

${OBJECTDIR}/genLogger.o: genLogger.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/genLogger.o genLogger.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/md5.o: md5.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/md5.o md5.cpp

${OBJECTDIR}/nse_xchng_mgs.o: nse_xchng_mgs.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nse_xchng_mgs.o nse_xchng_mgs.cpp

${OBJECTDIR}/pipe_handler.o: pipe_handler.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pipe_handler.o pipe_handler.cpp

${OBJECTDIR}/signal_handlers.o: signal_handlers.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/signal_handlers.o signal_handlers.cpp

${OBJECTDIR}/slidingWindow.o: slidingWindow.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/ebaadm/boost_1_57_0_32/boost_1_57_0 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/slidingWindow.o slidingWindow.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
