
foreach(p
  CMP0054 # CMake 3.1
  )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

if(NOT DEFINED MIDAS_API_DISPLAY_URL)
  set(MIDAS_API_DISPLAY_URL 0)
endif()

include(CMakeParseArguments)

#
# Download extension packages from the MIDAS server.
#
#   SERVER_URL The url of the MIDAS server
#   SLICER_ARCH
#   SLICER_OS
#   SLICER_REVISION
#   SLICER_EXTENSION_NAME
#
#   DOWNLOAD_URL_VARNAME Will set the value of ${DOWNLOAD_URL_VARNAME} to the URL
#                        allowing to download the extension package.
#
#   DOWNLOAD_URL_MD5_VARNAME Will set the value of ${DOWNLOAD_URL_MD5_VARNAME} to
#                            the package MD5.

function(midas_api_download_extension)
  set(expected_nonempty_args
    ARCHITECTURE
    EXTENSION_NAME
    OPERATING_SYSTEM
    SERVER_URL
    SLICER_REVISION
    DOWNLOAD_URL_VARNAME
    DOWNLOAD_URL_MD5_VARNAME
    PACKAGE_FILENAME_VARNAME
    )
  include(CMakeParseArguments)
  set(options)
  set(oneValueArgs ${expected_nonempty_args})
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity check
  foreach(var ${expected_nonempty_args})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

 include(MIDASAPILogin) # Required to include "midas_api_escape_for_url"

  #
  # Get extension metadata
  #
  midas_api_escape_for_url(productname "${MY_EXTENSION_NAME}")
  midas_api_escape_for_url(slicer_revision "${MY_SLICER_REVISION}")
  midas_api_escape_for_url(os "${MY_OPERATING_SYSTEM}")
  midas_api_escape_for_url(arch "${MY_ARCHITECTURE}")

  set(api_method "midas.slicerpackages.extension.list")
  set(params "${params}&slicer_revision=${slicer_revision}")
  set(params "${params}&os=${os}")
  set(params "${params}&arch=${arch}")
  set(params "${params}&productname=${productname}")
  set(params "${params}&codebase=Slicer4")
  set(url "${MY_SERVER_URL}/api/json?method=${api_method}${params}")

  if("${MIDAS_API_DISPLAY_URL}")
    message(STATUS "URL: ${url}")
  endif()

  set(response_filepath
    ${CMAKE_CURRENT_BINARY_DIR}/midas_api_response_filepath.txt)

  file(DOWNLOAD ${url} ${response_filepath} INACTIVITY_TIMEOUT 120)
  file(READ ${response_filepath} resp)

  if("${resp}" MATCHES ".*\"data\":\\[\\].*")
    message(FATAL_ERROR "Couldn't find extension \"${productname}\" for slicer_revision[${slicer_revision}] os[${os}] arch[${arch}]")
    return()
  endif()

  #message("resp:${resp}")

  foreach(property_name IN ITEMS
      item_id
      md5
      name
      package
      )
    # ".*${property_name}\":[ ]*\"([0-9a-zA-Z]*)\".*"
    string(REGEX REPLACE ".*\"${property_name}\":[ ]*\"([0-9a-zA-Z\\.\\-]*)\".*" "\\1" ${property_name} "${resp}")
    if(NOT "${${property_name}}" MATCHES "^[0-9a-zA-Z\\.\\-]+$")
      message(FATAL_ERROR "Failed to extract property \"${property_name}\" associated with repsonse of GET ${url}")
    endif()
  endforeach()

  #message("item_id:${item_id}")
  #message("md5:${md5}")
  #message("name:${name}")
  #message("package:${package}")

  #
  # Set extension package download URL
  #

  set(download_url "${MY_SERVER_URL}/download?items=${item_id}")

  #
  # Return
  #

  set(${MY_DOWNLOAD_URL_VARNAME} ${download_url} PARENT_SCOPE)
  set(${MY_DOWNLOAD_URL_MD5_VARNAME} ${md5} PARENT_SCOPE)
  set(${MY_PACKAGE_FILENAME_VARNAME} ${name} PARENT_SCOPE)

endfunction()


#
# Testing - cmake -DTEST_<TESTNAME>:BOOL=ON -P MIDASAPIDownloadExtension.cmake
#

#
# TESTNAME: midas_api_download_extension_test
#
if(TEST_midas_api_download_extension_test)

  function(midas_api_download_extension_test)

    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../CMake ${CMAKE_MODULE_PATH})

    set(server_url "http://slicer.kitware.com/midas3")
    set(slicer_revision "25927")
    set(extension_name "ShapePopulationViewer")
    set(operating_system "macosx")
    set(architecture "amd64")
    set(download_dir "/tmp")


    midas_api_download_extension(
      SERVER_URL ${server_url}
      SLICER_REVISION ${slicer_revision}
      EXTENSION_NAME ${extension_name}
      OPERATING_SYSTEM ${operating_system}
      ARCHITECTURE ${architecture}
      DOWNLOAD_URL_VARNAME download_url
      DOWNLOAD_URL_MD5_VARNAME download_url_md5
      PACKAGE_FILENAME_VARNAME package_filename
      )

    if("${download_url}" STREQUAL "")
      message(FATAL_ERROR "Problem with midas_api_download_extension()\n"
                          "DOWNLOAD_URL_VARNAME:${download_url} should not be empty")
    endif()

    if("${download_url_md5}" STREQUAL "")
      message(FATAL_ERROR "Problem with midas_api_download_extension()\n"
                          "DOWNLOAD_URL_MD5_VARNAME:${download_url_md5} should not be empty")
    endif()

    if("${package_filename}" STREQUAL "")
      message(FATAL_ERROR "Problem with midas_api_download_extension()\n"
                          "PACKAGE_FILENAME_VARNAME:${package_filename} should not be empty")
    endif()

    message("SUCCESS")
  endfunction()
  midas_api_download_extension_test()

endif()
