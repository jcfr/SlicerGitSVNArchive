/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkMRMLGlyphableVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageBlend.h>
#include <vtkImageResample.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageMathematics.h>
#include <vtkImageReslice.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

// STD includes

//----------------------------------------------------------------------------
// Convenient macros
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------------
const int vtkMRMLSliceLogic::SLICE_INDEX_ROTATED=-1;
const int vtkMRMLSliceLogic::SLICE_INDEX_OUT_OF_VOLUME=-2;
const int vtkMRMLSliceLogic::SLICE_INDEX_NO_VOLUME=-3;
const std::string vtkMRMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX = std::string("Volume Slice");

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLSliceLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLSliceLogic);

//---------------------------------------------------------------------------
class vtkMRMLSliceLogic::vtkInternal
{
public:
  vtkInternal(vtkMRMLSliceLogic* external);
  ~vtkInternal();

  /// Layers
  typedef vtkSmartPointer<vtkMRMLSliceLayerLogic> LayerListItem;
  typedef std::vector<LayerListItem> LayerList;
  typedef std::vector<LayerListItem>::iterator LayerListIterator;
  typedef std::vector<LayerListItem>::const_iterator LayerListConstIterator;

  static vtkMRMLVolumeNode* GetFirstNonNullLayerVolumeNode(const LayerList& layers);
  static bool IsDefaultLayer(unsigned int layerIndex);
  static std::string GetLayerName(unsigned int layerIndex);
  static std::string GetImageMathematicsOperationName(unsigned int operation);
  int GetNextNonNullImageLayerIndex(unsigned int layerIndex);

  LayerList Layers;

  /// Pipeline
  struct PipelineItem {
    PipelineItem();
    ~PipelineItem(){std::cout << "dtor PipelineItem" << std::endl;}
    void SetCompositingMode(int compositingMode);
    void PrintSelf(ostream& os, vtkIndent indent);
    void SetInput1(vtkImageData* image1, double opacity);
    void SetInput0(vtkImageData* image0);
    vtkImageData* GetOutput();
    vtkImageData* Output;
    // For alpha blending
    vtkNew<vtkImageBlend> Blend;
    // For addition, subtraction
    vtkNew<vtkImageMathematics> Math;
    vtkNew<vtkImageCast> Cast;
    // Compositing
    int CompositingMode;
    bool AlphaBlending;
  };

  typedef std::vector<PipelineItem*> PipelineType;
  typedef std::vector<PipelineItem*>::iterator PipelineIteratorType;

  void PrintPipeline(const PipelineType& pipeline, ostream& os, vtkIndent indent);
  void SetPipelineItem(PipelineType &pipeline, unsigned int pipelineItemIndex, vtkImageData* image1, double opacity, int compositingMode);
  void ClearPipeline(PipelineType &pipelineList);
  void TruncatePipeline(PipelineType &pipelineList, unsigned int position);
  vtkImageData* GetPipelineOutput();
  vtkImageData* GetPipelineOutputUVW();

  PipelineType Pipeline;
  PipelineType PipelineUVW;

  /// Miscellaneous
  char * Name;
  double SliceSpacing[3];
  vtkWeakPointer<vtkImageData> ImageData;
  vtkNew<vtkImageReslice> ExtractModelTexture;

  /// Node references
  vtkMRMLSliceNode *            SliceNode;
  vtkMRMLSliceCompositeNode *   SliceCompositeNode;
  vtkMRMLModelNode *            SliceModelNode;
  vtkMRMLModelDisplayNode *     SliceModelDisplayNode;
  vtkMRMLLinearTransformNode *  SliceModelTransformNode;

  vtkMRMLSliceLogic* External;
};

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::vtkInternal::vtkInternal(vtkMRMLSliceLogic* external)
{
  this->Name = 0;
  this->SliceSpacing[0] = this->SliceSpacing[1] = this->SliceSpacing[2] = 1;
  this->ExtractModelTexture->SetOutputDimensionality(2);

  this->SliceNode = 0;
  this->SliceCompositeNode = 0;
  this->SliceModelNode = 0;
  this->SliceModelDisplayNode = 0;
  this->SliceModelTransformNode = 0;

  this->External = external;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::vtkInternal::~vtkInternal()
{
  if (this->Name)
    {
    delete[] this->Name;
    this->Name = 0;
    }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLSliceLogic::vtkInternal::GetFirstNonNullLayerVolumeNode(
    const LayerList& layers)
{
  for (LayerListConstIterator iterator = layers.begin();
       iterator != layers.end();
       ++iterator)
    {
    vtkMRMLSliceLayerLogic * layer = *iterator;
    if (!layer)
      {
      continue;
      }
    vtkMRMLVolumeNode *volumeNode = layer->GetVolumeNode();
    if (volumeNode)
      {
      return volumeNode;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::vtkInternal::IsDefaultLayer(unsigned int layerIndex)
{
  return (layerIndex == vtkMRMLSliceCompositeNode::BackgroundLayer ||
          layerIndex == vtkMRMLSliceCompositeNode::ForegroundLayer ||
          layerIndex == vtkMRMLSliceCompositeNode::LabelLayer);
}

//----------------------------------------------------------------------------
std::string vtkMRMLSliceLogic::vtkInternal::GetLayerName(unsigned int layerIndex)
{
  std::string layerName;
  std::stringstream out;
  out << layerIndex;
  layerName = out.str();
  layerName += "th layer";
  if (layerIndex == vtkMRMLSliceCompositeNode::BackgroundLayer)
    {
    layerName += " (Background)";
    }
  else if (layerIndex == vtkMRMLSliceCompositeNode::ForegroundLayer)
    {
    layerName += " (Foreground)";
    }
  else if (layerIndex == vtkMRMLSliceCompositeNode::LabelLayer)
    {
    layerName += " (Label)";
    }
  return layerName;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSliceLogic::vtkInternal::GetImageMathematicsOperationName(unsigned int operation)
{
  if (operation == VTK_ADD)
    {
    return std::string("ADD");
    }
  else if (operation == VTK_SUBTRACT)
    {
    return std::string("SUBTRACT");
    }
  else
    {
    return std::string("UNKOWN");
    }
}

//----------------------------------------------------------------------------
int vtkMRMLSliceLogic::vtkInternal::GetNextNonNullImageLayerIndex(unsigned int layerIndex)
{
  for (unsigned int index = layerIndex; index < this->Layers.size(); ++index)
    {
    if (this->Layers.at(index))
      {
      vtkMRMLSliceLayerLogic* layer = this->Layers.at(index);
      if (layer->GetImageData())
        {
        return index;
        }
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::vtkInternal::PipelineItem::PipelineItem() :
  Output(0), CompositingMode(vtkMRMLSliceCompositeNode::Alpha), AlphaBlending(true)
{
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::PipelineItem::SetCompositingMode(int compositingMode)
{
  if (compositingMode == this->CompositingMode)
    {
    return;
    }
  this->CompositingMode = compositingMode;
  this->AlphaBlending = (this->CompositingMode == vtkMRMLSliceCompositeNode::Alpha ||
                         this->CompositingMode == vtkMRMLSliceCompositeNode::ReverseAlpha);

  if (this->CompositingMode == vtkMRMLSliceCompositeNode::Add)
    {
    // add the foreground and background
    this->Math->SetOperationToAdd();
    }
  else if (this->CompositingMode == vtkMRMLSliceCompositeNode::Subtract)
    {
    // subtract the foreground and background
    this->Math->SetOperationToSubtract();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::PipelineItem::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "PipelineItem\n";
  os << indent << "  CompositingMode:" << this->CompositingMode<< "\n";
  os << indent << "  AlphaBlending:" << this->AlphaBlending<< "\n";
  if (this->Output)
    {
    os << indent << "  Type:" << "IDENTITY" << "\n";
    os << indent << "    Input:" << this->Output << "\n";
    }
  else
    {
    if (this->AlphaBlending)
      {
      os << indent << "  Type:" << "BLEND" << "\n";
      os << indent << "    Input_0:" << this->Blend->GetInput(0) << "\n";
      os << indent << "    Opacity_0:" << this->Blend->GetOpacity(0) << "\n";
      os << indent << "    Input_1:" << this->Blend->GetInput(1) << "\n";
      os << indent << "    Opacity_1:" << this->Blend->GetOpacity(1) << "\n";
      }
    else
      {
      os << indent << "  Type:"
         << vtkInternal::GetImageMathematicsOperationName(this->Math->GetOperation()) << "\n";
      os << indent << "    Input_0:" << this->Math->GetInput(0) << "\n";
      os << indent << "    Input_1:" << this->Math->GetInput(1) << "\n";
      }
    }
  os << indent << "  Output:" << this->GetOutput()<< "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::PipelineItem::SetInput1(vtkImageData* image1, double opacity)
{
  if (!this->AlphaBlending)
    {
    this->Math->SetInput1(image1);
    }
  else if(this->CompositingMode == vtkMRMLSliceCompositeNode::Alpha)
    {
    this->Blend->SetInput(1, image1);
    this->Blend->SetOpacity(1, opacity);
    }
  else if(this->CompositingMode == vtkMRMLSliceCompositeNode::ReverseAlpha)
    {
    this->Blend->SetInput(0, image1);
    this->Blend->SetOpacity(1, opacity);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::PipelineItem::SetInput0(vtkImageData* image0)
{
  if (!this->AlphaBlending)
    {
    this->Math->SetInput2(image0);
    }
  else if(this->CompositingMode == vtkMRMLSliceCompositeNode::Alpha)
    {
    this->Blend->SetInput(0, image0);
    this->Blend->SetOpacity(0, 1.0);
    }
  else if(this->CompositingMode == vtkMRMLSliceCompositeNode::ReverseAlpha)
    {
    this->Blend->SetInput(1, image0);
    this->Blend->SetOpacity(0, 1.0);
    }
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLogic::vtkInternal::PipelineItem::GetOutput()
{
  if (this->Output)
    {
    return this->Output;
    }
  if (!this->AlphaBlending)
    {
    this->Math->GetOutput()->SetScalarType(VTK_SHORT);
    this->Cast->SetInput(this->Math->GetOutput());
    this->Cast->SetOutputScalarTypeToUnsignedChar();
    return this->Cast->GetOutput();
    }
  else
    {
    return this->Blend->GetOutput();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::PrintPipeline(
    const PipelineType& pipeline, ostream& os, vtkIndent indent)
{
  os << indent << "Number of items:" << pipeline.size() << "\n";
  for (unsigned int stageIndex = 0; stageIndex < pipeline.size(); ++stageIndex)
    {
    pipeline.at(stageIndex)->PrintSelf(os, indent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::SetPipelineItem(
    PipelineType &pipeline,
    unsigned int pipelineItemIndex,
    vtkImageData* image1,
    double opacity,
    int compositingMode)
{
  vtkImageData * image0 = 0;
  if (pipelineItemIndex >= 1 &&
      pipelineItemIndex < pipeline.size())
    {
    image0 = pipeline.at(pipelineItemIndex - 1)->GetOutput();
    }

  // Get existing item if any
  PipelineItem* pipelineItem = 0;
  if (pipelineItemIndex < pipeline.size())
    {
    pipelineItem = pipeline.at(pipelineItemIndex);
    }
  else
    {
    pipelineItem = new PipelineItem();
    pipeline.push_back(pipelineItem);
    }

  pipelineItem->Output = 0;
  pipelineItem->SetCompositingMode(compositingMode);

  if (image0)
    {
    pipelineItem->SetInput0(image0);
    pipelineItem->SetInput1(image1, opacity);
    }
  else
    {
    pipelineItem->Output = image1;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::ClearPipeline(
    PipelineType& pipelineList)
{
  this->TruncatePipeline(pipelineList, 0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::vtkInternal::TruncatePipeline(
    PipelineType& pipelineList, unsigned int position = 0)
{
  while(pipelineList.size() > position)
    {
    PipelineIteratorType it = pipelineList.end() - 1;
    delete *it;
    pipelineList.erase(it);
    }
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLogic::vtkInternal::GetPipelineOutput()
{
  if (this->Pipeline.size() > 0)
    {
    PipelineItem * pipelineItem = this->Pipeline.back();
    return pipelineItem->GetOutput();
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLogic::vtkInternal::GetPipelineOutputUVW()
{
  if (this->PipelineUVW.size() > 0)
    {
    PipelineItem * pipelineItem = this->PipelineUVW.back();
    return pipelineItem->GetOutput();
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::vtkMRMLSliceLogic()
{
  this->Internal = new vtkInternal(this);
  this->SetName("");
}

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::~vtkMRMLSliceLogic()
{
  this->Internal->ClearPipeline(this->Internal->Pipeline);
  this->Internal->ClearPipeline(this->Internal->PipelineUVW);

  this->SetSliceNode(0);

  for (unsigned int layerIndex = 0;
       layerIndex < this->Internal->Layers.size();
       ++layerIndex)
    {
    this->SetLayer(layerIndex, 0);
    }

  if (this->Internal->SliceCompositeNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->Internal->SliceCompositeNode, 0);
    }

  this->DeleteSliceModel();

  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  // Sanity checks
  if (!this->GetName() || strlen(this->GetName()) == 0)
    {
    vtkErrorMacro(<< "Name is NULL - Make sure you call SetName before SetMRMLScene !");
    return;
    }

  // List of events the slice logics should listen
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartCloseEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());

  this->ProcessMRMLLogicsEvents();

  this->GetBackgroundLayer()->SetMRMLScene(newScene);
  this->GetForegroundLayer()->SetMRMLScene(newScene);
  this->GetLabelLayer()->SetMRMLScene(newScene);

  this->ProcessMRMLSceneEvents(newScene, vtkMRMLScene::EndBatchProcessEvent, 0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetSliceNode(0);
    return;
    }
  // find SliceNode in the scene
  vtkSmartPointer<vtkMRMLSliceNode> node;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if (node->GetLayoutName() && !strcmp(node->GetLayoutName(), this->GetName()))
      {
      break;
      }
    }

  if ( this->Internal->SliceNode != 0 && node != 0 &&
        this->Internal->SliceCompositeNode &&
       (this->Internal->SliceCompositeNode->GetID() == 0 ||
        strcmp(this->Internal->SliceNode->GetID(), node->GetID()) != 0 ))
    {
    // local SliceNode is out of sync with the scene
    this->SetSliceNode (0);
    }

  if ( this->Internal->SliceNode == 0 )
    {
    if ( node == 0 )
      {
      node = vtkSmartPointer<vtkMRMLSliceNode>::New();
      node->SetName(this->GetName());
      node->SetLayoutName(this->GetName());
      this->SetSliceNode (node);
      this->UpdateSliceNodeFromLayout();
      }
    else
      {
      this->SetSliceNode (node);
      }
    }

  if ( this->GetMRMLScene()->GetNodeByID(this->Internal->SliceNode->GetID()) == 0)
    {
    // local node not in the scene
    node = this->Internal->SliceNode;
    this->SetSliceNode (0);
    this->GetMRMLScene()->AddNode(node);
    this->SetSliceNode(node);
    }

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNodeFromLayout()
{
  if (this->Internal->SliceNode == 0)
    {
    return;
    }

  if ( !strcmp( this->GetName(), "Red" ) )
    {
    this->Internal->SliceNode->SetOrientationToAxial();
    }
  if ( !strcmp( this->GetName(), "Yellow" ) )
    {
    this->Internal->SliceNode->SetOrientationToSagittal();
    }
  if ( !strcmp( this->GetName(), "Green" ) )
    {
    this->Internal->SliceNode->SetOrientationToCoronal();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceCompositeNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetSliceCompositeNode(0);
    return;
    }
  // find SliceCompositeNode in the scene
  vtkSmartPointer<vtkMRMLSliceCompositeNode> node =
      vtkMRMLSliceLogic::GetSliceCompositeNode(this->GetMRMLScene(), this->GetName());

  if ( this->Internal->SliceCompositeNode != 0 && node != 0 &&
       (this->Internal->SliceCompositeNode->GetID() == 0 ||
        strcmp(this->Internal->SliceCompositeNode->GetID(), node->GetID()) != 0) )
    {
    // local SliceCompositeNode is out of sync with the scene
    this->SetSliceCompositeNode (0);
    }

  if ( this->Internal->SliceCompositeNode == 0 )
    {
    if ( node == 0 )
      {
      node = vtkSmartPointer<vtkMRMLSliceCompositeNode>::New();
      node->SetLayoutName(this->GetName());
      this->SetSliceCompositeNode (node);
      }
    else
      {
      this->SetSliceCompositeNode (node);
      }
    }

  if ( this->GetMRMLScene()->GetNodeByID(this->Internal->SliceCompositeNode->GetID()) == 0)
    {
    // local node not in the scene
    node = this->Internal->SliceCompositeNode;
    this->SetSliceCompositeNode(0);
    this->GetMRMLScene()->AddNode(node);
    this->SetSliceCompositeNode(node);
    }

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateFromMRMLScene()
{
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!(node->IsA("vtkMRMLSliceCompositeNode")
        || node->IsA("vtkMRMLSliceNode")
        || node->IsA("vtkMRMLVolumeNode")))
    {
    return;
    }
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!(node->IsA("vtkMRMLSliceCompositeNode")
        || node->IsA("vtkMRMLSliceNode")
        || node->IsA("vtkMRMLVolumeNode")))
    {
    return;
    }
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneStartClose()
{
  this->UpdateSliceNodeFromLayout();
  this->DeleteSliceModel();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneEndImport()
{
  this->SetupCrosshairNode();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneEndRestore()
{
  this->SetupCrosshairNode();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNodes()
{
  if (this->GetMRMLScene()
      && this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }
  // Set up the nodes
  this->UpdateSliceNode();
  this->UpdateSliceCompositeNode();

  // Set up the models
  this->CreateSliceModel();

  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetupCrosshairNode()
{
  //
  // On a new scene or restore, create the singleton for the default crosshair
  // for navigation or cursor if it doesn't already exist in scene
  //
  bool foundDefault = false;
  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> crosshairs = this->GetMRMLScene()->GetNodesByClass("vtkMRMLCrosshairNode");
  for (crosshairs->InitTraversal(it);
       (node = (vtkMRMLNode*)crosshairs->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLCrosshairNode* crosshairNode =
      vtkMRMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode
        && crosshairNode->GetCrosshairName() == std::string("default"))
      {
      foundDefault = true;
      break;
      }
    }
  crosshairs->Delete();

  if (!foundDefault)
    {
    vtkNew<vtkMRMLCrosshairNode> crosshair;
    this->GetMRMLScene()->AddNode(crosshair.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLNodeModified(vtkMRMLNode* node)
{
  assert(node);
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }

  /// set slice extents in the layes
  this->SetSliceExtentsToSliceNode();

  // Update from SliceNode
  if (node == this->Internal->SliceNode)
    {
    // assert (sliceNode == this->Internal->SliceNode); not an assert because the node
    // might have change in CreateSliceModel() or UpdateSliceNode()
    vtkMRMLDisplayNode* sliceDisplayNode =
      this->Internal->SliceModelNode ? this->Internal->SliceModelNode->GetModelDisplayNode() : 0;
    if ( sliceDisplayNode)
      {
      sliceDisplayNode->SetVisibility( this->Internal->SliceNode->GetSliceVisible() );
      sliceDisplayNode->SetViewNodeIDs( this->Internal->SliceNode->GetThreeDViewIDs());
      }
    }
  else if (node == this->Internal->SliceCompositeNode)
    {
    this->UpdatePipeline();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::ProcessMRMLLogicsEvents(vtkObject* vtkNotUsed(caller),
                          unsigned long vtkNotUsed(event),
                          void* vtkNotUsed(callData))
{
  this->ProcessMRMLLogicsEvents();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::ProcessMRMLLogicsEvents()
{

  //
  // if we don't have layers yet, create them
  //
  for(unsigned int layerIndex = 0; layerIndex < vtkMRMLSliceCompositeNode::LabelLayer + 1; ++layerIndex)
    {
    if ( this->GetLayer(layerIndex) == 0 )
      {
      vtkNew<vtkMRMLSliceLayerLogic> layer;
      layer->SetIsLabelLayer(layerIndex == vtkMRMLSliceCompositeNode::LabelLayer);
      this->SetLayer(layerIndex, layer.GetPointer());
      }
    }

  // Update slice plane geometry
  if (this->Internal->SliceNode != 0
      && this->GetSliceModelNode() != 0
      && this->GetMRMLScene() != 0
      && this->GetMRMLScene()->GetNodeByID( this->Internal->SliceModelNode->GetID() ) != 0
      && this->Internal->SliceModelNode->GetPolyData() != 0 )
    {

    vtkPoints *points = this->Internal->SliceModelNode->GetPolyData()->GetPoints();

    int *dims1=0;
    int dims[3];
    vtkMatrix4x4 *textureToRAS = 0;
    if (this->Internal->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView)
      {
      textureToRAS = this->Internal->SliceNode->GetUVWToRAS();
      dims1 = this->Internal->SliceNode->GetUVWDimensions();
      dims[0] = dims1[0]-1;
      dims[1] = dims1[1]-1;
      }
    else
      {
      textureToRAS = this->Internal->SliceNode->GetXYToRAS();
      dims1 = this->Internal->SliceNode->GetDimensions();
      dims[0] = dims1[0];
      dims[1] = dims1[1];
      }

    // set the plane corner point for use in a model
    double inPt[4]={0,0,0,1};
    double outPt[4];
    double *outPt3 = outPt;

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->Internal->SliceNode->GetActiveSlice();

    textureToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(0, outPt3);

    inPt[0] = dims[0];
    textureToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(1, outPt3);

    inPt[0] = 0;
    inPt[1] = dims[1];
    textureToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(2, outPt3);

    inPt[0] = dims[0];
    inPt[1] = dims[1];
    textureToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(3, outPt3);

    this->UpdatePipeline();
    points->Modified();
    this->Internal->SliceModelNode->GetPolyData()->Modified();
    vtkMRMLModelDisplayNode *modelDisplayNode = this->Internal->SliceModelNode->GetModelDisplayNode();
    if ( modelDisplayNode )
      {
      if (this->GetLayerImageDataUVW(vtkMRMLSliceCompositeNode::LabelLayer))
        {
        modelDisplayNode->SetInterpolateTexture(0);
        }
      else
        {
        modelDisplayNode->SetInterpolateTexture(1);
        }
      if ( this->Internal->SliceCompositeNode != 0 )
        {
        modelDisplayNode->SetSliceIntersectionVisibility( this->Internal->SliceCompositeNode->GetSliceIntersectionVisibility() );
        }
      }
    }

  // This is called when a slice layer is modified, so pass it on
  // to anyone interested in changes to this sub-pipeline
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetName(const char* value)
{
  vtkSetStringPimplBodyMacro(Internal->Name, value);
}

//----------------------------------------------------------------------------
char* vtkMRMLSliceLogic::GetName()
{
  return this->Internal->Name;
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceLogic::GetSliceNode()
{
  return this->Internal->SliceNode;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceNode(vtkMRMLSliceNode * newSliceNode)
{
  if (this->Internal->SliceNode == newSliceNode)
    {
    return;
    }

  // Observe the slice node for general properties like slice visibility.
  // But the slice layers will also notify us when things like transforms have
  // changed.
  // This class takes care of passing the one slice node to each of the layers
  // so that users of this class only need to set the node in one place.
  vtkSetAndObserveMRMLNodeMacro( this->Internal->SliceNode, newSliceNode );

  for (vtkInternal::LayerListIterator iterator = this->Internal->Layers.begin();
       iterator != this->Internal->Layers.end();
       ++iterator)
    {
    vtkMRMLSliceLayerLogic * layer = *iterator;
    if (layer)
      {
      layer->SetSliceNode(newSliceNode);
      }
    }

  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceLogic::GetSliceCompositeNode()
{
  return this->Internal->SliceCompositeNode;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceCompositeNode(vtkMRMLSliceCompositeNode *sliceCompositeNode)
{
  // Observe the composite node, since this holds the parameters for this pipeline
  vtkSetAndObserveMRMLNodeMacro( this->Internal->SliceCompositeNode, sliceCompositeNode );
  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetLayer(unsigned int layerIndex)
{
  if (layerIndex < this->Internal->Layers.size())
    {
    return this->Internal->Layers.at(layerIndex);
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetLayer(unsigned int layerIndex, vtkMRMLSliceLayerLogic *layer)
{
  vtkMRMLSliceLayerLogic * currentLayer = this->GetLayer(layerIndex);
  if (currentLayer)
    {
    currentLayer->SetMRMLScene( 0 );
    }
  if (layerIndex >= this->Internal->Layers.size())
    {
    this->Internal->Layers.resize(layerIndex + 1);
    }
  this->Internal->Layers.at(layerIndex) = layer;
  if (layer)
    {
    layer->SetMRMLScene(this->GetMRMLScene());

    layer->SetSliceNode(this->Internal->SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      layer, vtkCommand::ModifiedEvent,
      this, this->GetMRMLLogicsCallbackCommand());
    }
  this->UpdatePipeline();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLogic::GetLayerImageData(unsigned int layerIndex)
{
  return this->GetLayer(layerIndex) ? this->GetLayer(layerIndex)->GetImageData() : 0;
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLSliceLogic::GetLayerImageDataUVW(unsigned int layerIndex)
{
  return this->GetLayer(layerIndex) ? this->GetLayer(layerIndex)->GetImageDataUVW() : 0;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetLayerVolumeNode(int layerIndex, vtkMRMLVolumeNode* volumeNode)
{
  if (this->GetLayer(layerIndex))
    {
    this->GetLayer(layerIndex)->SetVolumeNode(volumeNode);
    }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLSliceLogic::GetLayerVolumeNode(int layerIndex)
{
  return this->GetLayer(layerIndex) ? this->GetLayer(layerIndex)->GetVolumeNode() : 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetBackgroundLayer()
{
  return this->GetLayer(vtkMRMLSliceCompositeNode::BackgroundLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetBackgroundLayer(vtkMRMLSliceLayerLogic *backgroundLayer)
{
  this->SetLayer(vtkMRMLSliceCompositeNode::BackgroundLayer, backgroundLayer);
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetForegroundLayer()
{
  return this->GetLayer(vtkMRMLSliceCompositeNode::ForegroundLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetForegroundLayer(vtkMRMLSliceLayerLogic *foregroundLayer)
{
  this->SetLayer(vtkMRMLSliceCompositeNode::ForegroundLayer, foregroundLayer);
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetLabelLayer()
{
  return this->GetLayer(vtkMRMLSliceCompositeNode::LabelLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetLabelLayer(vtkMRMLSliceLayerLogic *labelLayer)
{
  this->SetLayer(vtkMRMLSliceCompositeNode::LabelLayer, labelLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::SetBackgroundWindowLevel(double newWindow, double newLevel)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer));
    // 0 is background layer, defined in this::GetLayerVolumeNode
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode =
    volumeNode ? volumeNode->GetScalarVolumeDisplayNode() : 0;
  if (!volumeDisplayNode)
    {
    return;
    }
  int disabledModify = volumeDisplayNode->StartModify();
  volumeDisplayNode->SetAutoWindowLevel(0);
  volumeDisplayNode->SetWindowLevel(newWindow, newLevel);
  volumeDisplayNode->EndModify(disabledModify);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::GetBackgroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer) );
  if (!volumeNode)
    {
    return;
    }
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode =
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  if (!volumeDisplayNode)
    {
    return;
    }
  vtkImageData* imageData = volumeNode->GetImageData();
  if (!imageData)
    {
    return;
    }
  window = volumeDisplayNode->GetWindow();
  level = volumeDisplayNode->GetLevel();
  double range[2];
  imageData->GetScalarRange(range);
  rangeLow = range[0];
  rangeHigh = range[1];
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLSliceLogic::GetSliceModelNode()
{
  return this->Internal->SliceModelNode;
}

//----------------------------------------------------------------------------
vtkMRMLModelDisplayNode* vtkMRMLSliceLogic::GetSliceModelDisplayNode()
{
  return this->Internal->SliceModelDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLSliceLogic::GetSliceModelTransformNode()
{
  return this->Internal->SliceModelTransformNode;
}

//----------------------------------------------------------------------------
vtkImageReslice* vtkMRMLSliceLogic::GetExtractModelTexture()
{
  return this->Internal->ExtractModelTexture.GetPointer();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::HasInputs()
{
  for (vtkInternal::LayerListIterator iterator = this->Internal->Layers.begin();
       iterator != this->Internal->Layers.end();
       ++iterator)
    {
    vtkMRMLSliceLayerLogic* layer = *iterator;
    if (layer && layer->GetImageData())
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
vtkImageData * vtkMRMLSliceLogic::GetImageData()
{
  if (this->HasInputs())
    {
    return this->Internal->ImageData;
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdatePipeline()
{
  if (!this->Internal->SliceCompositeNode ||
      !this->Internal->SliceNode)
    {
    return;
    }

  int modified = 0;

  // get the background and foreground image data from the layers
  // so we can use them as input to the image blend

  for (unsigned int layerIndex = 0;
       layerIndex < this->Internal->Layers.size();
       ++layerIndex)
    {
    vtkMRMLSliceLayerLogic * layer = this->Internal->Layers.at(layerIndex);
    if (!layer)
      {
      continue;
      }
    vtkMRMLVolumeNode *layerNode =
        this->Internal->SliceCompositeNode->GetLayerVolume(layerIndex);
    if (this->GetLayerVolumeNode(layerIndex) != layerNode)
      {
      this->SetLayerVolumeNode(layerIndex, layerNode);
      modified = 1;
      }
    }

  // set slice extents in the layers
  if (modified)
    {
    this->SetSliceExtentsToSliceNode();
    }

  vtkMRMLModelDisplayNode *modelDisplayNode =
    this->Internal->SliceModelNode ? this->Internal->SliceModelNode->GetModelDisplayNode() : 0;

  // update the slice intersection visibility to track the composite node setting
  if ( modelDisplayNode )
    {
    modelDisplayNode->SetSliceIntersectionVisibility(
      this->Internal->SliceCompositeNode->GetSliceIntersectionVisibility() );
    }

  unsigned int pipelineItemIndex = 0;

  for (unsigned int layerIndex = 0; layerIndex < this->Internal->Layers.size(); ++layerIndex)
    {
    int nextLayerIndex = this->Internal->GetNextNonNullImageLayerIndex(layerIndex);
    if (nextLayerIndex == -1)
      {
      continue;
      }
    vtkMRMLSliceLayerLogic* layer = this->GetLayer(nextLayerIndex);
    double opacity = this->Internal->SliceCompositeNode->GetLayerOpacity(nextLayerIndex);

    // By default, blend label layer
    int compositingMode = this->Internal->SliceCompositeNode->GetLayerCompositing(nextLayerIndex);
    if (vtkInternal::IsDefaultLayer(nextLayerIndex) && layer->GetIsLabelLayer())
      {
      compositingMode = vtkMRMLSliceCompositeNode::Alpha;
      }

    // 2D slice view pipeline
    this->Internal->SetPipelineItem(
          this->Internal->Pipeline, pipelineItemIndex,
          layer->GetImageData(), opacity, compositingMode);

    // UVW 3D slice plane texture pipeline
    if (this->Internal->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView)
      {
      this->Internal->SetPipelineItem(
            this->Internal->PipelineUVW, pipelineItemIndex,
            layer->GetImageDataUVW(), opacity, compositingMode);
      }

    ++pipelineItemIndex;
    layerIndex = nextLayerIndex;
    }

  // Truncate unused pipeline tail
  this->Internal->TruncatePipeline(this->Internal->Pipeline, pipelineItemIndex);
  this->Internal->TruncatePipeline(this->Internal->PipelineUVW, pipelineItemIndex);

  modified = 1;

  this->Internal->ImageData = this->Internal->GetPipelineOutput();

  //Models
  if (this->Internal->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
    {
    this->Internal->ExtractModelTexture->SetInput(this->Internal->ImageData);
    }
  else
    {
    this->Internal->ExtractModelTexture->SetInput(this->Internal->GetPipelineOutputUVW());
    }

  if (modelDisplayNode)
    {
    modelDisplayNode->SetVisibility(this->Internal->SliceNode->GetSliceVisible());
    modelDisplayNode->SetViewNodeIDs(this->Internal->SliceNode->GetThreeDViewIDs());
    if (!this->HasInputs())
      {
      modelDisplayNode->SetAndObserveTextureImageData(0);
      }
    else if (modelDisplayNode->GetTextureImageData() != this->Internal->ExtractModelTexture->GetOutput())
      {
      // update texture
      modelDisplayNode->SetAndObserveTextureImageData(this->Internal->ExtractModelTexture->GetOutput());
      }

    if (this->GetLayerImageData(vtkMRMLSliceCompositeNode::LabelLayer))
      {
      modelDisplayNode->SetInterpolateTexture(0);
      }
    else
      {
      modelDisplayNode->SetInterpolateTexture(1);
      }
    }
  if ( modified )
    {
    if (this->Internal->SliceModelNode && this->Internal->SliceModelNode->GetPolyData())
      {
      this->Internal->SliceModelNode->GetPolyData()->Modified();
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();

  os << indent << "SlicerSliceLogic:             " << this->GetClassName() << "\n";
  os << indent << "SLICE_MODEL_NODE_NAME_SUFFIX: " << this->SLICE_MODEL_NODE_NAME_SUFFIX << "\n";

  if (this->Internal->SliceNode)
    {
    os << indent << "SliceNode: ";
    os << (this->Internal->SliceNode->GetID() ? this->Internal->SliceNode->GetID() : "(0 ID)") << "\n";
    this->Internal->SliceNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "SliceNode: (none)\n";
    }

  if (this->Internal->SliceCompositeNode)
    {
    os << indent << "SliceCompositeNode: ";
    os << (this->Internal->SliceCompositeNode->GetID() ? this->Internal->SliceCompositeNode->GetID() : "(0 ID)") << "\n";
    this->Internal->SliceCompositeNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "SliceCompositeNode: (none)\n";
    }

  for (unsigned int layerIndex = 0;
       layerIndex < this->Internal->Layers.size();
       ++layerIndex)
    {
    std::string layerName = vtkInternal::GetLayerName(layerIndex);
    vtkMRMLSliceLayerLogic * layer = this->Internal->Layers.at(layerIndex);
    if (!layer)
      {
      os << indent << layerName << ": (none)\n";
      continue;
      }
    os << indent << layerName << ": ";
    layer->PrintSelf(os, nextIndent);
    }
  os << indent << "Pipeline\n";
  this->Internal->PrintPipeline(this->Internal->Pipeline, os, nextIndent);
  os << indent << "PipelineUVW\n";
  this->Internal->PrintPipeline(this->Internal->PipelineUVW, os, nextIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::DeleteSliceModel()
{
  // Remove References
  if (this->Internal->SliceModelNode != 0)
    {
    this->Internal->SliceModelNode->SetAndObserveDisplayNodeID(0);
    this->Internal->SliceModelNode->SetAndObserveTransformNodeID(0);
    this->Internal->SliceModelNode->SetAndObservePolyData(0);
    }
  if (this->Internal->SliceModelDisplayNode != 0)
    {
    this->Internal->SliceModelDisplayNode->SetAndObserveTextureImageData(0);
    }

  // Remove Nodes
  if (this->Internal->SliceModelNode != 0)
    {
    if (this->GetMRMLScene()
        && this->GetMRMLScene()->IsNodePresent(this->Internal->SliceModelNode))
      {
      this->GetMRMLScene()->RemoveNode(this->Internal->SliceModelNode);
      }
    this->Internal->SliceModelNode->Delete();
    this->Internal->SliceModelNode = 0;
    }
  if (this->Internal->SliceModelDisplayNode != 0)
    {
    if (this->GetMRMLScene()
        && this->GetMRMLScene()->IsNodePresent(this->Internal->SliceModelDisplayNode))
      {
      this->GetMRMLScene()->RemoveNode(this->Internal->SliceModelDisplayNode);
      }
    this->Internal->SliceModelDisplayNode->Delete();
    this->Internal->SliceModelDisplayNode = 0;
    }
  if (this->Internal->SliceModelTransformNode != 0)
    {
    if (this->GetMRMLScene()
        && this->GetMRMLScene()->IsNodePresent(this->Internal->SliceModelTransformNode))
      {
      this->GetMRMLScene()->RemoveNode(this->Internal->SliceModelTransformNode);
      }
    this->Internal->SliceModelTransformNode->Delete();
    this->Internal->SliceModelTransformNode = 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::CreateSliceModel()
{
  if(!this->GetMRMLScene())
    {
    return;
    }

  if (this->Internal->SliceModelNode != 0 &&
      this->GetMRMLScene()->GetNodeByID(this->GetSliceModelNode()->GetID()) == 0 )
    {
    this->DeleteSliceModel();
    }

  if ( this->Internal->SliceModelNode == 0)
    {
    this->Internal->SliceModelNode = vtkMRMLModelNode::New();
    this->Internal->SliceModelNode->SetScene(this->GetMRMLScene());
    this->Internal->SliceModelNode->SetDisableModifiedEvent(1);

    this->Internal->SliceModelNode->SetHideFromEditors(1);
    this->Internal->SliceModelNode->SetSelectable(0);
    this->Internal->SliceModelNode->SetSaveWithScene(0);

    // create plane slice
    vtkNew<vtkPlaneSource> planeSource;
    planeSource->GetOutput()->Update();
    this->Internal->SliceModelNode->SetAndObservePolyData(planeSource->GetOutput());
    this->Internal->SliceModelNode->SetDisableModifiedEvent(0);

    // create display node and set texture
    this->Internal->SliceModelDisplayNode = vtkMRMLModelDisplayNode::New();
    this->Internal->SliceModelDisplayNode->SetScene(this->GetMRMLScene());
    this->Internal->SliceModelDisplayNode->SetDisableModifiedEvent(1);

    //this->Internal->SliceModelDisplayNode->SetInputPolyData(this->Internal->SliceModelNode->GetOutputPolyData());
    this->Internal->SliceModelDisplayNode->SetVisibility(0);
    this->Internal->SliceModelDisplayNode->SetOpacity(1);
    this->Internal->SliceModelDisplayNode->SetColor(1,1,1);
    if (this->Internal->SliceNode)
      {
      // Auto-set the colors based on the slice node
      this->Internal->SliceModelDisplayNode->SetColor(this->Internal->SliceNode->GetLayoutColor());
      }
    this->Internal->SliceModelDisplayNode->SetAmbient(1);
    this->Internal->SliceModelDisplayNode->SetBackfaceCulling(0);
    this->Internal->SliceModelDisplayNode->SetDiffuse(0);
    this->Internal->SliceModelDisplayNode->SetAndObserveTextureImageData(this->Internal->ExtractModelTexture->GetOutput());
    this->Internal->SliceModelDisplayNode->SetSaveWithScene(0);
    this->Internal->SliceModelDisplayNode->SetDisableModifiedEvent(0);

    // Turn slice intersection off by default - there is a higher level GUI control
    // in the SliceCompositeNode that tells if slices should be enabled for a given
    // slice viewer
    this->Internal->SliceModelDisplayNode->SetSliceIntersectionVisibility(0);

    std::string name = std::string(this->GetName()) + std::string(" ") + this->SLICE_MODEL_NODE_NAME_SUFFIX;
    this->Internal->SliceModelNode->SetName (name.c_str());

    // make the xy to RAS transform
    this->Internal->SliceModelTransformNode = vtkMRMLLinearTransformNode::New();
    this->Internal->SliceModelTransformNode->SetScene(this->GetMRMLScene());
    this->Internal->SliceModelTransformNode->SetDisableModifiedEvent(1);

    this->Internal->SliceModelTransformNode->SetHideFromEditors(1);
    this->Internal->SliceModelTransformNode->SetSelectable(0);
    this->Internal->SliceModelTransformNode->SetSaveWithScene(0);
    // set the transform for the slice model for use by an image actor in the viewer
    vtkNew<vtkMatrix4x4> identity;
    identity->Identity();
    this->Internal->SliceModelTransformNode->SetMatrixTransformToParent(identity.GetPointer());

    this->Internal->SliceModelTransformNode->SetDisableModifiedEvent(0);

    }

  if (this->Internal->SliceModelNode != 0 && this->GetMRMLScene()->GetNodeByID( this->GetSliceModelNode()->GetID() ) == 0 )
    {
    this->GetMRMLScene()->AddNode(this->Internal->SliceModelDisplayNode);
    this->GetMRMLScene()->AddNode(this->Internal->SliceModelTransformNode);
    this->GetMRMLScene()->AddNode(this->Internal->SliceModelNode);
    this->Internal->SliceModelNode->SetAndObserveDisplayNodeID(this->Internal->SliceModelDisplayNode->GetID());
    this->Internal->SliceModelDisplayNode->SetAndObserveTextureImageData(this->Internal->ExtractModelTexture->GetOutput());
    this->Internal->SliceModelNode->SetAndObserveTransformNodeID(this->Internal->SliceModelTransformNode->GetID());
    }

  // update the description to refer back to the slice and composite nodes
  // TODO: this doesn't need to be done unless the ID change, but it needs
  // to happen after they have been set, so do it every event for now
  if ( this->Internal->SliceModelNode != 0 )
    {
    char description[256];
    std::stringstream ssD;
    vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
    if ( sliceNode && sliceNode->GetID() )
      {
      ssD << " SliceID " << sliceNode->GetID();
      }
    vtkMRMLSliceCompositeNode *compositeNode = this->GetSliceCompositeNode();
    if ( compositeNode && compositeNode->GetID() )
      {
      ssD << " CompositeID " << compositeNode->GetID();
      }

    ssD.getline(description,256);
    this->Internal->SliceModelNode->SetDescription(description);
    }
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetVolumeRASBox(vtkMRMLVolumeNode *volumeNode, double rasDimensions[3], double rasCenter[3])
{
  rasCenter[0] = rasDimensions[0] = 0.0;
  rasCenter[1] = rasDimensions[1] = 0.0;
  rasCenter[2] = rasDimensions[2] = 0.0;

  if (!volumeNode || ! volumeNode->GetImageData())
    {
    return;
    }

  double bounds[6];
  volumeNode->GetRASBounds(bounds);

  for (int i=0; i<3; i++)
    {
    rasDimensions[i] = bounds[2*i+1] - bounds[2*i];
    rasCenter[i] = 0.5*(bounds[2*i+1] + bounds[2*i]);
  }
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetVolumeSliceDimensions(vtkMRMLVolumeNode *volumeNode, double sliceDimensions[3], double sliceCenter[3])
{
  sliceCenter[0] = sliceDimensions[0] = 0.0;
  sliceCenter[1] = sliceDimensions[1] = 0.0;
  sliceCenter[2] = sliceDimensions[2] = 0.0;

  double sliceBounds[6];

  this->GetVolumeSliceBounds(volumeNode, sliceBounds);

  for (int i=0; i<3; i++)
    {
    sliceDimensions[i] = sliceBounds[2*i+1] - sliceBounds[2*i];
    sliceCenter[i] = 0.5*(sliceBounds[2*i+1] + sliceBounds[2*i]);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetVolumeSliceBounds(vtkMRMLVolumeNode *volumeNode, double sliceBounds[6])
{
  sliceBounds[0] = sliceBounds[1] = 0.0;
  sliceBounds[2] = sliceBounds[3] = 0.0;
  sliceBounds[4] = sliceBounds[5] = 0.0;

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();

  if ( !sliceNode )
    {
    return;
    }

  double rasDimensions[3], rasCenter[3];

  this->GetVolumeRASBox(volumeNode, rasDimensions, rasCenter);

  //
  // figure out how big that volume is on this particular slice plane
  //
  vtkNew<vtkMatrix4x4> rasToSlice;
  rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
  rasToSlice->SetElement(0, 3, 0.0);
  rasToSlice->SetElement(1, 3, 0.0);
  rasToSlice->SetElement(2, 3, 0.0);
  rasToSlice->Invert();

  double minBounds[3], maxBounds[3];
  double rasCorner[4], sliceCorner[4];
  int i,j,k;
  for ( i=0; i<3; i++)
    {
    minBounds[i] = 1.0e10;
    maxBounds[i] = -1.0e10;
    }
  for ( i=-1; i<=1; i=i+2)
    {
    for ( j=-1; j<=1; j=j+2)
      {
      for ( k=-1; k<=1; k=k+2)
        {
        rasCorner[0] = rasCenter[0] + i * rasDimensions[0] / 2.;
        rasCorner[1] = rasCenter[1] + j * rasDimensions[1] / 2.;
        rasCorner[2] = rasCenter[2] + k * rasDimensions[2] / 2.;
        rasCorner[3] = 1.;

        rasToSlice->MultiplyPoint( rasCorner, sliceCorner );

        for (int n=0; n<3; n++) {
          if (sliceCorner[n] < minBounds[n])
            {
            minBounds[n] = sliceCorner[n];
            }
          if (sliceCorner[n] > maxBounds[n])
            {
            maxBounds[n] = sliceCorner[n];
            }
          }
        }
      }
    }

  // ignore homogeneous coordinate
  sliceBounds[0] = minBounds[0];
  sliceBounds[1] = maxBounds[0];
  sliceBounds[2] = minBounds[1];
  sliceBounds[3] = maxBounds[1];
  sliceBounds[4] = minBounds[2];
  sliceBounds[5] = maxBounds[2];
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkMRMLSliceLogic::GetVolumeSliceSpacing(vtkMRMLVolumeNode *volumeNode)
{
  if ( !volumeNode )
    {
    return (this->Internal->SliceSpacing);
    }

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();

  if ( !sliceNode )
    {
    return (this->Internal->SliceSpacing);
    }

  if (sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
    {
    // jvm - should we cache the PrescribedSliceSpacing in SliceSpacing?
    double *pspacing = sliceNode->GetPrescribedSliceSpacing();
    this->Internal->SliceSpacing[0] = pspacing[0];
    this->Internal->SliceSpacing[1] = pspacing[1];
    this->Internal->SliceSpacing[2] = pspacing[2];
    return (pspacing);
    }

  vtkNew<vtkMatrix4x4> ijkToRAS;
  vtkNew<vtkMatrix4x4> rasToSlice;
  vtkNew<vtkMatrix4x4> ijkToSlice;

  volumeNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());

  // Apply the transform, if it exists
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode != 0 )
    {
    if ( transformNode->IsTransformToWorldLinear() )
      {
      vtkNew<vtkMatrix4x4> rasToRAS;
      transformNode->GetMatrixTransformToWorld( rasToRAS.GetPointer() );
      rasToRAS->Invert();
      vtkMatrix4x4::Multiply4x4(rasToRAS.GetPointer(), ijkToRAS.GetPointer(), ijkToRAS.GetPointer());
      }
    }

  rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
  rasToSlice->Invert();

  ijkToSlice->Multiply4x4(rasToSlice.GetPointer(), ijkToRAS.GetPointer(), ijkToSlice.GetPointer());

  double invector[4] = {1., 1., 1., 0.};
  double spacing[4];
  ijkToSlice->MultiplyPoint(invector, spacing);
  for (int i = 0; i < 3; ++i)
    {
    this->Internal->SliceSpacing[i] = fabs(spacing[i]);
    }

  return (this->Internal->SliceSpacing);
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current volume
void vtkMRMLSliceLogic::FitSliceToVolume(vtkMRMLVolumeNode *volumeNode, int width, int height)
{
  if (!volumeNode || ! volumeNode->GetImageData())
    {
    return;
    }

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }

  double rasDimensions[3], rasCenter[3];
  this->GetVolumeRASBox (volumeNode, rasDimensions, rasCenter);
  double sliceDimensions[3], sliceCenter[3];
  this->GetVolumeSliceDimensions (volumeNode, sliceDimensions, sliceCenter);

  double fitX, fitY, fitZ, displayX, displayY;
  displayX = fitX = fabs(sliceDimensions[0]);
  displayY = fitY = fabs(sliceDimensions[1]);
  fitZ = this->GetVolumeSliceSpacing(volumeNode)[2] * sliceNode->GetDimensions()[2];


  // fit fov to min dimension of window
  double pixelSize;
  if ( height > width )
    {
    pixelSize = fitX / (1.0 * width);
    fitY = pixelSize * height;
    }
  else
    {
    pixelSize = fitY / (1.0 * height);
    fitX = pixelSize * width;
    }

  // if volume is still too big, shrink some more
  if ( displayX > fitX )
    {
    fitY = fitY / ( fitX / (displayX * 1.0) );
    fitX = displayX;
    }
  if ( displayY > fitY )
    {
    fitX = fitX / ( fitY / (displayY * 1.0) );
    fitY = displayY;
    }

  sliceNode->SetFieldOfView(fitX, fitY, fitZ);

  //
  // set the origin to be the center of the volume in RAS
  //
  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(sliceNode->GetSliceToRAS());
  sliceToRAS->SetElement(0, 3, rasCenter[0]);
  sliceToRAS->SetElement(1, 3, rasCenter[1]);
  sliceToRAS->SetElement(2, 3, rasCenter[2]);
  sliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
  sliceNode->SetSliceOrigin(0,0,0);
  //sliceNode->SetSliceOffset(offset);

  //TODO Fit UVW space

  sliceNode->UpdateMatrices( );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundRASBox(double rasDimensions[3], double rasCenter[3])
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer);
  this->GetVolumeRASBox( backgroundNode, rasDimensions, rasCenter );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundSliceDimensions(double sliceDimensions[3], double sliceCenter[3])
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer);
  this->GetVolumeSliceDimensions( backgroundNode, sliceDimensions, sliceCenter );
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkMRMLSliceLogic::GetBackgroundSliceSpacing()
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer);
  return (this->GetVolumeSliceSpacing( backgroundNode ));
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetBackgroundSliceBounds(double sliceBounds[6])
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer);
  this->GetVolumeSliceBounds(backgroundNode, sliceBounds);
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current background volume
void vtkMRMLSliceLogic::FitSliceToBackground(int width, int height)
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer);
  this->FitSliceToVolume( backgroundNode, width, height );
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of all volume layers
void vtkMRMLSliceLogic::FitSliceToAll(int width, int height)
{
  // Use SliceNode dimensions if width and height parameters are omitted
  if (width < 0 || height < 0)
    {
    int* dimensions = this->Internal->SliceNode->GetDimensions();
    width = dimensions ? dimensions[0] : -1;
    height = dimensions ? dimensions[1] : -1;
    }

  if (width < 0 || height < 0)
    {
    vtkErrorMacro(<< __FUNCTION__ << "- Invalid size:" << width
                  << "x" << height);
    return;
    }

  vtkMRMLVolumeNode* volumeNode = vtkInternal::GetFirstNonNullLayerVolumeNode(this->Internal->Layers);
  if (volumeNode)
    {
    this->FitSliceToVolume(volumeNode, width, height);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::FitFOVToBackground(double fov)
{
  vtkMRMLScalarVolumeNode* backgroundNode =
      vtkMRMLScalarVolumeNode::SafeDownCast(this->GetLayerVolumeNode(vtkMRMLSliceCompositeNode::BackgroundLayer));
  vtkImageData *backgroundImage = backgroundNode ? backgroundNode->GetImageData() : 0;
  if (!backgroundImage)
    {
    return;
    }
  // get viewer's width and height. we may be using a LightBox
  // display, so base width and height on renderer0 in the SliceViewer.
  int width = this->Internal->SliceNode->GetDimensions()[0];
  int height = this->Internal->SliceNode->GetDimensions()[1];

  int dimensions[3];
  double rasDimensions[4];
  double doubleDimensions[4];
  vtkNew<vtkMatrix4x4> ijkToRAS;

  // what are the actual dimensions of the imagedata?
  backgroundImage->GetDimensions(dimensions);
  doubleDimensions[0] = static_cast<double>(dimensions[0]);
  doubleDimensions[1] = static_cast<double>(dimensions[1]);
  doubleDimensions[2] = static_cast<double>(dimensions[2]);
  doubleDimensions[3] = 0.0;
  backgroundNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  ijkToRAS->MultiplyPoint(doubleDimensions, rasDimensions);

  // and what are their slice dimensions?
  vtkNew<vtkMatrix4x4> rasToSlice;
  double sliceDimensions[4];
  rasToSlice->DeepCopy(this->Internal->SliceNode->GetSliceToRAS());
  rasToSlice->SetElement(0, 3, 0.0);
  rasToSlice->SetElement(1, 3, 0.0);
  rasToSlice->SetElement(2, 3, 0.0);
  rasToSlice->Invert();
  rasToSlice->MultiplyPoint(rasDimensions, sliceDimensions);

  double fovh, fovv;
  // which is bigger, slice viewer width or height?
  // assign user-specified fov to smaller slice window
  // dimension
  if ( width < height )
    {
    fovh = fov;
    fovv = fov * height/width;
    }
  else
    {
    fovv = fov;
    fovh = fov * width/height;
    }
  // we want to compute the slice dimensions of the
  // user-specified fov (note that the slice node's z field of
  // view is NOT changed)
  this->Internal->SliceNode->SetFieldOfView(fovh, fovv, this->Internal->SliceNode->GetFieldOfView()[2]);

  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->Internal->SliceNode->GetSliceToRAS());
  this->Internal->SliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::ResizeSliceNode(double newWidth, double newHeight)
{
  if (!this->Internal->SliceNode)
    {
    return;
    }

  // New size must be the active slice vtkRenderer size. It's the same than the window
  // if the layout is 1x1.
  newWidth /= this->Internal->SliceNode->GetLayoutGridColumns();
  newHeight /= this->Internal->SliceNode->GetLayoutGridRows();

  // The following was previously in SliceSWidget.tcl
  double sliceStep = this->Internal->SliceSpacing[2];
  int oldDimensions[3];
  this->Internal->SliceNode->GetDimensions(oldDimensions);
  double oldFOV[3];
  this->Internal->SliceNode->GetFieldOfView(oldFOV);

  double scalingX = (newWidth != 0 && oldDimensions[0] != 0 ? newWidth / oldDimensions[0] : 1.);
  double scalingY = (newHeight != 0 && oldDimensions[1] != 0 ? newHeight / oldDimensions[1] : 1.);

  double magnitudeX = (scalingX >= 1. ? scalingX : 1. / scalingX);
  double magnitudeY = (scalingY >= 1. ? scalingY : 1. / scalingY);

  double newFOV[3];
  if (magnitudeX < magnitudeY)
    {
    newFOV[0] = oldFOV[0];
    newFOV[1] = oldFOV[1] * scalingY / scalingX;
    }
  else
    {
    newFOV[0] = oldFOV[0] * scalingX / scalingY;
    newFOV[1] = oldFOV[1];
    }
  newFOV[2] = sliceStep * oldDimensions[2];
  double windowAspect = (newWidth != 0. ? newHeight / newWidth : 1.);
  double planeAspect = (newFOV[0] != 0. ? newFOV[1] / newFOV[0] : 1.);
  if (windowAspect != planeAspect)
    {
    newFOV[0] = (windowAspect != 0. ? newFOV[1] / windowAspect : newFOV[0]);
    }
  int disabled = this->Internal->SliceNode->StartModify();
  this->Internal->SliceNode->SetDimensions(newWidth, newHeight, oldDimensions[2]);
  this->Internal->SliceNode->SetFieldOfView(newFOV[0], newFOV[1], newFOV[2]);
  this->Internal->SliceNode->EndModify(disabled);
}

//----------------------------------------------------------------------------
double *vtkMRMLSliceLogic::GetLowestVolumeSliceSpacing()
{
  // TBD: Doesn't return the lowest slice spacing, just the first valid spacing
  vtkMRMLVolumeNode* volumeNode = vtkInternal::GetFirstNonNullLayerVolumeNode(this->Internal->Layers);
  if (volumeNode)
    {
    return this->GetVolumeSliceSpacing(volumeNode);
    }
  return this->Internal->SliceSpacing;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetLowestVolumeSliceBounds(double sliceBounds[6])
{
  vtkMRMLVolumeNode* volumeNode = vtkInternal::GetFirstNonNullLayerVolumeNode(this->Internal->Layers);
  if (volumeNode)
    {
    return this->GetVolumeSliceBounds(volumeNode, sliceBounds);
    }
  // return the default values
  return this->GetVolumeSliceBounds(0, sliceBounds);
}

#define LARGE_BOUNDS_NUM 1.0e10
#define SMALL_BOUNDS_NUM -1.0e10
//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetSliceBounds(double sliceBounds[6])
{
  for (unsigned int i=0; i < 3; i++)
    {
    sliceBounds[2*i]   = LARGE_BOUNDS_NUM;
    sliceBounds[2*i+1] = SMALL_BOUNDS_NUM;
    }

  vtkMRMLVolumeNode* volumeNode = vtkInternal::GetFirstNonNullLayerVolumeNode(this->Internal->Layers);
  if (volumeNode)
    {
    double bounds[6];
    this->GetVolumeSliceBounds( volumeNode, bounds );
    for (unsigned int i=0; i < 3; i++)
      {
      if (bounds[2*i] < sliceBounds[2*i])
        {
        sliceBounds[2*i] = bounds[2*i];
        }
      if (bounds[2*i+1] > sliceBounds[2*i+1])
        {
        sliceBounds[2*i+1] = bounds[2*i+1];
        }
      }
    }

  // default
  for (unsigned int i=0; i < 3; i++)
    {
    if (sliceBounds[2*i] == LARGE_BOUNDS_NUM)
      {
      sliceBounds[2*i] = -100;
      }
    if (sliceBounds[2*i+1] == SMALL_BOUNDS_NUM)
      {
      sliceBounds[2*i+1] = 100;
      }
    }
}

//----------------------------------------------------------------------------
// Get/Set the current distance from the origin to the slice plane
double vtkMRMLSliceLogic::GetSliceOffset()
{
  // this method has been moved to vtkMRMLSliceNode
  // the API stays for backwards compatibility

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if ( !sliceNode )
    {
    return 0.0;
    }
  return sliceNode->GetSliceOffset();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceOffset(double offset)
{
  // this method has been moved to vtkMRMLSliceNode
  // the API stays for backwards compatibility

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if ( !sliceNode )
    {
    return;
    }
  sliceNode->SetSliceOffset(offset);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceCompositeNodeInteraction(unsigned int parameters)
{
  vtkMRMLSliceCompositeNode *compositeNode = this->GetSliceCompositeNode();

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on HotLinkedControl and LinkedControl
  compositeNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if (compositeNode &&
      compositeNode->GetHotLinkedControl() && compositeNode->GetLinkedControl())
    {
    if (compositeNode)
      {
      compositeNode->InteractingOn();
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::EndSliceCompositeNodeInteraction()
{
  vtkMRMLSliceCompositeNode *compositeNode = this->GetSliceCompositeNode();

  // If we have linked controls, then we want to broadcast changes
  if (compositeNode && compositeNode->GetLinkedControl())
    {
    if (compositeNode)
      {
      // Need to trigger a final message to broadcast to all the nodes
      // that are linked
      compositeNode->InteractingOn();
      compositeNode->Modified();
      compositeNode->InteractingOff();
      compositeNode->SetInteractionFlags(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceNodeInteraction(unsigned int parameters)
{
  if (this->Internal->SliceNode == NULL || this->Internal->SliceCompositeNode == NULL)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on HotLinkedControl and LinkedControl
  this->Internal->SliceNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if ((this->Internal->SliceCompositeNode->GetHotLinkedControl() || parameters == vtkMRMLSliceNode::MultiplanarReformatFlag)
      && this->Internal->SliceCompositeNode->GetLinkedControl())
    {
    this->Internal->SliceNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceExtentsToSliceNode()
{
  if (this->Internal->SliceNode == NULL)
    {
    return;
    }

  double sliceBounds[6];
  this->GetSliceBounds( sliceBounds );

  double extents[3];
  extents[0] = sliceBounds[1] - sliceBounds[0];
  extents[1] = sliceBounds[3] - sliceBounds[2];
  extents[2] = sliceBounds[5] - sliceBounds[4];

  if (this->Internal->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
    {
    this->Internal->SliceNode->SetUVWExtentsAndDimensions(this->Internal->SliceNode->GetFieldOfView(),
                                                this->Internal->SliceNode->GetUVWDimensions());
    }
 else if (this->Internal->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatchVolumes)
    {
    // TODO: the GetLowestVolumeSliceSpacing currently returns spacing not lowest spacing
    double *spacing = this->GetLowestVolumeSliceSpacing();
    double minSpacing = spacing[0];
    minSpacing = minSpacing < spacing[1] ? minSpacing:spacing[1];
    minSpacing = minSpacing < spacing[2] ? minSpacing:spacing[2];

    int sliceResolutionMax = 200;
    if (minSpacing > 0.0)
      {
      double maxExtent = extents[0];
      maxExtent = maxExtent > extents[1] ? maxExtent:extents[1];
      maxExtent = maxExtent > extents[2] ? maxExtent:extents[2];

      sliceResolutionMax = maxExtent/minSpacing;
      }
    int dimensions[]={sliceResolutionMax, sliceResolutionMax, 1};

    this->Internal->SliceNode->SetUVWExtentsAndDimensions(extents, dimensions);
    }
  else if (this->Internal->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceFOVMatch2DViewSpacingMatchVolumes)
    {
    // TODO: the GetLowestVolumeSliceSpacing currently returns spacing not lowest spacing
    double *spacing = this->GetLowestVolumeSliceSpacing();
    double minSpacing = spacing[0];
    minSpacing = minSpacing < spacing[1] ? minSpacing:spacing[1];
    minSpacing = minSpacing < spacing[2] ? minSpacing:spacing[2];

    double fov[3];
    int dimensions[]={0,0,1};
    this->Internal->SliceNode->GetFieldOfView(fov);
    for (int i=0; i<2; i++)
      {
       dimensions[i] = ceil(fov[i]/minSpacing +0.5);
      }
    this->Internal->SliceNode->SetUVWExtentsAndDimensions(fov, dimensions);
    }
  else if (this->Internal->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceFOVMatchVolumesSpacingMatch2DView)
    {
    // compute RAS spacing in 2D view
    vtkMatrix4x4 *xyToRAS = this->Internal->SliceNode->GetXYToRAS();
    int  dims[3];

    //
    double inPt[4]={0,0,0,1};
    double outPt0[4];
    double outPt1[4];
    double outPt2[4];

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->Internal->SliceNode->GetActiveSlice();

    // transform XYZ = (0,0,0)
    xyToRAS->MultiplyPoint(inPt, outPt0);

    // transform XYZ = (1,0,0)
    inPt[0] = 1;
    xyToRAS->MultiplyPoint(inPt, outPt1);

    // transform XYZ = (0,1,0)
    inPt[0] = 0;
    inPt[1] = 1;
    xyToRAS->MultiplyPoint(inPt, outPt2);

    double xSpacing = sqrt(vtkMath::Distance2BetweenPoints(outPt0, outPt1));
    double ySpacing = sqrt(vtkMath::Distance2BetweenPoints(outPt0, outPt2));

    dims[0] = extents[0]/xSpacing+1;
    dims[1] = extents[2]/ySpacing+1;
    dims[2] = 1;

    this->Internal->SliceNode->SetUVWExtentsAndDimensions(extents, dims);
    }

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::EndSliceNodeInteraction()
{
  if (this->Internal->SliceNode == NULL || this->Internal->SliceCompositeNode == NULL)
    {
    return;
    }

  // If we have linked controls, then we want to broadcast changes
  if (this->Internal->SliceCompositeNode->GetLinkedControl())
    {
    // Need to trigger a final message to broadcast to all the nodes
    // that are linked
    this->Internal->SliceNode->InteractingOn();
    this->Internal->SliceNode->Modified();
    this->Internal->SliceNode->InteractingOff();
    this->Internal->SliceNode->SetInteractionFlags(0);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceOffsetInteraction()
{
  // This method is here in case we want to do something specific when
  // we start SliceOffset interactions

  this->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceToRASFlag);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::EndSliceOffsetInteraction()
{
  // This method is here in case we want to do something specific when
  // we complete SliceOffset interactions

  this->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SnapSliceOffsetToIJK()
{
  double offset, *spacing, bounds[6];
  double oldOffset = this->GetSliceOffset();
  spacing = this->GetLowestVolumeSliceSpacing();
  this->GetLowestVolumeSliceBounds( bounds );

  // number of slices along the offset dimension (depends on ijkToRAS and Transforms)
  // - find the slice index corresponding to the current slice offset
  // - move the offset to the middle of that slice
  // - note that bounds[4] 'furthest' edge of the volume from the point of view of this slice
  // - note also that spacing[2] may correspond to i, j, or k depending on ijkToRAS and sliceToRAS
  double slice = (oldOffset - bounds[4]) / spacing[2];
  int intSlice = static_cast<int> (slice);
  offset = (intSlice + 0.5) * spacing[2] + bounds[4];
  this->SetSliceOffset( offset );
}


//----------------------------------------------------------------------------
std::vector< vtkMRMLDisplayNode*> vtkMRMLSliceLogic::GetPolyDataDisplayNodes()
{
  std::vector< vtkMRMLDisplayNode*> nodes;
  std::vector<vtkMRMLSliceLayerLogic *> layerLogics;
  layerLogics.push_back(this->GetBackgroundLayer());
  layerLogics.push_back(this->GetForegroundLayer());
  for (unsigned int i=0; i<layerLogics.size(); i++)
    {
    vtkMRMLSliceLayerLogic *layerLogic = layerLogics[i];
    if (layerLogic && layerLogic->GetVolumeNode())
      {
      vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast (layerLogic->GetVolumeNode());
      vtkMRMLGlyphableVolumeDisplayNode *displayNode = vtkMRMLGlyphableVolumeDisplayNode::SafeDownCast( layerLogic->GetVolumeNode()->GetDisplayNode() );
      if (displayNode)
        {
        std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> dnodes  = displayNode->GetSliceGlyphDisplayNodes(volumeNode);
        for (unsigned int n=0; n<dnodes.size(); n++)
          {
          vtkMRMLGlyphableVolumeSliceDisplayNode* dnode = dnodes[n];
          if (layerLogic->GetSliceNode()
            && layerLogic->GetSliceNode()->GetLayoutName()
            && !strcmp(layerLogic->GetSliceNode()->GetLayoutName(), dnode->GetName()) )
            {
            nodes.push_back(dnode);
            }
          }
        }//  if (volumeNode)
      }// if (layerLogic && layerLogic->GetVolumeNode())
    }
  return nodes;
}

//----------------------------------------------------------------------------
int vtkMRMLSliceLogic::GetSliceIndexFromOffset(double sliceOffset, vtkMRMLVolumeNode *volumeNode)
{
  if (!volumeNode || ! volumeNode->GetImageData())
    {
    return SLICE_INDEX_NO_VOLUME;
    }
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if ( !sliceNode )
    {
    return SLICE_INDEX_NO_VOLUME;
    }

  vtkNew<vtkMatrix4x4> ijkToRAS;
  volumeNode->GetIJKToRASMatrix (ijkToRAS.GetPointer());
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode )
    {
    vtkNew<vtkMatrix4x4> rasToRAS;
    transformNode->GetMatrixTransformToWorld(rasToRAS.GetPointer());
    vtkMatrix4x4::Multiply4x4 (rasToRAS.GetPointer(), ijkToRAS.GetPointer(), ijkToRAS.GetPointer());
    }

  // Get the slice normal in RAS

  vtkNew<vtkMatrix4x4> rasToSlice;
  rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
  rasToSlice->Invert();

  double sliceNormal_IJK[4]={0,0,1,0};  // slice normal vector in IJK coordinate system
  double sliceNormal_RAS[4]={0,0,0,0};  // slice normal vector in RAS coordinate system
  sliceNode->GetSliceToRAS()->MultiplyPoint(sliceNormal_IJK, sliceNormal_RAS);

  // Find an axis normal that has the same orientation as the slice normal
  double axisDirection_RAS[3]={0,0,0};
  int axisIndex=0;
  double volumeSpacing=1.0; // spacing along axisIndex
  for (axisIndex=0; axisIndex<3; axisIndex++)
    {
    axisDirection_RAS[0]=ijkToRAS->GetElement(0,axisIndex);
    axisDirection_RAS[1]=ijkToRAS->GetElement(1,axisIndex);
    axisDirection_RAS[2]=ijkToRAS->GetElement(2,axisIndex);
    volumeSpacing=vtkMath::Norm(axisDirection_RAS); // spacing along axisIndex
    vtkMath::Normalize(sliceNormal_RAS);
    vtkMath::Normalize(axisDirection_RAS);
    double dotProd=vtkMath::Dot(sliceNormal_RAS, axisDirection_RAS);
    // Due to numerical inaccuracies the dot product of two normalized vectors
    // can be slightly bigger than 1 (and acos cannot be computed) - fix that.
    if (dotProd>1.0)
      {
      dotProd=1.0;
      }
    else if (dotProd<-1.0)
      {
      dotProd=-1.0;
      }
    double axisMisalignmentDegrees=acos(dotProd)*180.0/vtkMath::Pi();
    if (fabs(axisMisalignmentDegrees)<0.1)
      {
      // found an axis that is aligned to the slice normal
      break;
      }
    if (fabs(axisMisalignmentDegrees-180)<0.1 || fabs(axisMisalignmentDegrees+180)<0.1)
      {
      // found an axis that is aligned to the slice normal, just points to the opposite direction
      volumeSpacing*=-1.0;
      break;
      }
    }

  if (axisIndex>=3)
    {
    // no aligned axis is found
    return SLICE_INDEX_ROTATED;
    }

  // Determine slice index
  double originPos_RAS[4]={
    ijkToRAS->GetElement( 0, 3 ),
    ijkToRAS->GetElement( 1, 3 ),
    ijkToRAS->GetElement( 2, 3 ),
    0};
  double originPos_Slice[4]={0,0,0,0};
  rasToSlice->MultiplyPoint(originPos_RAS, originPos_Slice);
  double volumeOriginOffset=originPos_Slice[2];
  double sliceShift=sliceOffset-volumeOriginOffset;
  double normalizedSliceShift=sliceShift/volumeSpacing;
  int sliceIndex=vtkMath::Round(normalizedSliceShift)+1; // +0.5 because the slice plane is displayed in the center of the slice

  // Check if slice index is within the volume
  int sliceCount = volumeNode->GetImageData()->GetDimensions()[axisIndex];
  if (sliceIndex<1 || sliceIndex>sliceCount)
    {
    sliceIndex=SLICE_INDEX_OUT_OF_VOLUME;
    }

  return sliceIndex;
}

//----------------------------------------------------------------------------
// sliceIndex: DICOM slice index, 1-based
int vtkMRMLSliceLogic::GetSliceIndexFromOffset(double sliceOffset)
{
  vtkMRMLVolumeNode* volumeNode = vtkInternal::GetFirstNonNullLayerVolumeNode(this->Internal->Layers);
  if (volumeNode)
    {
    return this->GetSliceIndexFromOffset(sliceOffset, volumeNode);
    }
  // slice is not aligned to any of the layers or out of the volume
  return SLICE_INDEX_NO_VOLUME;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceLogic
::GetSliceCompositeNode(vtkMRMLSliceNode* sliceNode)
{
  return sliceNode ? vtkMRMLSliceLogic::GetSliceCompositeNode(
    sliceNode->GetScene(), sliceNode->GetLayoutName()) : 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceLogic
::GetSliceCompositeNode(vtkMRMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return 0;
    }
  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  for (scene->GetNodes()->InitTraversal(it);
       (node = (vtkMRMLNode*)scene->GetNodes()->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLSliceCompositeNode* sliceCompositeNode =
      vtkMRMLSliceCompositeNode::SafeDownCast(node);
    if (sliceCompositeNode &&
        sliceCompositeNode->GetLayoutName() &&
        !strcmp(sliceCompositeNode->GetLayoutName(), layoutName))
      {
      return sliceCompositeNode;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsSliceModelNode(vtkMRMLNode *mrmlNode)
{
  if (mrmlNode != NULL &&
      mrmlNode->IsA("vtkMRMLModelNode") &&
      mrmlNode->GetName() != NULL &&
      strstr(mrmlNode->GetName(), vtkMRMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX.c_str()) != NULL)
    {
    return true;
    }
  return false;
}
