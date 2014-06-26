dnl $Id$
dnl config.m4 for extension facial

PHP_ARG_WITH(facial, for facial support,
[  --with-facial             Include facial support])

if test "$PHP_FACIAL" != "no"; then
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/opencv/cv.h"
  if test -r $PHP_FACIAL/$SEARCH_FOR; then # path given as parameter
    OPENCV_DIR=$PHP_FACIAL
  else # search default path list
    AC_MSG_CHECKING([for facial files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        OPENCV_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  dnl
  if test -z "$OPENCV_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the opencv distribution])
  fi

  dnl # --with-facial -> add include path
  PHP_ADD_INCLUDE($OPENCV_DIR/include)

  PHP_CHECK_LIBRARY(opencv_core, cvLoad,
  [
    PHP_ADD_LIBRARY_WITH_PATH(opencv_core, $OPENCV_DIR/lib, FACIAL_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(opencv_objdetect, $OPENCV_DIR/lib, FACIAL_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(opencv_imgproc, $OPENCV_DIR/lib, FACIAL_SHARED_LIBADD)

    AC_DEFINE(HAVE_FACIAL_LIBS,1,[ ])
  ],[
    AC_MSG_ERROR([wrong opencv core version or opencv not found])
  ],[
    -L$OPENCV_DIR/lib -lopencv_core
  ])
  dnl
  PHP_SUBST(FACIAL_SHARED_LIBADD)

  PHP_NEW_EXTENSION(facial, facial.c, $ext_shared)
fi
