# $Id: Makefile.am,v 1.114 2008/08/28 13:57:02 dvermeir Exp $
AM_CLAGS = -g -Wall
# The following line ensures that #include <dvutil/bla.h> will be found
# correctly when installing from another directory than the source
# directory. This is necessary, e.g. for "make distcheck" to succeed.
AM_CPPFLAGS=
#
bin_PROGRAMS= shebang 

shebang_SOURCES = shebang.c

SUFFIXES=%.sh
%:	%.sh
	cat $< >$@
	chmod +x $@

EXTRA_DIST = 
  
# 
check_PROGRAMS	= 

# 
# common directives for all test programs
#
LDADD =
AM_LDFLAGS =
# 
#
TESTS = 

# 
noinst_SCRIPTS = 

CLEANFILES=$(TESTS) $(noinst_SCRIPTS)

