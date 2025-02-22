# -*-Mode: makefile;-*-

## * BeginRiceCopyright *****************************************************
##
## $HeadURL$
## $Id$
##
## --------------------------------------------------------------------------
## Part of HPCToolkit (hpctoolkit.org)
##
## Information about sources of support for research and development of
## HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
## --------------------------------------------------------------------------
##
## Copyright ((c)) 2002-2020, Rice University
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
##
## * Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in the
##   documentation and/or other materials provided with the distribution.
##
## * Neither the name of Rice University (RICE) nor the names of its
##   contributors may be used to endorse or promote products derived from
##   this software without specific prior written permission.
##
## This software is provided by RICE and contributors "as is" and any
## express or implied warranties, including, but not limited to, the
## implied warranties of merchantability and fitness for a particular
## purpose are disclaimed. In no event shall RICE or contributors be
## liable for any direct, indirect, incidental, special, exemplary, or
## consequential damages (including, but not limited to, procurement of
## substitute goods or services; loss of use, data, or profits; or
## business interruption) however caused and on any theory of liability,
## whether in contract, strict liability, or tort (including negligence
## or otherwise) arising in any way out of the use of this software, even
## if advised of the possibility of such damage.
##
## ******************************************************* EndRiceCopyright *

#############################################################################
##
## File:
##   $HeadURL$
##
## Description:
##   *Process with automake to produce Makefile.in*
##
##   Note: All local variables are prefixed with MY to prevent name
##   clashes with automatic automake variables.
##
#############################################################################

# We do not want the standard GNU files (NEWS README AUTHORS ChangeLog...)
AUTOMAKE_OPTIONS = subdir-objects foreign

#############################################################################
# Common settings
#############################################################################

include $(top_srcdir)/src/Makeinclude.config

#############################################################################
# Local settings
#############################################################################

DYNINST_IFLAGS = @DYNINST_IFLAGS@
TBB_IFLAGS = @TBB_IFLAGS@
REDSHOW_INC_FLGS = @OPT_REDSHOW_IFLAGS@

MYSOURCES = \
	CallPath.hpp CallPath.cpp \
	CallPath-MetricComponentsFact.hpp CallPath-MetricComponentsFact.cpp \
	CallPath-CudaCFG.hpp CallPath-CudaCFG.cpp \
	CallPath-DataFlow.hpp CallPath-DataFlow.cpp \
	advisor/GPUAdvisor.hpp advisor/GPUAdvisor-Blame.cpp \
  advisor/GPUAdvisor-Advise.cpp advisor/GPUAdvisor-Init.cpp \
	advisor/GPUOptimizer.hpp advisor/GPUOptimizer.cpp \
	advisor/GPUInstruction.hpp advisor/GPUInstruction.cpp \
	advisor/GPUArchitecture.hpp advisor/GPUArchitecture.cpp \
	advisor/GPUEstimator.hpp advisor/GPUEstimator.cpp \
	advisor/Inspection.hpp advisor/Inspection.cpp \
	\
	MetricNameProfMap.hpp MetricNameProfMap.cpp \
	Flat-SrcCorrelation.hpp Flat-SrcCorrelation.cpp \
	Flat-ObjCorrelation.hpp Flat-ObjCorrelation.cpp \
	\
	Raw.hpp Raw.cpp	\
	\
	Args.hpp Args.cpp \
	ArgsHPCProf.hpp ArgsHPCProf.cpp \
	\
	Util.hpp Util.cpp \
	TextUtil.hpp TextUtil.cpp

# GNU binutils flags are needed for HPCLIB_ISA.
MYCFLAGS   = @HOST_CFLAGS@   $(HPC_IFLAGS) $(REDSHOW_INC_FLGS) @BINUTILS_IFLAGS@
MYCXXFLAGS = @HOST_CXXFLAGS@ $(HPC_IFLAGS) $(REDSHOW_INC_FLGS) @BINUTILS_IFLAGS@ @XERCES_IFLAGS@ @BOOST_IFLAGS@ $(DYNINST_IFLAGS) $(TBB_IFLAGS)

if IS_HOST_AR
  MYAR = @HOST_AR@
else
  MYAR = $(AR) cru
endif

MYLIBADD = @HOST_LIBTREPOSITORY@

MYCLEAN = @HOST_LIBTREPOSITORY@

#############################################################################
# Automake rules
#############################################################################

noinst_LTLIBRARIES = libHPCanalysis.la

libHPCanalysis_la_SOURCES  = $(MYSOURCES)
libHPCanalysis_la_CFLAGS   = $(MYCFLAGS)
libHPCanalysis_la_CXXFLAGS = $(MYCXXFLAGS)
libHPCanalysis_la_AR       = $(MYAR)
libHPCanalysis_la_LIBADD   = $(MYLIBADD)

MOSTLYCLEANFILES = $(MYCLEAN)

#############################################################################
# Common rules
#############################################################################

include $(top_srcdir)/src/Makeinclude.rules

