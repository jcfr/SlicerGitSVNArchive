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
#include <QProcess>
#include <QUrl>

// CTK includes
#include <ctkCmdLineModuleBackendLocalProcess.h>
#include <ctkCmdLineModuleManager.h>

// SlicerQt includes
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"
#include "qSlicerUtils.h"
#ifdef Q_OS_MAC
# include "qSlicerCoreApplication.h"
#endif

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryItem::qSlicerCLIExecutableModuleFactoryItem(
    ctkCmdLineModuleManager* cmdLineModuleManager, const QString& newTempDirectory) :
  CmdLineModuleManager(cmdLineModuleManager), TempDirectory(newTempDirectory)
{
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactoryItem::load()
{
  return true;
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerCLIExecutableModuleFactoryItem::instanciator()
{
  ctkScopedCurrentDir scopedCurrentDir(QFileInfo(this->path()).path());

  // Using a scoped pointer ensures the memory will be cleaned if instantiator
  // fails before returning the module. See QScopedPointer::take()
  QScopedPointer<qSlicerCLIModule> module(new qSlicerCLIModule());
  module->setModuleType("CommandLineModule");
  module->setEntryPoint(this->path());

  module->setCmdLineModuleReference(
        this->CmdLineModuleManager->registerModule(QUrl::fromLocalFile(this->path())));

  module->setTempDirectory(this->TempDirectory);
  module->setPath(this->path());
  module->setInstalled(qSlicerCLIModuleFactoryHelper::isInstalled(this->path()));

  return module.take();
}

//-----------------------------------------------------------------------------
// qSlicerCLIExecutableModuleFactoryPrivate

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCLIExecutableModuleFactory);
protected:
  qSlicerCLIExecutableModuleFactory* const q_ptr;
public:
  typedef qSlicerCLIExecutableModuleFactoryPrivate Self;
  qSlicerCLIExecutableModuleFactoryPrivate(qSlicerCLIExecutableModuleFactory& object);

  void init();

private:
  QString TempDirectory;
  ctkCmdLineModuleBackendLocalProcess CmdLineModuleBackend;
  ctkCmdLineModuleManager * CmdLineModuleManager;
};

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryPrivate::qSlicerCLIExecutableModuleFactoryPrivate(qSlicerCLIExecutableModuleFactory& object)
:q_ptr(&object)
{
  this->TempDirectory = QDir::tempPath();
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactoryPrivate::init()
{
  this->CmdLineModuleManager->registerBackend(&this->CmdLineModuleBackend);
}

//-----------------------------------------------------------------------------
// qSlicerCLIExecutableModuleFactory

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::qSlicerCLIExecutableModuleFactory(
    ctkCmdLineModuleManager * cmdLineModuleManager)
  : d_ptr(new qSlicerCLIExecutableModuleFactoryPrivate(*this))
{
  Q_D(qSlicerCLIExecutableModuleFactory);
  d->CmdLineModuleManager = cmdLineModuleManager;
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::~qSlicerCLIExecutableModuleFactory()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::registerItems()
{
  QStringList modulePaths = qSlicerCLIModuleFactoryHelper::modulePaths();

#ifdef Q_OS_MAC
  // HACK - See CMakeLists.txt for additional details
  if (qSlicerCoreApplication::application()->isInstalled())
    {
    modulePaths.prepend(qSlicerCoreApplication::application()->slicerHome() + "/" Slicer_CLIMODULES_SUBDIR);
    }
#endif
  this->registerAllFileItems(modulePaths);
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactory::isValidFile(const QFileInfo& file)const
{
  if (!this->Superclass::isValidFile(file))
    {
    return false;
    }
  if (!file.isExecutable())
    {
    return false;
    }
  return qSlicerUtils::isCLIExecutable(file.absoluteFilePath());
}

//-----------------------------------------------------------------------------
ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* qSlicerCLIExecutableModuleFactory
::createFactoryFileBasedItem()
{
  Q_D(qSlicerCLIExecutableModuleFactory);
  return new qSlicerCLIExecutableModuleFactoryItem(d->CmdLineModuleManager, d->TempDirectory);
}

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactory::fileNameToKey(const QString& executableName)const
{
  return qSlicerUtils::extractModuleNameFromLibraryName(executableName);
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::setTempDirectory(const QString& newTempDirectory)
{
  Q_D(qSlicerCLIExecutableModuleFactory);
  d->TempDirectory = newTempDirectory;
}
