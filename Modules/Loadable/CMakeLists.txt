
project(SlicerModulesLoadable)

include(${Slicer_CMAKE_DIR}/SlicerMacroBuildQtModule.cmake)

#
# VTK
#
if(${VTK_VERSION_MAJOR} GREATER 5)
  set(${PROJECT_NAME}_VTK_COMPONENTS
    vtkFiltersExtraction
    vtkFiltersSources
    vtkIOImage
    vtkIOXML
    vtkInteractionWidgets
    vtkRenderingFreeTypeOpenGL
    vtkRenderingVolumeOpenGL
    )
  if(UNIX AND NOT APPLE)
    find_package(FontConfig QUIET)
    if (FONTCONFIG_FOUND)
      list(APPEND ${PROJECT_NAME}_VTK_COMPONENTS vtkRenderingFreeTypeFontConfig)
    endif()
  endif()
  if(BUILD_TESTING)
    list(APPEND ${CMAKE_PROJECT_NAME}_COMPONENTS
      vtkTestingRendering
      )
  endif()
  find_package(VTK COMPONENTS ${${PROJECT_NAME}_VTK_COMPONENTS} REQUIRED)
  include(${VTK_USE_FILE})
endif()

# --------------------------------------------------------------------------
set(qtmodules
  AtlasCreator
  Cameras
  Units
  Colors
  SubjectHierarchy
  Transforms
  Annotations
  Data
  DoubleArrays
  Markups
  #Measurements
  Models
  ModelMirror
  ModelTransform
  #FiberBundles
  SceneViews
  SlicerWelcome
  Reformat
  ViewControllers
  Volumes
  VolumeRendering # Volume Rendering depends of Volumes
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND qtmodules
    CropVolume
    )
endif()
if(Slicer_BUILD_DIFFUSION_SUPPORT)
  list(APPEND qtmodules
    TractographyDisplay
    TractographyInteractiveSeeding # tractography depends of Volumes
    )
endif()

foreach(module ${qtmodules})
  list(FIND Slicer_QTLOADABLEMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    add_subdirectory(${module})
  endif()
endforeach(module)

# External modules have been moved to Modules/Remote/CMakeLists.txt
