dnl Configure paths for librep
dnl $Id$
dnl
dnl AM_PATH_REP([MINIMUM_VERSION])
dnl Test for librep, define REP_VERSION, REP_CFLAGS, REP_LIBS and REP_EXECDIR
dnl
AC_DEFUN(AM_PATH_REP,
[dnl
  min_rep_version=ifelse([$1], ,0.1,$1)
  AC_MSG_CHECKING(for rep - version >= $min_rep_version)
  rep_version=`rep-config --version`
  if test $? -eq 0; then
    rep_major=`echo $rep_version \
	| sed -e 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\(\.\([[0-9]]\+\)\)\?/\1/'`
    rep_minor=`echo $rep_version \
	| sed -e 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\(\.\([[0-9]]\+\)\)\?/\2/'`
    min_rep_major=`echo $min_rep_version \
	| sed -e 's/\([[0-9]]\+\)\.\([[0-9]]\+\)/\1/'`
    min_rep_minor=`echo $min_rep_version \
	| sed -e 's/\([[0-9]]\+\)\.\([[0-9]]\+\)/\2/'`
    if test '(' $rep_major -gt $min_rep_major ')' \
	-o '(' $rep_major -eq $min_rep_major \
	       -a $rep_minor -ge $min_rep_minor ')';
    then
      REP_VERSION="${rep_version}"
      REP_CFLAGS="`rep-config --cflags`"
      REP_LIBS="`rep-config --libs`"
      REP_EXECDIR="`rep-config --execdir`"
      AC_SUBST(REP_VERSION)
      AC_SUBST(REP_CFLAGS)
      AC_SUBST(REP_LIBS)
      AC_SUBST(REP_EXECDIR)
      AC_MSG_RESULT([version ${rep_version}])
    else
      AC_MSG_ERROR([version ${rep_version}; require $min_rep_version])
    fi
  else
    AC_MSG_ERROR([can't find librep; is it installed?])
  fi
])