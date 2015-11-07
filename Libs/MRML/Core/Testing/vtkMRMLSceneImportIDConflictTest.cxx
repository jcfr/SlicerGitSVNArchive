/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingUtilities.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkPolyData.h>

// STD includes
#include <vtkNew.h>

using namespace vtkMRMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int vtkMRMLSceneImportIDConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  vtkNew<vtkMRMLScene> scene;

  // Add displayable node
  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  // add poly data
  vtkNew<vtkPolyData> polyData;
  modelNode->SetAndObservePolyData(polyData.GetPointer());
  std::cout << "Polydata pointer = " << polyData.GetPointer() << std::endl;

  // Add display node
  vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());
  modelNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkMRMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1 (valid polydata)

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 2)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelNode1", modelNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelDisplayNode1", modelDisplayNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelNode->GetDisplayNode() / modelDisplayNode",
              modelNode->GetDisplayNode(),
              modelDisplayNode.GetPointer())
      )
    {
    return false;
    }

  //
  // Import
  //

  // Here is the scene that will be imported:
  //
  //  Scene
  //    |---- vtkMRMLModelNode1  (null polydata / New Model1)
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1 (null polydata / New Display 1)
  //    |
  //    |---- vtkMRMLModelDisplayNode2 (null polydata / New Display 2)
  //    |
  //    |---- vtkMRMLModelNode2  (null polydata / New Model2)
  //               |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode2

  const char scene1XML[] =
    "<MRML  version=\"18916\" userTags=\"\">"
    "  <Model id=\"vtkMRMLModelNode1\" name=\"New Model1\" displayNodeRef=\"vtkMRMLModelDisplayNode1\" ></Model>"
    "  <ModelDisplay id=\"vtkMRMLModelDisplayNode1\" name=\"New Display 1\" ></ModelDisplay>"
    "  <ModelDisplay id=\"vtkMRMLModelDisplayNode2\" name=\"New Display 2\" ></ModelDisplay>"
    "  <Model id=\"vtkMRMLModelNode2\" name=\"New Model2\" displayNodeRef=\"vtkMRMLModelDisplayNode2\" ></Model>"
    "</MRML>"
    ;

  scene->SetSceneXMLString(scene1XML);
  scene->SetLoadFromXMLString(1);
  scene->Import();

  // When importing the scene, there is conflict between the existing nodes
  // and added nodes. New IDs are set by Import to the added nodes.

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkMRMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1 (valid polydata)
  //    |
  //    |---- vtkMRMLModelNode2  (null polydata / New Model1)            [was vtkMRMLModelNode1]
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode2
  //    |
  //    |---- vtkMRMLModelDisplayNode2 (null polydata / New Display 1)   [was vtkMRMLModelDisplayNode1]
  //    |
  //    |---- vtkMRMLModelDisplayNode3 (null polydata / New Display 2)   [was vtkMRMLModelDisplayNode2]
  //    |
  //    |---- vtkMRMLModelNode3  (null polydata / New Model2)            [was vtkMRMLModelNode2]
  //               |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode3

  //
  // Check scene contains original node
  //

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 6)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelNode1", modelNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelDisplayNode1", modelDisplayNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelNode->GetDisplayNode() / modelDisplayNode",
              modelNode->GetDisplayNode(),
              modelDisplayNode.GetPointer())
      )
    {
    return false;
    }

  //
  // Part 1
  //

  vtkMRMLModelNode* modelNode2 =
      vtkMRMLModelNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelNode3"));

  if (!CheckNotNull(
        __LINE__,
        "GetNodeByID(\"vtkMRMLModelNode3\")", modelNode2)

      ||!CheckNodeIdAndName(
        __LINE__, modelNode2, "vtkMRMLModelNode3", "New Model1")

      ||!CheckNodeIdAndName(
        __LINE__, modelNode2->GetDisplayNode(),
        "vtkMRMLModelDisplayNode3", "New Display 1")
      )
    {
    return EXIT_FAILURE;
    }

  //
  // Part2
  //

  vtkMRMLModelNode* modelNode3 =
      vtkMRMLModelNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelNode2"));

  if (!CheckNotNull(
        __LINE__,
        "GetNodeByID(\"vtkMRMLModelNode2\")", modelNode3)

      ||!CheckNodeIdAndName(
        __LINE__, modelNode3, "vtkMRMLModelNode2", "New Model2")

      ||!CheckNodeIdAndName(
        __LINE__, modelNode3->GetDisplayNode(),
        "vtkMRMLModelDisplayNode2", "New Display 2")
      )
    {
    return EXIT_FAILURE;
    }

  //
  // Check PolyData / InputPolyData
  //

  vtkMRMLModelDisplayNode* modelDisplayNode2 =
      vtkMRMLModelDisplayNode::SafeDownCast(modelNode2->GetDisplayNode());

  // check that the model nodes and model display nodes point to the right poly data
  if (!CheckNull(
        __LINE__,
        "Import failed: new model node should have null polydata\n"
        "modelNode2->GetPolyData()",
        modelNode2->GetPolyData())

      ||!CheckNull(
        __LINE__,
        "Import failed: new model node's display node should have null polydata\n"
        "modelDisplayNode2->GetInputPolyData()",
        modelDisplayNode2->GetInputPolyData())

      ||!CheckNotNull(
        __LINE__,
        "Import failed: original model node should not have null polydata\n"
        "modelNode->GetPolyData()",
        modelNode->GetPolyData())

      ||!CheckNotNull(
        __LINE__,
        "Import failed: original model display node should not have null polydata\n"
        "modelDisplayNode->GetInputPolyData()",
        modelDisplayNode->GetInputPolyData()
        )

      ||!CheckPointer(
        __LINE__,
        "Import failed: original model node and display node don't have the same poly data\n"
        "modelNode->GetPolyData() / modelDisplayNode->GetInputPolyData()",
        modelNode->GetPolyData(),
        modelDisplayNode->GetInputPolyData()
        )
      )
    {
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
