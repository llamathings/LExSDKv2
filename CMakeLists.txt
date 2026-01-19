cmake_minimum_required (VERSION 3.27)
project (LESDK LANGUAGES CXX)


# ! Global configuration.
# ========================================

cmake_policy (SET CMP0057 NEW)
cmake_policy (SET CMP0077 NEW)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)


# ! Build configuration.
# ========================================

option (LESDK_WITH_LE1 "Create LE1 SDK project." ON)
option (LESDK_WITH_LE2 "Create LE2 SDK project." ON)
option (LESDK_WITH_LE3 "Create LE3 SDK project." ON)
option (LESDK_WITH_SAMPLES "Create sample projects." ${PROJECT_IS_TOP_LEVEL})
option (LESDK_WITH_TESTS "Create test projects." ${PROJECT_IS_TOP_LEVEL})


# ! Shared project configuration.
# ========================================

set (PROJ_NAME LESDK)
set (PROJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set (SRCS_ROOT ${PROJ_ROOT}/Src/${PROJ_NAME})


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  set (PROJ_DEFS_SHARED
    "_CRT_SECURE_NO_WARNINGS"
    "NOMINMAX"
  )

  set (PROJ_OPTS_SHARED
    "/bigobj" "/MP" "/permissive" "/utf-8" "/W4" "/Zc:__cplusplus" "/Zi"
    # Disable warnings below...
    "/wd4121"   # alignment of a member was sensitive to packing
    "/wd26495"  # variable is uninitialized
  )

  if (PROJECT_IS_TOP_LEVEL)
    add_compile_options (
      "$<$<IN_LIST:$<CONFIG>,DEBUG>:/MDd;/Od>"
      "$<$<IN_LIST:$<CONFIG>,RELEASE>:/MD;/O2>"
    )
    add_link_options (
      "/DEBUG"
      "$<$<IN_LIST:$<CONFIG>,DEBUG>:/OPT:NOREF;/OPT:NOICF>"
      "$<$<IN_LIST:$<CONFIG>,RELEASE>:/OPT:ICF;/INCREMENTAL:NO>"
    )
  endif ()

else ()

  # TODO: Consider supporting Clang-CL.
  message(FATAL_ERROR "Only Visual Studio compiler is supported.")

endif ()


set (PROJ_INCL_SHARED
  # Shared include directories.
  ${CMAKE_BINARY_DIR}
  ${PROJ_ROOT}/Src/
  ${PROJ_ROOT}/External/
)

set (PROJ_LINK_SHARED
  # Add shared link targets...
)

set (PROJ_SRCS_SHARED
  ${SRCS_ROOT}/LESDK.natvis

  ${SRCS_ROOT}/Common/Core.hpp
  ${SRCS_ROOT}/Common/Frame.hpp
  ${SRCS_ROOT}/Common/FString.hpp
  ${SRCS_ROOT}/Common/Misc.hpp
  ${SRCS_ROOT}/Common/SFXName.hpp
  ${SRCS_ROOT}/Common/TArray.hpp
  ${SRCS_ROOT}/Common/TMap.hpp
  ${SRCS_ROOT}/Common/Math.hpp

  ${SRCS_ROOT}/Common/Common.cpp
  ${SRCS_ROOT}/Common/Common.hpp

  ${SRCS_ROOT}/Headers.hpp
  ${SRCS_ROOT}/Init.cpp
  ${SRCS_ROOT}/Init.hpp
)


# ! Binary output structure.
# ========================================

if (PROJECT_IS_TOP_LEVEL)

  if (CMAKE_GENERATOR MATCHES "Visual Studio 17 2022")
    set (LESDK_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/$<CONFIG>/)
  else ()
    message (FATAL_ERROR "!! Unsupported generator detected: ${CMAKE_GENERATOR}")
  endif ()

  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LESDK_OUTPUT_DIRECTORY})
  set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LESDK_OUTPUT_DIRECTORY})
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LESDK_OUTPUT_DIRECTORY})

endif ()


# ! LESDK project for LE1.
# ========================================

if (LESDK_WITH_LE1)

  file (GLOB SRCS_GENERATED_LE1 LIST_DIRECTORIES false
    ${SRCS_ROOT}/LE1/*.cpp
    ${SRCS_ROOT}/LE1/*.hpp
  )

  list (FILTER SRCS_GENERATED_LE1 EXCLUDE REGEX "PlotManagerDLC_")

  set (PROJ_SRCS_LE1
    ${SRCS_GENERATED_LE1}
    ${SRCS_ROOT}/Includes.LE1.hpp
  )


  add_library (${PROJ_NAME}-LE1 STATIC ${PROJ_SRCS_SHARED} ${PROJ_SRCS_LE1})
  source_group (TREE ${SRCS_ROOT} FILES ${PROJ_SRCS_SHARED} ${PROJ_SRCS_LE1})

  target_compile_definitions (${PROJ_NAME}-LE1 PRIVATE ${PROJ_DEFS_SHARED} PUBLIC "SDK_TARGET_LE1")
  target_compile_options (${PROJ_NAME}-LE1 PRIVATE ${PROJ_OPTS_SHARED})
  target_include_directories (${PROJ_NAME}-LE1 PUBLIC ${PROJ_INCL_SHARED})
  target_link_libraries (${PROJ_NAME}-LE1 PUBLIC ${PROJ_LINK_SHARED})
  target_precompile_headers (${PROJ_NAME}-LE1 PUBLIC ${SRCS_ROOT}/_Global.pch.hpp)

endif ()


# ! LESDK project for LE2.
# ========================================

if (LESDK_WITH_LE2)

  file (GLOB SRCS_GENERATED_LE2 LIST_DIRECTORIES false
    ${SRCS_ROOT}/LE2/*.cpp
    ${SRCS_ROOT}/LE2/*.hpp
  )

  list (FILTER SRCS_GENERATED_LE2 EXCLUDE REGEX "PlotManagerDLC_")

  set (PROJ_SRCS_LE2
    ${SRCS_GENERATED_LE2}
    ${SRCS_ROOT}/Includes.LE2.hpp
  )


  add_library (${PROJ_NAME}-LE2 STATIC ${PROJ_SRCS_SHARED} ${PROJ_SRCS_LE2})
  source_group (TREE ${SRCS_ROOT} FILES ${PROJ_SRCS_SHARED} ${PROJ_SRCS_LE2})

  target_compile_definitions (${PROJ_NAME}-LE2 PRIVATE ${PROJ_DEFS_SHARED} PUBLIC "SDK_TARGET_LE2")
  target_compile_options (${PROJ_NAME}-LE2 PRIVATE ${PROJ_OPTS_SHARED})
  target_include_directories (${PROJ_NAME}-LE2 PUBLIC ${PROJ_INCL_SHARED})
  target_link_libraries (${PROJ_NAME}-LE2 PUBLIC ${PROJ_LINK_SHARED})
  target_precompile_headers (${PROJ_NAME}-LE2 PUBLIC ${SRCS_ROOT}/_Global.pch.hpp)

endif ()


# ! LESDK project for LE3.
# ========================================

if (LESDK_WITH_LE3)

  file (GLOB SRCS_GENERATED_LE3 LIST_DIRECTORIES false
    ${SRCS_ROOT}/LE3/*.cpp
    ${SRCS_ROOT}/LE3/*.hpp
  )

  set (PROJ_SRCS_LE3
    ${SRCS_GENERATED_LE3}
    ${SRCS_ROOT}/Includes.LE3.hpp
  )


  add_library (${PROJ_NAME}-LE3 STATIC ${PROJ_SRCS_SHARED} ${PROJ_SRCS_LE3})
  source_group (TREE ${SRCS_ROOT} FILES ${PROJ_SRCS_SHARED} ${PROJ_SRCS_LE3})

  target_compile_definitions (${PROJ_NAME}-LE3 PRIVATE ${PROJ_DEFS_SHARED} PUBLIC "SDK_TARGET_LE3")
  target_compile_options (${PROJ_NAME}-LE3 PRIVATE ${PROJ_OPTS_SHARED})
  target_include_directories (${PROJ_NAME}-LE3 PUBLIC ${PROJ_INCL_SHARED})
  target_link_libraries (${PROJ_NAME}-LE3 PUBLIC ${PROJ_LINK_SHARED})
  target_precompile_headers (${PROJ_NAME}-LE3 PUBLIC ${SRCS_ROOT}/_Global.pch.hpp)

endif ()


# ! Tests project.
# ========================================

if (LESDK_WITH_TESTS)

  set (SRCS_ROOT_TESTS ${PROJ_ROOT}/Tests)

  set (PROJ_SRCS_TESTS
    ${SRCS_ROOT_TESTS}/Entry.cpp
    ${SRCS_ROOT_TESTS}/Utilities.hpp

    ${SRCS_ROOT_TESTS}/Tests.FString.hpp
    ${SRCS_ROOT_TESTS}/Tests.TArray.hpp
    ${SRCS_ROOT_TESTS}/Tests.TMap.hpp
  )

  add_executable (${PROJ_NAME}-TESTS ${PROJ_SRCS_TESTS} ${SRCS_ROOT}/LESDK.natvis)
  source_group (TREE ${SRCS_ROOT_TESTS} FILES ${PROJ_SRCS_TESTS})

  target_compile_definitions (${PROJ_NAME}-TESTS PRIVATE ${PROJ_DEFS_SHARED})
  target_compile_options (${PROJ_NAME}-TESTS PRIVATE ${PROJ_OPTS_SHARED})
  target_include_directories (${PROJ_NAME}-TESTS PRIVATE ${PROJ_INCL_SHARED})
  target_link_libraries (${PROJ_NAME}-TESTS PRIVATE ${PROJ_LINK_SHARED})
  target_precompile_headers (${PROJ_NAME}-TESTS PRIVATE ${SRCS_ROOT}/_Global.pch.hpp)

  # We only test containers, we don't really care about SDK target.
  # Include any that is available.

  if (LESDK_WITH_LE1)
    target_compile_definitions (${PROJ_NAME}-TESTS PRIVATE "SDK_TARGET_LE1")
    target_link_libraries (LESDK-TESTS PRIVATE LESDK-LE1)
  elseif (LESDK_WITH_LE2)
    target_compile_definitions (${PROJ_NAME}-TESTS PRIVATE "SDK_TARGET_LE2")
    target_link_libraries (LESDK-TESTS PRIVATE LESDK-LE2)
  elseif (LESDK_WITH_LE3)
    target_compile_definitions (${PROJ_NAME}-TESTS PRIVATE "SDK_TARGET_LE3")
    target_link_libraries (LESDK-TESTS PRIVATE LESDK-LE3)
  endif ()

endif ()


# ! Fake project for repository files.
# ========================================

if (PROJECT_IS_TOP_LEVEL)

  add_custom_target (Repository SOURCES
    .editorconfig
    .gitattributes
    .gitignore
    CMakeLists.txt
    CMakePresets.json
  )

endif ()
