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
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <iostream>

namespace
{

//----------------------------------------------------------------------------
class vtkMRMLTestScene : public vtkMRMLScene
{
public:
  static vtkMRMLTestScene *New();
  typedef vtkMRMLTestScene Self;

  vtkTypeMacro(vtkMRMLTestScene, vtkMRMLScene);

  std::string test_ExtractBaseID(const std::string& nodeID)
  {
    return this->ExtractBaseID(nodeID);
  }

protected:
  vtkMRMLTestScene()
  {
  }
};
vtkStandardNewMacro(vtkMRMLTestScene);

}


//---------------------------------------------------------------------------
int vtkMRMLSceneIDTest(
  int vtkNotUsed(argc), char * vtkNotUsed(argv) [] )
{
  vtkNew<vtkMRMLTestScene> scene;

  //---------------------------------------------------------------------------
  // GenerateUniqueName
  //---------------------------------------------------------------------------
  std::string baseName("Node Name");
  std::string nodeName = scene->GenerateUniqueName(baseName);
  if (nodeName != std::string(baseName))
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  nodeName = scene->GenerateUniqueName(baseName);
  if (nodeName != std::string("Node Name_1"))
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  nodeName = scene->GenerateUniqueName(baseName);
  if (nodeName != std::string("Node Name_2"))
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  // "Node Name_1" is considered as a different basename
  std::string baseName_1 = baseName + "_1";
  nodeName = scene->GenerateUniqueName(baseName_1);
  if (nodeName != baseName_1)
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  nodeName = scene->GenerateUniqueName(baseName_1);
  if (nodeName != std::string("Node Name_1_1"))
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLModelNode> node;
  node->SetName("Node Name_4");
  scene->AddNode(node.GetPointer());

  // GenerateUniqueName() must check into the scene if there is not already
  // a node with the same name.
  nodeName = scene->GenerateUniqueName(std::string("Node Name_4"));
  if (nodeName != std::string("Node Name_4_1"))
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  // GenerateUniqueName() must check into the scene if there is not already
  // a node with the same name.
  nodeName = scene->GenerateUniqueName(baseName);
  if (nodeName != std::string("Node Name_3"))
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  // GenerateUniqueName() must check into the scene if there is not already
  // a node with the same name.
  nodeName = scene->GenerateUniqueName(baseName);
  if (nodeName != std::string("Node Name_5"))
    {
    std::cerr << "GenerateUniqueName failed: " << nodeName << std::endl;
    return EXIT_FAILURE;
    }

  //---------------------------------------------------------------------------
  // UniqueIDs
  //---------------------------------------------------------------------------
  if (node->GetID() != std::string("vtkMRMLModelNode1"))
    {
    std::cerr << __LINE__ << " Node ID failed: " << node->GetID() << std::endl;
    return EXIT_FAILURE;
    }

  // Test node ID increment
  vtkNew<vtkMRMLModelNode> node2;
  scene->AddNode(node2.GetPointer());

  if (node2->GetID() != std::string("vtkMRMLModelNode2"))
    {
    std::cerr << __LINE__ << " Node ID failed: "
              << node2->GetID() << std::endl;
    return EXIT_FAILURE;
    }

  // Test reserved ID
  scene->AddReservedID("vtkMRMLModelNode3");
  vtkNew<vtkMRMLModelNode> node3;
  scene->AddNode(node3.GetPointer());

  if (node3->GetID() != std::string("vtkMRMLModelNode4"))
    {
    std::cerr << __LINE__ << " Node ID failed: "
              << node3->GetID() << std::endl;
    return EXIT_FAILURE;
    }

  // Test reserved ID by removing the node
  scene->AddReservedID("vtkMRMLModelNode5");
  scene->RemoveReservedIDs();
  vtkNew<vtkMRMLModelNode> node4;
  scene->AddNode(node4.GetPointer());

  if (node4->GetID() != std::string("vtkMRMLModelNode5"))
    {
    std::cerr << __LINE__ << " Node ID failed: "
              << node4->GetID() << std::endl;
    return EXIT_FAILURE;
    }

  // Singleton nodes have their tag in the node ID
  vtkNew<vtkMRMLSliceNode> redSlice;
  redSlice->SetLayoutName("Red"); // sets the singleton tag
  scene->AddNode(redSlice.GetPointer());

  if (strcmp(redSlice->GetID(), "vtkMRMLSliceNodeRed") != 0)
    {
    std::cerr << __LINE__ << " Node ID failed: "
              << redSlice->GetID() << std::endl;
    return EXIT_FAILURE;
    }
  // Test increment when using different singleton tags
  vtkNew<vtkMRMLSliceNode> greenSlice;
  greenSlice->SetLayoutName("Green"); // sets the singleton tag
  scene->AddNode(greenSlice.GetPointer());

  if (strcmp(greenSlice->GetID(), "vtkMRMLSliceNodeGreen") != 0)
    {
    std::cerr << __LINE__ << " Node ID failed: "
              << greenSlice->GetID() << std::endl;
    return EXIT_FAILURE;
    }

  // Test Singleton unique ID
  vtkNew<vtkMRMLSliceNode> greenSlice2;
  greenSlice2->SetLayoutName("Green"); // sets the singleton tag
  scene->AddNode(greenSlice2.GetPointer());

  if (greenSlice2->GetID() != 0 ||
      scene->GetNodeByID("vtkMRMLSliceNodeGreen") != greenSlice.GetPointer())
    {
    std::cerr << __LINE__ << " Node ID failed: "
              << greenSlice->GetID() << std::endl;
    return EXIT_FAILURE;
    }


  //---------------------------------------------------------------------------
  // ExtractBaseID
  //---------------------------------------------------------------------------
  {
    std::string current = scene->test_ExtractBaseID("vtkMRMLModelNode5");
    std::string expected = "vtkMRMLModelNode";
    if (current != expected)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with ExtractBaseID()"
                << "\n\tcurrent: " << current
                << "\n\texpected: " << expected << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    std::string current = scene->test_ExtractBaseID("vtkMRMLModelNode12");
    std::string expected = "vtkMRMLModelNode";
    if (current != expected)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with ExtractBaseID()"
                << "\n\tcurrent: " << current
                << "\n\texpected: " << expected << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    std::string current = scene->test_ExtractBaseID("vtkMRMLModelNode");
    std::string expected = "vtkMRMLModelNode";
    if (current != expected)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with ExtractBaseID()"
                << "\n\tcurrent: " << current
                << "\n\texpected: " << expected << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    std::string current = scene->test_ExtractBaseID("vtkMRMLModel1Node1");
    std::string expected = "vtkMRMLModel1Node";
    if (current != expected)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with ExtractBaseID()"
                << "\n\tcurrent: " << current
                << "\n\texpected: " << expected << std::endl;
      return EXIT_FAILURE;
      }
  }

  return EXIT_SUCCESS;
}
