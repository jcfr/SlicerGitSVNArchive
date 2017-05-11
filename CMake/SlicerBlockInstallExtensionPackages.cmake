
# Install extension install directories
foreach(dir IN LISTS Slicer_EXTENSION_INSTALL_DIRS)
  if(NOT EXISTS ${dir})
    message(WARNING "Skipping nonexistent extension install directory [${dir}]")
    continue()
  endif()
  install(
    DIRECTORY "${dir}/"
    DESTINATION ${Slicer_INSTALL_ROOT}
    USE_SOURCE_PERMISSIONS
    COMPONENT Runtime
    )
endforeach()
