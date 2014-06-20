AC_PREREQ([2.59])
AC_INIT([libcec-daemon], [0.9])
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_HEADERS([config.h])
#
AC_CANONICAL_HOST
#
AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE
#
AC_PROG_CXX
#
AX_CXX_COMPILE_STDCXX_11(,[mandatory])
#
AC_CHECK_LIB([dl], [dlopen])
#
#AC_CHECK_LIB(cec, cec_initialize)
PKG_CHECK_MODULES([LIBCEC], [libcec >= 2.1], [LIBS="${LIBCEC_LIBS} ${LIBS}"], AC_MSG_ERROR("required package libcec is missing"))
#
AC_LANG_PUSH([C++])
#
AC_CHECK_HEADERS([log4cplus/logger.h],, AC_MSG_ERROR("required log4cplus headers are either missing or incomplete"))
AX_CXX_CHECK_LIB([log4cplus], [log4cplus::Logger::getRoot()],, AC_MSG_ERROR("required library log4cplus is missing"))
#
AC_CHECK_HEADERS([boost/program_options.hpp boost/thread/thread.hpp],, AC_MSG_ERROR("required boost headers are either missing or incomplete"))
AX_CXX_CHECK_LIB([boost_program_options], [boost::program_options::positional_options_description],, AC_MSG_ERROR("required library boost is either missing or incomplete"))
AX_CXX_CHECK_LIB([boost_thread], [boost::thread],, AC_MSG_ERROR("required library boost is either missing or incomplete"))
AX_CXX_CHECK_LIB([boost_system], [main],, AC_MSG_ERROR("required library boost is either missing or incomplete"))
AC_CACHE_CHECK([whether boost::program_options::typed_value<> supports value_name() member.],
    [my_cv_boost_po_typed_value_name],
    AC_TRY_COMPILE(["#include <boost/program_options.hpp>"],
        [auto a = boost::program_options::value<int>()->value_name("test")], [my_cv_boost_po_typed_value_name=yes], [my_cv_boost_po_typed_value_name=no]))
if test "$xmy_cv_boost_po_typed_value_name" = xyes; then
    AC_DEFINE([HAVE_BOOST_PO_TYPED_VALUE_NAME], [1], [Define if boost::program_options::typed_value<> supports value_name])
fi
#
AC_LANG_POP
#
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
