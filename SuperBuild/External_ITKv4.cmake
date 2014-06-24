
set(proj ITKv4)

# Set dependency list
set(${proj}_DEPENDENCIES "zlib")
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND ${proj}_DEPENDENCIES DCMTK)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(ITK_DIR CACHE)
  find_package(ITK 4.6 REQUIRED NO_MODULE)
endif()

# Sanity checks
if(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  message(FATAL_ERROR "ITK_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ITK_DIR AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  if(NOT DEFINED git_protocol)
      set(git_protocol "git")
  endif()

  set(ITKv4_REPOSITORY ${git_protocol}://github.com/Slicer/ITK.git)
  set(ITKv4_GIT_TAG 56fae278ad0ae805da4f4dbea5a4b9979cf4262c) # slicer-v4.6.0-2014-09-19-554e46a

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)

  if(NOT Slicer_ITKV3_COMPATIBILITY AND CMAKE_CL_64)
    # enables using long long type for indexes and size on platforms
    # where long is only 32-bits (msvc)
    set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DITK_USE_64BITS_IDS:BOOL=ON
      )
  endif()

  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
      )
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY ${ITKv4_REPOSITORY}
    GIT_TAG ${ITKv4_GIT_TAG}
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    INSTALL_DIR ${proj}-install
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/${proj}-install
      -DITK_INSTALL_ARCHIVE_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DITK_INSTALL_LIBRARY_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DITK_LEGACY_REMOVE:BOOL=OFF
      -DITKV3_COMPATIBILITY:BOOL=${Slicer_ITKV3_COMPATIBILITY}
      -DITK_BUILD_DEFAULT_MODULES:BOOL=ON
      -DModule_ITKReview:BOOL=ON
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DITK_INSTALL_NO_DEVELOPMENT:BOOL=OFF
      -DKWSYS_USE_MD5:BOOL=ON # Required by SlicerExecutionModel
      -DITK_WRAPPING:BOOL=OFF #${BUILD_SHARED_LIBS} ## HACK:  QUICK CHANGE
      # DCMTK
      -DITK_USE_SYSTEM_DCMTK:BOOL=ON
      -DDCMTK_DIR:PATH=${DCMTK_DIR}
      -DModule_ITKIODCMTK:BOOL=${Slicer_BUILD_DICOM_SUPPORT}
      # ZLIB
      -DITK_USE_SYSTEM_ZLIB:BOOL=ON
      -DZLIB_ROOT:PATH=${ZLIB_ROOT}
      -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(ITK_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(_lib_subdir lib)
  if(WIN32)
    set(_lib_subdir bin)
  endif()

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${ITK_DIR}/${_lib_subdir}/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS ITK_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
