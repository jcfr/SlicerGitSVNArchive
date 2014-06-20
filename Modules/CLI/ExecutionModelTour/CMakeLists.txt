
#-----------------------------------------------------------------------------
set(MODULE_NAME ExecutionModelTour)

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
    )
  find_package(VTK 6.1 COMPONENTS ${${PROJECT_NAME}_VTK_COMPONENTS} REQUIRED)
  include(${VTK_USE_FILE})
endif()

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/BillsLogo.h
  TARGET_LIBRARIES
    # ${VTK_LIBRARIES} # Not needed: All dependencies are transitively satisfied by other targets
    MRMLCore
  INCLUDE_DIRECTORIES
    ${MRMLCore_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
