import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# ModelMakerBreaksSliceViewsIssue3462
#

class ModelMakerBreaksSliceViewsIssue3462(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "ModelMakerBreaksSliceViewsIssue3462" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = []
    self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)"] # replace with "Firstname Lastname (Org)"
    self.parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    It performs a simple thresholding on the input volume and optionally captures a screenshot.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc. and Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.

#
# qModelMakerBreaksSliceViewsIssue3462Widget
#

class ModelMakerBreaksSliceViewsIssue3462Widget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

    pass

class ModelMakerBreaksSliceViewsIssue3462Test(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_ModelMakerBreaksSliceViewsIssue3462_1()
    self.test_ModelMakerBreaksSliceViewsIssue3462_2()
    self.test_ModelMakerBreaksSliceViewsIssue3462_3()
    self.test_ModelMakerBreaksSliceViewsIssue3462_4()

  def test_ModelMakerBreaksSliceViewsIssue3462_1(self):
    self.delayDisplay("Starting the test #1")

    self.delayDisplay("Adding vtkMRMLModelHierarchyNode")
    # Model hierarchy
    modelHierarchyNode = slicer.vtkMRMLModelHierarchyNode()
    slicer.mrmlScene.AddNode(modelHierarchyNode)

    self.delayDisplay("Clear scene")
    slicer.mrmlScene.Clear(0)

    self.delayDisplay('Test passed!')

  def test_ModelMakerBreaksSliceViewsIssue3462_2(self):
    self.delayDisplay("Starting the test #2")

    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    self.delayDisplay("Getting Data")
    head = sampleDataLogic.downloadMRHead()
    self.delayDisplay('Finished with download and loading')
    volumeNode = slicer.util.getNode('MRHead')

    self.delayDisplay("Setting up LabelMap")
    volumesLogic = slicer.modules.volumes.logic()
    headLabel = volumesLogic.CreateAndAddLabelVolume(slicer.mrmlScene, head, '%s-label' % head.GetName())
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(head.GetID())
    selectionNode.SetReferenceActiveLabelVolumeID(headLabel.GetID())
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    self.delayDisplay("Open Editor")
    slicer.util.moduleSelector().selectModule('Editor')

    from EditorLib.EditUtil import EditUtil
    self.delayDisplay("Select PaintEffect")
    EditUtil.setCurrentEffect('PaintEffect')
    EditUtil.getParameterNode().SetParameter('PaintEffect,radius', '10')

    lm = slicer.app.layoutManager()
    sliceView = lm.sliceWidget('Red').sliceView()
    logic = ScriptedLoadableModuleLogic()

    center = (sliceView.width / 2, sliceView.height / 2)

    self.delayDisplay("Draw label 1")
    EditUtil.setLabel(1)
    logic.clickAndDrag(sliceView, start=(center[0] - 20, center[1]), end=(center[0] - 20, center[1]), steps=1)

    self.delayDisplay("Open ModelMaker")
    slicer.util.moduleSelector().selectModule('ModelMaker')
    cliNode = slicer.cli.createNode(slicer.modules.modelmaker)

    # Input volume
    cliNode.SetParameterAsString('InputVolume', headLabel.GetID())

    # Model hierarchy
    modelHierarchyNode = slicer.vtkMRMLModelHierarchyNode()
    slicer.mrmlScene.AddNode(modelHierarchyNode)
    cliNode.SetParameterAsString('ModelSceneFile', modelHierarchyNode.GetID())

    cliWidget = slicer.modules.modelmaker.widgetRepresentation()
    cliWidget.setCurrentCommandLineModuleNode(cliNode)

    self.delayDisplay("Generate models")
    slicer.cli.runSync(slicer.modules.modelmaker, cliNode)

    slicer.mrmlScene.Clear(0)

    self.delayDisplay('Test passed!')

  def test_ModelMakerBreaksSliceViewsIssue3462_3(self):
    self.delayDisplay("Starting the test #3")

    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    self.delayDisplay("Getting Data")
    head = sampleDataLogic.downloadMRHead()
    self.delayDisplay('Finished with download and loading')
    volumeNode = slicer.util.getNode('MRHead')

    self.delayDisplay("Setting up LabelMap")
    volumesLogic = slicer.modules.volumes.logic()
    headLabel = volumesLogic.CreateAndAddLabelVolume(slicer.mrmlScene, head, '%s-label' % head.GetName())
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(head.GetID())
    selectionNode.SetReferenceActiveLabelVolumeID(headLabel.GetID())
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    self.delayDisplay("Open Editor")
    slicer.util.moduleSelector().selectModule('Editor')

    from EditorLib.EditUtil import EditUtil
    self.delayDisplay("Select PaintEffect")
    EditUtil.setCurrentEffect('PaintEffect')
    EditUtil.getParameterNode().SetParameter('PaintEffect,radius', '10')

    lm = slicer.app.layoutManager()
    sliceView = lm.sliceWidget('Red').sliceView()
    logic = ScriptedLoadableModuleLogic()

    center = (sliceView.width / 2, sliceView.height / 2)

    self.delayDisplay("Draw label 1")
    EditUtil.setLabel(1)
    logic.clickAndDrag(sliceView, start=(center[0] - 20, center[1]), end=(center[0] - 20, center[1]), steps=1)

    self.delayDisplay("Draw label 2")
    EditUtil.setLabel(2)
    logic.clickAndDrag(sliceView, start=(center[0] + 20, center[1]), end=(center[0] + 20, center[1]), steps=1)

    self.delayDisplay("Open ModelMaker")
    slicer.util.moduleSelector().selectModule('ModelMaker')
    cliNode = slicer.cli.createNode(slicer.modules.modelmaker)

    # Input volume
    cliNode.SetParameterAsString('InputVolume', headLabel.GetID())

    # Model hierarchy
    modelHierarchyNode = slicer.vtkMRMLModelHierarchyNode()
    slicer.mrmlScene.AddNode(modelHierarchyNode)
    cliNode.SetParameterAsString('ModelSceneFile', modelHierarchyNode.GetID())

    cliWidget = slicer.modules.modelmaker.widgetRepresentation()
    cliWidget.setCurrentCommandLineModuleNode(cliNode)

    self.delayDisplay("Generate models")
    slicer.cli.runSync(slicer.modules.modelmaker, cliNode)

    slicer.mrmlScene.Clear(0)

    self.delayDisplay('Test passed!')

  def test_ModelMakerBreaksSliceViewsIssue3462_4(self):

    self.delayDisplay("Starting the test #4")

    self.delayDisplay("Open Annotations")
    slicer.util.moduleSelector().selectModule('Annotations')

    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    self.delayDisplay("Getting Data")
    head = sampleDataLogic.downloadMRHead()
    self.delayDisplay('Finished with download and loading')
    volumeNode = slicer.util.getNode('MRHead')

    self.delayDisplay("Setting up LabelMap")
    volumesLogic = slicer.modules.volumes.logic()
    headLabel = volumesLogic.CreateAndAddLabelVolume(slicer.mrmlScene, head, '%s-label' % head.GetName())
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(head.GetID())
    selectionNode.SetReferenceActiveLabelVolumeID(headLabel.GetID())
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    self.delayDisplay("Open Editor")
    slicer.util.moduleSelector().selectModule('Editor')

    from EditorLib.EditUtil import EditUtil
    self.delayDisplay("Select PaintEffect")
    EditUtil.setCurrentEffect('PaintEffect')
    EditUtil.getParameterNode().SetParameter('PaintEffect,radius', '10')

    lm = slicer.app.layoutManager()
    sliceView = lm.sliceWidget('Red').sliceView()
    logic = ScriptedLoadableModuleLogic()

    center = (sliceView.width / 2, sliceView.height / 2)

    self.delayDisplay("Draw label 1")
    EditUtil.setLabel(1)
    logic.clickAndDrag(sliceView, start=(center[0] - 20, center[1]), end=(center[0] - 20, center[1]), steps=1)

    self.delayDisplay("Draw label 2")
    EditUtil.setLabel(2)
    logic.clickAndDrag(sliceView, start=(center[0] + 20, center[1]), end=(center[0] + 20, center[1]), steps=1)

    self.delayDisplay("Open ModelMaker")
    slicer.util.moduleSelector().selectModule('ModelMaker')
    cliNode = slicer.cli.createNode(slicer.modules.modelmaker)

    # Input volume
    cliNode.SetParameterAsString('InputVolume', headLabel.GetID())

    # Model hierarchy
    modelHierarchyNode = slicer.vtkMRMLModelHierarchyNode()
    slicer.mrmlScene.AddNode(modelHierarchyNode)
    cliNode.SetParameterAsString('ModelSceneFile', modelHierarchyNode.GetID())

    cliWidget = slicer.modules.modelmaker.widgetRepresentation()
    cliWidget.setCurrentCommandLineModuleNode(cliNode)

    self.delayDisplay("Generate models")
    slicer.cli.runSync(slicer.modules.modelmaker, cliNode)

    slicer.mrmlScene.Clear(0)

    self.delayDisplay('Test passed!')
