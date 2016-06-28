# $Id: Makefile.am,v 1.114 2008/08/28 13:57:02 dvermeir Exp $
AM_CFLAGS = -g -Wall
# The following line ensures that #include <dvutil/bla.h> will be found
# correctly when installing from another directory than the source
# directory. This is necessary, e.g. for "make distcheck" to succeed.
AM_CPPFLAGS=
#
SUFFIXES=%.sh
%:	%.sh
	cat $< >$@
	chmod +x $@

# This is used to prevent 'make distcheck' complaining about non-deleted README
dist-hook: README
	chmod u+w $(distdir) # $(distdir) may not be writable
	mv README $(distdir)

README: shebang.man
	MANWIDTH=80 man ${srcdir}/$< >$@ # 80 chars are shown properly in Github

bin_PROGRAMS= shebang 

shebang_SOURCES = shebang.c

man1_MANS = shebang.man
dist_man_MANS = shebang.man

EXTRA_DIST = README http-grab.sh \
	test1.sh test2.sh test3.sh
						 

  
# 
# 
# common directives for all test programs
#
LDADD =
AM_LDFLAGS =
# 
#
TESTS = test1 test2 test3
# TESTS_ENVIRONMENT = cd $(srcdir) &&
# 
noinst_SCRIPTS = 

CLEANFILES=$(TESTS) $(noinst_SCRIPTS)

