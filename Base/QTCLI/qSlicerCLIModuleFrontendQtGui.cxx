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

// Slicer includes
#include "qSlicerCLIModuleFrontendQtGui.h"
#include "qSlicerCLIModuleQtXslTransform.h"
#include "qSlicerCLIModuleUiLoader.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
struct qSlicerCLIModuleFrontendQtGuiPrivate
{
  qSlicerCLIModuleFrontendQtGuiPrivate(){}

  void init();

  mutable vtkSmartPointer<vtkMRMLScene> MRMLScene;
  mutable QScopedPointer<qSlicerCLIModuleUiLoader> Loader;
  mutable QScopedPointer<qSlicerCLIModuleQtXslTransform> Transform;
};

//-----------------------------------------------------------------------------
qSlicerCLIModuleFrontendQtGui::qSlicerCLIModuleFrontendQtGui(const ctkCmdLineModuleReference& moduleRef)
  : Superclass(moduleRef), d_ptr(new qSlicerCLIModuleFrontendQtGuiPrivate)
{

}

//-----------------------------------------------------------------------------
qSlicerCLIModuleFrontendQtGui::~qSlicerCLIModuleFrontendQtGui()
{

}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerCLIModuleFrontendQtGui::mrmlScene() const
{
  Q_D(const qSlicerCLIModuleFrontendQtGui);
  return d->MRMLScene;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleFrontendQtGui::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerCLIModuleFrontendQtGui);
  if (scene == d->MRMLScene)
    {
    return ;
    }
  d->MRMLScene = scene;
  emit mrmlSceneChanged(scene);
}

//-----------------------------------------------------------------------------
QUiLoader* qSlicerCLIModuleFrontendQtGui::uiLoader() const
{
  Q_D(const qSlicerCLIModuleFrontendQtGui);
  if (d->Loader == NULL)
    {
    d->Loader.reset(new qSlicerCLIModuleUiLoader());
    d->Loader->setMRMLScene(this->mrmlScene());
    connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
            d->Loader.data(), SLOT(setMRMLScene(vtkMRMLScene*)));
    }
  return d->Loader.data();
}

//-----------------------------------------------------------------------------
ctkCmdLineModuleXslTransform* qSlicerCLIModuleFrontendQtGui::xslTransform() const
{
  Q_D(const qSlicerCLIModuleFrontendQtGui);
  if (d->Transform == NULL)
    {
    d->Transform.reset(new qSlicerCLIModuleQtXslTransform());
    }
  return d->Transform.data();
}

