set(proj SlicerExtensionPackages)

if("${Slicer_EXTENSION_PACKAGE_NAMES}" STREQUAL "")
  return()
endif()

if(NOT DEFINED Slicer_EXTENSION_PACKAGES_SLICER_REVISION)
  set(Slicer_EXTENSION_PACKAGES_SLICER_REVISION ${Slicer_WC_REVISION})
endif()

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()


include(${Slicer_EXTENSIONS_CMAKE_DIR}/MIDASAPIDownloadExtension.cmake)

ExternalProject_Message(${proj} "  Slicer_OS:${Slicer_OS}")
ExternalProject_Message(${proj} "  Slicer_ARCHITECTURE:${Slicer_ARCHITECTURE}")
ExternalProject_Message(${proj} "  Slicer_EXTENSION_PACKAGES_SLICER_REVISION:${Slicer_EXTENSION_PACKAGES_SLICER_REVISION}")

set(sub_project_names)

set(Slicer_EXTENSION_INSTALL_DIRS )

foreach(extension_name ${Slicer_EXTENSION_PACKAGE_NAMES})

  ExternalProject_Message(${proj} "    ${extension_name}")

  midas_api_download_extension(
    SERVER_URL "http://slicer.kitware.com/midas3"
    SLICER_REVISION ${Slicer_EXTENSION_PACKAGES_SLICER_REVISION}
    EXTENSION_NAME ${extension_name}
    OPERATING_SYSTEM ${Slicer_OS}
    ARCHITECTURE ${Slicer_ARCHITECTURE}
    DOWNLOAD_URL_VARNAME download_url
    DOWNLOAD_URL_MD5_VARNAME download_url_md5
    PACKAGE_FILENAME_VARNAME package_filename
    )

  set(download_url "${download_url}&filename=${package_filename}")

  ExternalProject_Message(${proj} "    url:${download_url}")
  ExternalProject_Message(${proj} "    url_md5:${download_url_md5}")
  ExternalProject_Message(${proj} "    filename:${package_filename}")
  ExternalProject_Message(${proj} "")

  set(install_dir ${CMAKE_BINARY_DIR}/${proj}-${extension_name})

  if(APPLE)
    set(_dirname "${Slicer_EXTENSIONS_DIRBASENAME}-${Slicer_EXTENSION_PACKAGES_SLICER_REVISION}")
    set(_root "${Slicer_BUNDLE_LOCATION}/${_dirname}/${extension_name}")
    set(install_dir "${install_dir}/${_root}")
  endif()

  ExternalProject_Add(${proj}-${extension_name}
    URL ${download_url}
    URL_MD5 ${download_url_md5}
    SOURCE_DIR ${install_dir}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )

  list(APPEND sub_project_names ${proj}-${extension_name})
  list(APPEND Slicer_EXTENSION_INSTALL_DIRS ${install_dir})

endforeach()

#
# This is the main project depending on each subproject responsible
# to download extension packages.
#
ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${${proj}_DEPENDENCIES} ${sub_project_names}
  )

mark_as_superbuild(Slicer_EXTENSION_INSTALL_DIRS:STRING)
