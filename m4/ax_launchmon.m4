################################################################################
# Copyright (c) 2010-2012 Krell Institute. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#################################################################################

################################################################################
# Check for libdwarf (http://www.reality.sgiweb.org/davea/dwarf.html)
################################################################################

AC_DEFUN([AX_LAUNCHMON], [
  AC_ARG_WITH(launchmon,
    [AS_HELP_STRING([--with-launchmon=prefix],
      [Add the compile and link search paths for launchmon]
    )],
    [CXXFLAGS="$CXXFLAGS -I${withval}/include"
       LAUNCHMONPREFIX="${withval}"
       LDFLAGS="$LDFLAGS -L${withval}/lib -Wl,-rpath=${withval}/lib"
    ],
    [CXXFLAGS="$CXXFLAGS"
      LAUNCHMONPREFIX=""
    ]
  )
  AC_LANG_PUSH(C++)
  AC_CHECK_HEADERS(lmon_api/lmon_fe.h, 
    [],
    [AC_MSG_ERROR([lmon_api/lmon_fe.h is required.  Specify launchmon prefix with --with-launchmon])],
    AC_INCLUDES_DEFAULT
  )
  AC_CHECK_HEADERS(lmon_api/lmon_be.h, 
    [], 
    [AC_MSG_ERROR([lmon_api/lmon_be.h is required.  Specify launchmon prefix with --with-launchmon])],
    AC_INCLUDES_DEFAULT
  )
  AC_CHECK_LIB(monfeapi,LMON_fe_createSession,libmonfeapi_found=yes,libmonfeapi_found=no)
  if test "$libmonfeapi_found" = yes; then
    FELIBS="$FELIBS -lmonfeapi"
  else
    AC_MSG_ERROR([libmonfeapi is required.  Specify libmonfeapi prefix with --with-launchmon])
  fi
  AC_CHECK_LIB(monbeapi,LMON_be_init,libmonbeapi_found=yes,libmonbeapi_found=no)
  if test "$libmonbeapi_found" = yes; then
    BELIBS="$BELIBS -lmonbeapi"
  else
    AC_MSG_ERROR([libmonbeapi is required.  Specify libmonbeapi prefix with --with-launchmon])
  fi
  AC_LANG_POP(C++)
  AC_PATH_PROG([LAUNCHMONBIN], [launchmon], [no], [$LAUNCHMONPREFIX/bin$PATH_SEPARATOR$PATH])
  if test $LAUNCHMONBIN = no; then
    AC_MSG_ERROR([the launchmon executable is required.  Specify launchmon prefix with --with-launchmon])
  fi
AC_SUBST(LAUNCHMONBIN)
])
