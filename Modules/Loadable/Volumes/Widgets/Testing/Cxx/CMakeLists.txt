set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(INPUT "${MRMLCore_SOURCE_DIR}/Testing/TestData")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qMRMLVolumeInfoWidgetTest1.cxx
  qSlicerDiffusionTensorVolumeDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest2.cxx
  )

if(Slicer_USE_QtTesting)
  list(APPEND KIT_TEST_SRCS
    qMRMLVolumeInfoWidgetEventTranslatorPlayerTest1.cxx
    )
endif()

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkSlicerVolumesModuleLogic
  INCLUDE_DIRECTORIES
    ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
    ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qMRMLVolumeInfoWidgetTest1)
simple_test(qSlicerDiffusionTensorVolumeDisplayWidgetTest1)
simple_test(qSlicerDTISliceDisplayWidgetTest1)
simple_test(qSlicerDTISliceDisplayWidgetTest2 ${INPUT}/helix-DTI.nhdr)

if(Slicer_USE_QtTesting)
  macro(QtTesting_TEST testname)
    simple_test(${testname} ${ARGN})
    set_tests_properties(${testname} PROPERTIES RUN_SERIAL ON)
  endmacro()
  QtTesting_TEST(qMRMLVolumeInfoWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
endif()
