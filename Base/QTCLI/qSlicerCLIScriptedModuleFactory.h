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

#ifndef __qSlicerCLIScriptedModuleFactory_h
#define __qSlicerCLIScriptedModuleFactory_h

// SlicerQT includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerBaseQTCLIExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractPluginFactory.h>

//-----------------------------------------------------------------------------
class qSlicerCLIScriptedModuleFactoryItem
  : public ctkAbstractFactoryFileBasedItem<qSlicerAbstractCoreModule>
{
public:
  qSlicerCLIScriptedModuleFactoryItem(const QString& newTempDirectory);
  virtual bool load();
protected:
  virtual qSlicerAbstractCoreModule* instanciator();
private:
  QString TempDirectory;
};

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIScriptedModuleFactory :
  public ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule>
{
public:
  typedef ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerCLIScriptedModuleFactory();
  qSlicerCLIScriptedModuleFactory(const QString& tempDir);

  virtual void registerItems();

  /// Extract module name given \a executableName
  /// For example:
  ///  Threshold.exe -> threshold
  ///  Threshold -> threshold
  virtual QString fileNameToKey(const QString& fileName)const;

  void setTempDirectory(const QString& newTempDirectory);

protected:
  virtual bool isValidFile(const QFileInfo& file)const;

  virtual ctkAbstractFactoryItem<qSlicerAbstractCoreModule>*
    createFactoryFileBasedItem();

private:
  QString TempDirectory;
};

#endif
