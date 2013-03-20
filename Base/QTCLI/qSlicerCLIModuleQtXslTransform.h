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

#ifndef __qSlicerCLIModuleQtXslTransform_h
#define __qSlicerCLIModuleQtXslTransform_h

// CTK includes
#include <ctkCmdLineModuleQtXslTransform.h>

// Slicer includes
#include "qSlicerBaseQTCLIExport.h"

struct qSlicerCLIModuleQtXslTransformPrivate;


/// Derived from ctkCmdLineModuleQtXslTransform to customize the XSL transformer so that it
/// generate a UI file specific to Slicer.
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleQtXslTransform : public ctkCmdLineModuleQtXslTransform
{
public:
  typedef ctkCmdLineModuleQtXslTransform Superclass;
  qSlicerCLIModuleQtXslTransform(QIODevice* input = 0, QIODevice* output = 0);
  virtual ~qSlicerCLIModuleQtXslTransform();

private:

  QScopedPointer<qSlicerCLIModuleQtXslTransformPrivate> d_ptr;

  Q_DECLARE_PRIVATE(qSlicerCLIModuleQtXslTransform)
  Q_DISABLE_COPY(qSlicerCLIModuleQtXslTransform)

};

#endif
