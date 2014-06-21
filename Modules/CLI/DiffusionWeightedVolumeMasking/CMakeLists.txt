
#-----------------------------------------------------------------------------
set(MODULE_NAME DiffusionWeightedVolumeMasking)

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
    vtkImagingMath
    vtkImagingMorphological
    )
  find_package(VTK 6.1 COMPONENTS ${${PROJECT_NAME}_VTK_COMPONENTS} REQUIRED)
else()
  find_package(VTK REQUIRED)
endif()
include(${VTK_USE_FILE})

#-----------------------------------------------------------------------------
set(${MODULE_NAME}_TARGET_LIBRARIES
  MRMLCore vtkTeem vtkSlicerEditorLibModuleLogic ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkSlicerEditorLibModuleLogic_SOURCE_DIR}
    ${vtkSlicerEditorLibModuleLogic_BINARY_DIR}
  TARGET_LIBRARIES ${${MODULE_NAME}_TARGET_LIBRARIES}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
