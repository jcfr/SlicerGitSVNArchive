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

#ifndef __qSlicerExtensionsManager_h
#define __qSlicerExtensionsManager_h

// Qt includes
#include <QWidget>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsManagerPrivate;
class qSlicerExtensionsManagerModel;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsManager
  : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerExtensionsManager(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerExtensionsManager();

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;
  void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

protected slots:
  void onModelUpdated();

protected:
  QScopedPointer<qSlicerExtensionsManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsManager);
  Q_DISABLE_COPY(qSlicerExtensionsManager);
};

#endif
