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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QFormLayout>
#include <QDebug>

// SlicerQt includes
#include "qSlicerCLIProgressBar.h"

// MRML includes
#include <vtkMRMLCommandLineModuleNode.h>

//-----------------------------------------------------------------------------
// qSlicerCLIProgressBarPrivate methods

//-----------------------------------------------------------------------------
class qSlicerCLIProgressBarPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCLIProgressBar);
protected:
  qSlicerCLIProgressBar* const q_ptr;
public:
  typedef qSlicerCLIProgressBarPrivate Self;
  qSlicerCLIProgressBarPrivate(qSlicerCLIProgressBar& object);

  void init();

  // progressBar(s)
  // label

  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
};

//-----------------------------------------------------------------------------
// qSlicerCLIProgressBarPrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIProgressBarPrivate::qSlicerCLIProgressBarPrivate(qSlicerCLIProgressBar& object)
  :q_ptr(&object)
{
  this->CommandLineModuleNode = 0;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBarPrivate::init()
{
  // Create widget .. layout
}

//-----------------------------------------------------------------------------
// qSlicerCLIProgressBar methods

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::qSlicerCLIProgressBar(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qSlicerCLIProgressBarPrivate(*this))
{
  Q_D(qSlicerCLIProgressBar);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::~qSlicerCLIProgressBar()
{
}

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode * qSlicerCLIProgressBar::commandLineModuleNode()const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->CommandLineModuleNode;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::setCommandLineModuleNode(
  vtkMRMLCommandLineModuleNode* commandLineModuleNode)
{
  Q_D(qSlicerCLIProgressBar);
  if (commandLineModuleNode == d->CommandLineModuleNode)
    {
    return;
    }

  // Connect node modified event to updateUi that synchronize the values of the
  // nodes with the Ui
  this->qvtkReconnect(d->CommandLineModuleNode, commandLineModuleNode,
    vtkCommand::ModifiedEvent,
    this, SLOT(updateUiFromCommandLineModuleNode(vtkObject*)));

  d->CommandLineModuleNode = commandLineModuleNode;
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::updateUiFromCommandLineModuleNode(
  vtkObject* commandLineModuleNode)
{
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  Q_ASSERT(node);
/*
  // Update progress
  this->StatusLabel->setText(node->GetStatusString());
  this->ProgressBar->setVisible(node->GetStatus() != vtkMRMLCommandLineModuleNode::Idle &&
                                node->GetStatus() != vtkMRMLCommandLineModuleNode::Cancelled);
  this->StageProgressBar->setVisible(node->GetStatus() == vtkMRMLCommandLineModuleNode::Running);
  ModuleProcessInformation* info = node->GetModuleDescription().GetProcessInformation();
  switch (node->GetStatus())
    {
    case vtkMRMLCommandLineModuleNode::Cancelled:
      this->CancelPushButton->setEnabled(false);
      this->ProgressBar->setMaximum(0);
      break;
    case vtkMRMLCommandLineModuleNode::Scheduled:
      this->ApplyPushButton->setEnabled(false);
      this->CancelPushButton->setEnabled(true);
      this->ProgressBar->setMaximum(0);
      break;
    case vtkMRMLCommandLineModuleNode::Running:
      this->DefaultPushButton->setEnabled(false);
      this->ApplyPushButton->setEnabled(false);
      this->CancelPushButton->setEnabled(true);
      
      this->ProgressBar->setMaximum(info->Progress != 0.0 ? 100 : 0);
      this->ProgressBar->setValue(info->Progress * 100.);
      if (info->ElapsedTime != 0.)
        {
        this->StatusLabel->setText(QString("%1 (%2)").arg(node->GetStatusString()).arg(info->ElapsedTime));
        }
      this->StageProgressBar->setMaximum(info->StageProgress != 0.0 ? 100 : 0);
      this->StageProgressBar->setFormat(info->ProgressMessage);
      this->StageProgressBar->setValue(info->StageProgress * 100.);
      break;
    case vtkMRMLCommandLineModuleNode::Completed:
    case vtkMRMLCommandLineModuleNode::CompletedWithErrors:
      this->DefaultPushButton->setEnabled(true);
      this->ApplyPushButton->setEnabled(true);
      this->CancelPushButton->setEnabled(false);
      this->ProgressBar->setMaximum(100);
      this->ProgressBar->setValue(100);
      break;
    default:
    case vtkMRMLCommandLineModuleNode::Idle:
      this->DefaultPushButton->setEnabled(true);
      this->ApplyPushButton->setEnabled(true);
      this->CancelPushButton->setEnabled(false);
      break;
    }*/
}

