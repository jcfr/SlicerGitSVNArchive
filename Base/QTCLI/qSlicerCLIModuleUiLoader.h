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

#ifndef __qSlicerCLIModuleUiLoader_h
#define __qSlicerCLIModuleUiLoader_h

// CTK includes
#include <ctkCmdLineModuleQtUiLoader.h>

// Slicer includes
#include "qSlicerBaseQTCLIExport.h"

struct qSlicerCLIModuleUiLoaderPrivate;
class vtkMRMLScene;

/// Derived from ctkCmdLineModuleQtXslTransform to customize the XSL transformer so that it
/// generate a UI file specific to Slicer.
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleUiLoader : public ctkCmdLineModuleQtUiLoader
{
  Q_OBJECT
public:
  typedef ctkCmdLineModuleQtUiLoader Superclass;
  qSlicerCLIModuleUiLoader(QObject* parent = 0);
  virtual ~qSlicerCLIModuleUiLoader();

  /// Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;

  virtual QWidget* createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString() );

public slots:

  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

signals:

  void mrmlSceneChanged(vtkMRMLScene*);

private:

  QScopedPointer<qSlicerCLIModuleUiLoaderPrivate> d_ptr;

  Q_DECLARE_PRIVATE(qSlicerCLIModuleUiLoader)
  Q_DISABLE_COPY(qSlicerCLIModuleUiLoader)

};

#endif
