
AC_DEFUN([BOSL_SET_FLAG], [
  # add symbol strip and garbage sections for release
  AS_IF([test "x$enable_release" == "xyes"], [
    AX_APPEND_COMPILE_FLAGS([-O3 -ffunction-sections -fdata-sections])
    AX_APPEND_LINK_FLAGS([-Wl,-s])
    AX_APPEND_LINK_FLAGS([-Wl,--gc-sections])
  ])

  # Compilation flags
  AX_APPEND_COMPILE_FLAGS([-fstack-protector-all -Wstack-protector])
  #FIXME: ENABLE ADDRESS SINITIZER
  #AX_APPEND_COMPILE_FLAGS([-fsanitize=address])
  #AX_APPEND_LINK_FLAGS([-lasan])
  # warnings
  AX_APPEND_COMPILE_FLAGS([-Wall -Werror -Wextra -Wpedantic])
  AX_APPEND_COMPILE_FLAGS([-Wconversion -Wpacked -Wredundant-decls])
  AX_APPEND_COMPILE_FLAGS([-Wmisleading-indentation -Wundef])
  AX_APPEND_COMPILE_FLAGS([-Wpacked-bitfield-compat -Wrestrict])
  AX_APPEND_COMPILE_FLAGS([-Wpacked-not-aligned -Wstrict-prototypes])
  AX_APPEND_COMPILE_FLAGS([-Wmissing-prototypes -Wshadow])
  AX_APPEND_COMPILE_FLAGS([-Wmissing-noreturn -Wmissing-format-attribute])
  AX_APPEND_COMPILE_FLAGS([-Wduplicated-branches -Wduplicated-cond])
  # generic
  AX_APPEND_COMPILE_FLAGS([-fno-exceptions -std=c18])
  AX_APPEND_COMPILE_FLAGS([-fomit-frame-pointer])
  # flag to disable libcheck issues with clang
  AX_APPEND_COMPILE_FLAGS([-Wno-gnu-zero-variadic-macro-arguments])

  # automatic macro include
  AX_APPEND_COMPILE_FLAGS([-I${ac_pwd}])
  AX_APPEND_COMPILE_FLAGS([-imacros\ $($BOSL_READLINK -f ${srcdir})/config.h])
  # information about compiling project
  AX_APPEND_COMPILE_FLAGS([-D_COMPILING_BOSL])

  # custom optimization level if coverage is not defined
  AS_IF([test "x$enable_code_coverage" = "xyes"], [
      # additional stuff for coverage reports
      AX_APPEND_COMPILE_FLAGS([-fno-inline-small-functions])
      AX_APPEND_COMPILE_FLAGS([-fkeep-inline-functions])
      AX_APPEND_COMPILE_FLAGS([-fkeep-static-functions])
    ], [
      AS_IF([test "x$enable_release" != "xyes"], [
        # debug parameter
        AS_IF([test "x$with_debug_symbols" == "xyes"], [
          # debug symbols and sanitizer
          # -fsanitize=undefined
          AX_APPEND_COMPILE_FLAGS([-g -Og])
        ])
        # optimization level
        case "${with_optimization_level}" in
          no | 0)
            AX_APPEND_COMPILE_FLAGS([-O0])
            ;;
          1)
            AX_APPEND_COMPILE_FLAGS([-O1])
            ;;
          2)
            AX_APPEND_COMPILE_FLAGS([-O2])
            ;;
          3)
            AX_APPEND_COMPILE_FLAGS([-O3])
            ;;
          s)
            AX_APPEND_COMPILE_FLAGS([-Os])
            ;;
          g)
            AX_APPEND_COMPILE_FLAGS([-Og])
            ;;
          *)
            AS_IF([test "x$with_debug_symbols" != "xyes"], [
              AX_APPEND_COMPILE_FLAGS([-O2])
            ] )
            ;;
        esac
      ])
    ]
  )
])
