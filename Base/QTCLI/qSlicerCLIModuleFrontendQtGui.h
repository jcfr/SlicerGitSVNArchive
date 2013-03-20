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

#ifndef __qSlicerCLIModuleFrontendQtGui_h
#define __qSlicerCLIModuleFrontendQtGui_h

// CTK includes
#include <ctkCmdLineModuleFrontendQtGui.h>
#include <ctkCmdLineModuleReference.h>

#include "qSlicerBaseQTCLIExport.h"

struct qSlicerCLIModuleFrontendQtGuiPrivate;
class vtkMRMLScene;

class qSlicerCLIModuleFrontendQtGui : public ctkCmdLineModuleFrontendQtGui
{
  Q_OBJECT
public:
  typedef ctkCmdLineModuleFrontendQtGui Superclass;
  qSlicerCLIModuleFrontendQtGui(const ctkCmdLineModuleReference& moduleRef);
  virtual ~qSlicerCLIModuleFrontendQtGui();

  /// Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;

public slots:

  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

signals:

  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  virtual QUiLoader* uiLoader() const;
  virtual ctkCmdLineModuleXslTransform* xslTransform() const;

private:
  QScopedPointer<qSlicerCLIModuleFrontendQtGuiPrivate> d_ptr;

  Q_DECLARE_PRIVATE(qSlicerCLIModuleFrontendQtGui);
  Q_DISABLE_COPY(qSlicerCLIModuleFrontendQtGui);
};

#endif

