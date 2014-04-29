/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSliceCompositeNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
static const char* LAYER_VOLUME_REFERENCE_ROLE = "layerVolumeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSliceCompositeNode)

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::vtkMRMLSliceCompositeNode()
{
  this->HideFromEditors = 1;

  this->Compositing = 0;
  this->LinkedControl = 0;
  this->FiducialVisibility = 1;
  this->FiducialLabelVisibility = 1;
  this->AnnotationSpace = vtkMRMLSliceCompositeNode::IJKAndRAS;
  this->AnnotationMode = vtkMRMLSliceCompositeNode::All;
  this->SliceIntersectionVisibility = 0;
  this->DoPropagateVolumeSelection = true;
  this->Interacting = 0;
  this->InteractionFlags = 0;
  this->HotLinkedControl = 0;
  this->InteractionFlagsModifier = (unsigned int) -1;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::~vtkMRMLSliceCompositeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " compositing=\"" << this->Compositing << "\"";
  of << indent << " foregroundOpacity=\"" << this->GetLayerOpacity(Self::ForegroundLayer) << "\"";
  of << indent << " labelOpacity=\"" << this->GetLayerOpacity(Self::LabelLayer) << "\"";
  of << indent << " linkedControl=\"" << this->LinkedControl << "\"";
  of << indent << " fiducialVisibility=\"" << this->FiducialVisibility << "\"";
  of << indent << " fiducialLabelVisibility=\"" << this->FiducialLabelVisibility << "\"";
  of << indent << " sliceIntersectionVisibility=\"" << this->SliceIntersectionVisibility << "\"";
  if (this->GetLayoutName() != NULL)
    {
    of << indent << " layoutName=\"" << this->GetLayoutName() << "\"";
    }

  if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::XYZ)
    {
    of << indent << " annotationSpace=\"" << "xyz" << "\"";
    }
  else if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::IJK)
    {
    of << indent << " annotationSpace=\"" << "ijk" << "\"";
    }
  else if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::RAS)
    {
    of << indent << " annotationSpace=\"" << "RAS" << "\"";
    }
  else if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::IJKAndRAS)
    {
    of << indent << " annotationSpace=\"" << "IJKAndRAS" << "\"";
    }

  if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::NoAnnotation )
    {
    of << indent << " annotationMode=\"" << "NoAnnotation" << "\"";
    }
  else if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::All )
    {
    of << indent << " annotationMode=\"" << "All" << "\"";
    }
  if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::LabelValuesOnly )
    {
    of << indent << " annotationMode=\"" << "LabelValuesOnly" << "\"";
    }
  if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly )
    {
    of << indent << " annotationMode=\"" << "LabelAndVoxelValuesOnly" << "\"";
    }
  of << indent << " doPropagateVolumeSelection=\"" << (int)this->DoPropagateVolumeSelection << "\"";
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteractionFlagsModifier(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlagsModifier = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::ResetInteractionFlagsModifier()
{
  // Don't call Modified()
  this->InteractionFlagsModifier = (unsigned int) -1;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "backgroundVolumeID"))
      {
      if (attValue && *attValue == '\0')
        {
        this->SetBackgroundVolumeID(NULL);
        }
      else
        {
        this->SetBackgroundVolumeID(attValue);
        }
      }
    else if (!strcmp(attName, "foregroundVolumeID"))
      {
      if (attValue && *attValue == '\0')
        {
        this->SetForegroundVolumeID(NULL);
        }
      else
        {
        this->SetForegroundVolumeID(attValue);
        }
      }
    else if (!strcmp(attName, "labelVolumeID"))
      {
      if (attValue && *attValue == '\0')
        {
        this->SetLabelVolumeID(NULL);
        }
      else
        {
        this->SetLabelVolumeID(attValue);
        }
      }
    else if (!strcmp(attName, "compositing"))
      {
      this->SetCompositing( atoi(attValue) );
      }
    else if (!strcmp(attName, "foregroundOpacity"))
      {
      this->SetForegroundOpacity( atof(attValue) );
      }
    else if (!strcmp(attName, "labelOpacity"))
      {
      this->SetLabelOpacity( atof(attValue) );
      }
    else if (!strcmp(attName, "linkedControl"))
      {
      this->SetLinkedControl( atoi(attValue) );
      }
    else if (!strcmp(attName, "hotLinkedControl"))
      {
      this->SetHotLinkedControl( atoi(attValue) );
      }
    else if (!strcmp(attName, "fiducialVisibility"))
      {
      this->SetFiducialVisibility( atoi(attValue) );
      }
    else if (!strcmp(attName, "fiducialLabelVisibility"))
      {
      this->SetFiducialLabelVisibility( atoi(attValue) );
      }
    else if (!strcmp(attName, "sliceIntersectionVisibility"))
      {
      this->SetSliceIntersectionVisibility( atoi(attValue) );
      }
   else if (!strcmp(attName, "layoutName"))
      {
      this->SetLayoutName( attValue );
      }

    else if(!strcmp (attName, "annotationSpace" ))
      {
      if (!strcmp (attValue, "xyz"))
        {
        this->SetAnnotationSpace (vtkMRMLSliceCompositeNode::XYZ);
        }
      else if (!strcmp (attValue, "ijk"))
        {
        this->SetAnnotationSpace (vtkMRMLSliceCompositeNode::IJK);
        }
      else if (!strcmp (attValue, "RAS"))
        {
        this->SetAnnotationSpace  (vtkMRMLSliceCompositeNode::RAS);
        }
      else if (!strcmp (attValue, "IJKAndRAS"))
        {
        this->SetAnnotationSpace  (vtkMRMLSliceCompositeNode::IJKAndRAS);
        }
      }
    else if(!strcmp (attName, "annotationMode" ))
      {
      if (!strcmp (attValue, "NoAnnotation"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::NoAnnotation);
        }
      else if (!strcmp (attValue, "All"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::All);
        }
      else if (!strcmp (attValue, "LabelValuesOnly"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::LabelValuesOnly);
        }
      else if (!strcmp (attValue, "LabelAndVoxelValuesOnly"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly);
        }
      }
    else if(!strcmp (attName, "doPropagateVolumeSelection" ))
      {
      this->SetDoPropagateVolumeSelection(atoi(attValue)?true:false);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSliceCompositeNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);
  vtkMRMLSliceCompositeNode *node = vtkMRMLSliceCompositeNode::SafeDownCast(anode);

  this->SetCompositing(node->GetCompositing());
  for(unsigned int layerIndex = Self::ForegroundLayer;
      layerIndex < static_cast<unsigned int>(node->GetNumberOfNodeReferences(LAYER_VOLUME_REFERENCE_ROLE));
      ++layerIndex)
    {
    this->SetLayerOpacity(layerIndex, node->GetLayerOpacity(layerIndex));
    }
  this->SetLinkedControl (node->GetLinkedControl());
  this->SetHotLinkedControl (node->GetHotLinkedControl());
  this->SetFiducialVisibility ( node->GetFiducialVisibility ( ) );
  this->SetFiducialLabelVisibility ( node->GetFiducialLabelVisibility ( ) );
  this->SetAnnotationSpace ( node->GetAnnotationSpace() );
  this->SetAnnotationMode ( node->GetAnnotationMode() );
  this->SetDoPropagateVolumeSelection (node->GetDoPropagateVolumeSelection());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Compositing: " << this->Compositing << "\n";
  os << indent << "ForegroundOpacity: " << this->GetLayerOpacity(Self::ForegroundLayer)<< "\n";
  os << indent << "LabelOpacity: " << this->GetLayerOpacity(Self::LabelLayer) << "\n";
  os << indent << "LinkedControl: " << this->LinkedControl << "\n";
  os << indent << "HotLinkedControl: " << this->HotLinkedControl << "\n";
  os << indent << "FiducialVisibility: " << this->FiducialVisibility << "\n";
  os << indent << "FiducialLabelVisibility: " << this->FiducialLabelVisibility << "\n";
  os << indent << "SliceIntersectionVisibility: " << this->SliceIntersectionVisibility << "\n";
  os << indent << "AnnotationSpace: " << this->AnnotationSpace << "\n";
  os << indent << "AnnotationMode: " << this->AnnotationMode << "\n";
  os << indent << "DoPropagateVolumeSelection: " << this->DoPropagateVolumeSelection << "\n";
  os << indent << "Interacting: " <<
    (this->Interacting ? "on" : "off") << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLSliceCompositeNode::GetLayerVolume(unsigned int layerIndex)
{
  return vtkMRMLVolumeNode::SafeDownCast(
        this->GetNthNodeReference(LAYER_VOLUME_REFERENCE_ROLE, layerIndex));
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::
SetLayerVolume(unsigned int layerIndex, vtkMRMLVolumeNode* volumeNode)
{
  char * volumeNodeID = 0;
  if (volumeNode)
    {
    volumeNodeID = volumeNode->GetID();
    }
  this->SetLayerVolumeID(layerIndex, volumeNodeID);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetLayerVolumeID(unsigned int layerIndex)
{
  vtkMRMLVolumeNode* volumeNode = this->GetLayerVolume(layerIndex);
  if (volumeNode)
    {
    return volumeNode->GetID();
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::
SetLayerVolumeID(unsigned int layerIndex, const char* id)
{
  this->SetNthNodeReferenceID(LAYER_VOLUME_REFERENCE_ROLE, layerIndex, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetBackgroundVolumeID()
{
  return this->GetLayerVolumeID(Self::BackgroundLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetBackgroundVolumeID(const char* id)
{
  this->SetLayerVolumeID(Self::BackgroundLayer, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetForegroundVolumeID()
{
  return this->GetLayerVolumeID(Self::ForegroundLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetForegroundVolumeID(const char* id)
{
  this->SetLayerVolumeID(Self::ForegroundLayer, id);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceCompositeNode::GetLabelVolumeID()
{
  return this->GetLayerVolumeID(Self::LabelLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetLabelVolumeID(const char* id)
{
  this->SetLayerVolumeID(Self::LabelLayer, id);
}

//----------------------------------------------------------------------------
int vtkMRMLSliceCompositeNode::GetCompositing()
{
  return this->Compositing;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetCompositing(int value)
{
  if (this->Compositing != value)
    {
    this->Compositing = value;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
double vtkMRMLSliceCompositeNode::GetLayerOpacity(unsigned int layerIndex)
{
  if (layerIndex < this->LayerOpacities.size())
    {
    return this->LayerOpacities.at(layerIndex);
    }
  return 0.0;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetLayerOpacity(unsigned int layerIndex, double value)
{
  if (layerIndex >= this->LayerOpacities.size())
    {
    this->LayerOpacities.resize(layerIndex + 1);
    }
  if (this->LayerOpacities.at(layerIndex) != value)
    {
    this->LayerOpacities.at(layerIndex) = value;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
double vtkMRMLSliceCompositeNode::GetForegroundOpacity()
{
  return this->GetLayerOpacity(Self::ForegroundLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetForegroundOpacity(double value)
{
  this->SetLayerOpacity(Self::ForegroundLayer, value);
}

//----------------------------------------------------------------------------
double vtkMRMLSliceCompositeNode::GetLabelOpacity()
{
  return this->GetLayerOpacity(Self::LabelLayer);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetLabelOpacity(double value)
{
  this->SetLayerOpacity(Self::LabelLayer, value);
}
