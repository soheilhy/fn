AC_PREREQ(2.64)
AC_INIT([func], [0.1], [soheil@cs.toronto.edu])

AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_cxx_compile_stdcxx_11.m4])

AM_INIT_AUTOMAKE([foreign])

: ${CXX_FLAGS="-std=c++11 -g -O0"}

AC_PROG_CXX
AC_LANG([C++])

AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

AC_CHECK_HEADER_STDBOOL
AC_CHECK_HEADER(memory)
AC_CHECK_HEADER(string)
AC_CHECK_HEADER(vector)
AC_CHECK_HEADER(unordered_map)
AC_CHECK_HEADER(utility)

AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])


AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([examples/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT

