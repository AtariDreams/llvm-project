#
#//===----------------------------------------------------------------------===//
#//
#// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
#// See https://llvm.org/LICENSE.txt for license information.
#// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#//
#//===----------------------------------------------------------------------===//
#

# Setup the flags correctly for cmake (covert to string)
# Pretty them up (STRIP any beginning and trailing whitespace,
# remove duplicates, remove empty entries)
macro(libomp_setup_flags flags)
  if(NOT "${${flags}}" STREQUAL "") # if flags are empty, don't do anything
    set(flags_local)
    list(REMOVE_DUPLICATES ${flags}) # remove duplicates
    list(REMOVE_ITEM ${flags} "") # remove empty items
    libomp_list_to_string("${${flags}}" flags_local)
    string(STRIP "${flags_local}" flags_local)
    set(${flags} "${flags_local}")
  endif()
endmacro()

# Gets flags common to both the C and C++ compiler
function(libomp_get_c_and_cxxflags_common flags)
  set(flags_local)
  libomp_append(flags_local -fno-exceptions LIBOMP_HAVE_FNO_EXCEPTIONS_FLAG)
  libomp_append(flags_local -fno-rtti LIBOMP_HAVE_FNO_RTTI_FLAG)
  if(${OPENMP_STANDALONE_BUILD})
    libomp_append(flags_local -Wsign-compare LIBOMP_HAVE_WNO_SIGN_COMPARE_FLAG)
    libomp_append(flags_local -Wunused-function LIBOMP_HAVE_WNO_UNUSED_FUNCTION_FLAG)
    libomp_append(flags_local -Wunused-local-typedef LIBOMP_HAVE_WNO_UNUSED_LOCAL_TYPEDEF_FLAG)
    libomp_append(flags_local -Wunused-value LIBOMP_HAVE_WNO_UNUSED_VALUE_FLAG)
    libomp_append(flags_local -Wunused-variable LIBOMP_HAVE_WNO_UNUSED_VARIABLE_FLAG)
    libomp_append(flags_local -Wdeprecated-register LIBOMP_HAVE_WNO_DEPRECATED_REGISTER_FLAG)
    libomp_append(flags_local -Wunknown-pragmas LIBOMP_HAVE_WNO_UNKNOWN_PRAGMAS_FLAG)
    libomp_append(flags_local -Wcomment LIBOMP_HAVE_WNO_COMMENT_FLAG)
    libomp_append(flags_local -Wself-assign LIBOMP_HAVE_WNO_SELF_ASSIGN_FLAG)
    libomp_append(flags_local -Wformat-pedantic LIBOMP_HAVE_WNO_FORMAT_PEDANTIC_FLAG)
  endif()
  libomp_append(flags_local -Wno-switch LIBOMP_HAVE_WNO_SWITCH_FLAG)
  libomp_append(flags_local -Wno-covered-switch-default LIBOMP_HAVE_WNO_COVERED_SWITCH_DEFAULT_FLAG)
  libomp_append(flags_local -Wno-gnu-anonymous-struct LIBOMP_HAVE_WNO_GNU_ANONYMOUS_STRUCT_FLAG)
  libomp_append(flags_local -Wno-missing-field-initializers LIBOMP_HAVE_WNO_MISSING_FIELD_INITIALIZERS_FLAG)
  libomp_append(flags_local -Wno-missing-braces LIBOMP_HAVE_WNO_MISSING_BRACES_FLAG)
  libomp_append(flags_local -Wno-vla-extension LIBOMP_HAVE_WNO_VLA_EXTENSION_FLAG)
  libomp_append(flags_local -Wstringop-overflow=0 LIBOMP_HAVE_WSTRINGOP_OVERFLOW_FLAG)
  libomp_append(flags_local /GS LIBOMP_HAVE_GS_FLAG)
  libomp_append(flags_local /EHsc LIBOMP_HAVE_EHSC_FLAG)
  libomp_append(flags_local /Oy- LIBOMP_HAVE_OY__FLAG)
  libomp_append(flags_local -mrtm LIBOMP_HAVE_MRTM_FLAG)
  # Intel(R) C Compiler flags
  libomp_append(flags_local /Qsafeseh LIBOMP_HAVE_QSAFESEH_FLAG)
  libomp_append(flags_local -Qoption,cpp,--extended_float_types LIBOMP_HAVE_EXTENDED_FLOAT_TYPES_FLAG)
  libomp_append(flags_local -Qlong_double LIBOMP_HAVE_LONG_DOUBLE_FLAG)
  libomp_append(flags_local -Qdiag-disable:177 LIBOMP_HAVE_DIAG_DISABLE_177_FLAG)
  if(${RELEASE_BUILD} OR ${RELWITHDEBINFO_BUILD})
    libomp_append(flags_local -Qinline-min-size=1 LIBOMP_HAVE_INLINE_MIN_SIZE_FLAG)
  endif()
  # Architectural C and C++ flags
  if(${IA32})
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      libomp_append(flags_local -m32 LIBOMP_HAVE_M32_FLAG)
    endif()
    libomp_append(flags_local /arch:SSE2 LIBOMP_HAVE_ARCH_SSE2_FLAG)
    libomp_append(flags_local -msse2 LIBOMP_HAVE_MSSE2_FLAG)
    libomp_append(flags_local -falign-stack=maintain-16-byte LIBOMP_HAVE_FALIGN_STACK_FLAG)
  elseif(${MIC})
    libomp_append(flags_local -mmic LIBOMP_HAVE_MMIC_FLAG)
    libomp_append(flags_local -ftls-model=initial-exec LIBOMP_HAVE_FTLS_MODEL_FLAG)
    libomp_append(flags_local "-opt-streaming-stores never" LIBOMP_HAVE_OPT_STREAMING_STORES_FLAG)
  endif()
  set(${flags} ${flags_local} PARENT_SCOPE)
endfunction()

# C compiler flags
function(libomp_get_cflags cflags)
  set(cflags_local)
  libomp_get_c_and_cxxflags_common(cflags_local)
  # flags only for the C Compiler
  libomp_append(cflags_local /TP LIBOMP_HAVE_TP_FLAG)
  libomp_append(cflags_local "-x c++" LIBOMP_HAVE_X_CPP_FLAG)
  set(cflags_local ${cflags_local} ${LIBOMP_CFLAGS})
  libomp_setup_flags(cflags_local)
  set(${cflags} ${cflags_local} PARENT_SCOPE)
endfunction()

# C++ compiler flags
function(libomp_get_cxxflags cxxflags)
  set(cxxflags_local)
  libomp_get_c_and_cxxflags_common(cxxflags_local)
  if(${OPENMP_STANDALONE_BUILD})
      libomp_append(cxxflags_local -Wcast-qual LIBOMP_HAVE_WCAST_QUAL_FLAG)
  endif()
  set(cxxflags_local ${cxxflags_local} ${LIBOMP_CXXFLAGS})
  libomp_setup_flags(cxxflags_local)
  set(${cxxflags} ${cxxflags_local} PARENT_SCOPE)
endfunction()

# Assembler flags
function(libomp_get_asmflags asmflags)
  set(asmflags_local)
  libomp_append(asmflags_local "-x assembler-with-cpp" LIBOMP_HAVE_X_ASSEMBLER_WITH_CPP_FLAG)
  # Architectural assembler flags
  if(${IA32})
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      libomp_append(asmflags_local -m32 LIBOMP_HAVE_M32_FLAG)
    endif()
    libomp_append(asmflags_local /safeseh LIBOMP_HAVE_SAFESEH_MASM_FLAG)
    libomp_append(asmflags_local /coff LIBOMP_HAVE_COFF_MASM_FLAG)
  elseif(${MIC})
    libomp_append(asmflags_local -mmic LIBOMP_HAVE_MMIC_FLAG)
  endif()
  set(asmflags_local ${asmflags_local} ${LIBOMP_ASMFLAGS})
  libomp_setup_flags(asmflags_local)
  set(${asmflags} ${asmflags_local} PARENT_SCOPE)
endfunction()

# Linker flags
function(libomp_get_ldflags ldflags)
  set(ldflags_local)
  libomp_append(ldflags_local "${CMAKE_LINK_DEF_FILE_FLAG}${CMAKE_CURRENT_BINARY_DIR}/${LIBOMP_LIB_NAME}.def"
    IF_DEFINED CMAKE_LINK_DEF_FILE_FLAG)
  libomp_append(ldflags_local "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}${LIBOMP_VERSION_MAJOR}.${LIBOMP_VERSION_MINOR}"
    IF_DEFINED CMAKE_C_OSX_CURRENT_VERSION_FLAG)
  libomp_append(ldflags_local "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}${LIBOMP_VERSION_MAJOR}.${LIBOMP_VERSION_MINOR}"
    IF_DEFINED CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG)
  libomp_append(ldflags_local -Wl,--warn-shared-textrel LIBOMP_HAVE_WARN_SHARED_TEXTREL_FLAG)
  libomp_append(ldflags_local -Wl,--as-needed LIBOMP_HAVE_AS_NEEDED_FLAG)
  libomp_append(ldflags_local "-Wl,--version-script=${LIBOMP_SRC_DIR}/exports_so.txt" LIBOMP_HAVE_VERSION_SCRIPT_FLAG)
  libomp_append(ldflags_local -static-libgcc LIBOMP_HAVE_STATIC_LIBGCC_FLAG)
  libomp_append(ldflags_local -Wl,-z,noexecstack LIBOMP_HAVE_Z_NOEXECSTACK_FLAG)
  libomp_append(ldflags_local -Wl,-fini=__kmp_internal_end_fini LIBOMP_HAVE_FINI_FLAG)
  libomp_append(ldflags_local -no-intel-extensions LIBOMP_HAVE_NO_INTEL_EXTENSIONS_FLAG)
  libomp_append(ldflags_local -static-intel LIBOMP_HAVE_STATIC_INTEL_FLAG)
  libomp_append(ldflags_local /SAFESEH LIBOMP_HAVE_SAFESEH_FLAG)
  # Architectural linker flags
  if(${IA32})
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      libomp_append(ldflags_local -m32 LIBOMP_HAVE_M32_FLAG)
    endif()
    libomp_append(ldflags_local -msse2 LIBOMP_HAVE_MSSE2_FLAG)
  elseif(${MIC})
    libomp_append(ldflags_local -mmic LIBOMP_HAVE_MMIC_FLAG)
    libomp_append(ldflags_local -Wl,-x LIBOMP_HAVE_X_FLAG)
  endif()
  set(ldflags_local ${ldflags_local} ${LIBOMP_LDFLAGS})
  libomp_setup_flags(ldflags_local)
  set(${ldflags} ${ldflags_local} PARENT_SCOPE)
endfunction()

# Library flags
function(libomp_get_libflags libflags)
  set(libflags_local)
  libomp_append(libflags_local "${CMAKE_THREAD_LIBS_INIT}")
  libomp_append(libflags_local "${LIBOMP_HWLOC_LIBRARY}" LIBOMP_USE_HWLOC)
  if(${IA32})
    libomp_append(libflags_local -lirc_pic LIBOMP_HAVE_IRC_PIC_LIBRARY)
  endif()
  IF(${CMAKE_SYSTEM_NAME} MATCHES "DragonFly")
    libomp_append(libflags_local "-Wl,--no-as-needed" LIBOMP_HAVE_AS_NEEDED_FLAG)
    libomp_append(libflags_local "-lm")
    libomp_append(libflags_local "-Wl,--as-needed" LIBOMP_HAVE_AS_NEEDED_FLAG)
  ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "DragonFly")
  IF(${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
    libomp_append(libflags_local -lm)
  ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
  set(libflags_local ${libflags_local} ${LIBOMP_LIBFLAGS})
  libomp_setup_flags(libflags_local)
  set(${libflags} ${libflags_local} PARENT_SCOPE)
endfunction()

# Fortran flags
function(libomp_get_fflags fflags)
  set(fflags_local)
  if(${IA32})
    libomp_append(fflags_local -m32 LIBOMP_HAVE_M32_FORTRAN_FLAG)
  endif()
  set(fflags_local ${fflags_local} ${LIBOMP_FFLAGS})
  libomp_setup_flags(fflags_local)
  set(${fflags} ${fflags_local} PARENT_SCOPE)
endfunction()

# Perl generate-defs.pl flags (For Windows only)
function(libomp_get_gdflags gdflags)
  set(gdflags_local)
  if(${IA32})
    set(libomp_gdflag_arch arch_32)
  elseif(${INTEL64})
    set(libomp_gdflag_arch arch_32e)
  else()
    set(libomp_gdflag_arch arch_${LIBOMP_ARCH})
  endif()
  libomp_append(gdflags_local "-D ${libomp_gdflag_arch}")
  libomp_append(gdflags_local "-D msvc_compat")
  libomp_append(gdflags_local "-D norm" NORMAL_LIBRARY)
  libomp_append(gdflags_local "-D prof" PROFILE_LIBRARY)
  libomp_append(gdflags_local "-D stub" STUBS_LIBRARY)
  libomp_append(gdflags_local "-D HAVE_QUAD" LIBOMP_USE_QUAD_PRECISION)
  libomp_append(gdflags_local "-D USE_DEBUGGER" LIBOMP_USE_DEBUGGER)
  if(${LIBOMP_OMP_VERSION} GREATER 50 OR ${LIBOMP_OMP_VERSION} EQUAL 50)
    libomp_append(gdflags_local "-D OMP_50")
  endif()
  if(${LIBOMP_OMP_VERSION} GREATER 45 OR ${LIBOMP_OMP_VERSION} EQUAL 45)
    libomp_append(gdflags_local "-D OMP_45")
  endif()
  if(${LIBOMP_OMP_VERSION} GREATER 40 OR ${LIBOMP_OMP_VERSION} EQUAL 40)
    libomp_append(gdflags_local "-D OMP_40")
  endif()
  if(${LIBOMP_OMP_VERSION} GREATER 30 OR ${LIBOMP_OMP_VERSION} EQUAL 30)
    libomp_append(gdflags_local "-D OMP_30")
  endif()
  if(${DEBUG_BUILD} OR ${RELWITHDEBINFO_BUILD})
    libomp_append(gdflags_local "-D KMP_DEBUG")
  endif()
  set(${gdflags} ${gdflags_local} PARENT_SCOPE)
endfunction()
