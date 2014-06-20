
#-----------------------------------------------------------------------------
set(MODULE_NAME MergeModels)

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#
# VTK
#
if(${VTK_VERSION_MAJOR} GREATER 5)
  set(${PROJECT_NAME}_VTK_COMPONENTS
    vtkCommonCore
    vtkFiltersCore
    vtkIOLegacy
    vtkIOXML
    )
  find_package(VTK 6.1 COMPONENTS ${${PROJECT_NAME}_VTK_COMPONENTS} REQUIRED)
  include(${VTK_USE_FILE})
else()
  set(VTK_LIBRARIES
    vtkIO
    vtkGraphics
    )
endif()

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${VTK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
