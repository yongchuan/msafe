dnl $Id$
dnl config.m4 for extension msafe

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(msafe, for msafe support,
dnl Make sure that the comment is aligned:
[  --with-msafe             Include msafe support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(msafe, whether to enable msafe support,
dnl Make sure that the comment is aligned:
dnl [  --enable-msafe           Enable msafe support])

if test "$PHP_MSAFE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-msafe -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/msafe.h"  # you most likely want to change this
  dnl if test -r $PHP_MSAFE/$SEARCH_FOR; then # path given as parameter
  dnl   MSAFE_DIR=$PHP_MSAFE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for msafe files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       MSAFE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$MSAFE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the msafe distribution])
  dnl fi

  dnl # --with-msafe -> add include path
  dnl PHP_ADD_INCLUDE($MSAFE_DIR/include)

  dnl # --with-msafe -> check for lib and symbol presence
  dnl LIBNAME=msafe # you may want to change this
  dnl LIBSYMBOL=msafe # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $MSAFE_DIR/lib, MSAFE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_MSAFELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong msafe lib version or lib not found])
  dnl ],[
  dnl   -L$MSAFE_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(MSAFE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(msafe, msafe.c, $ext_shared)
fi
