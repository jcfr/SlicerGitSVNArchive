
#-----------------------------------------------------------------------------
set(MODULE_NAME TractographyLabelMapSeeding)

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
    vtkIOLegacy
    vtkIOXML
    vtkImagingCore
    )
  find_package(VTK 6.1 COMPONENTS ${${PROJECT_NAME}_VTK_COMPONENTS} REQUIRED)
  include(${VTK_USE_FILE})
endif()

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES vtkTeem SlicerBaseLogic ${VTK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
