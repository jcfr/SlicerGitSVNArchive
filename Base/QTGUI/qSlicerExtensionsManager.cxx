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

// QtGUI includes
#include "qSlicerCoreApplication.h"
#include "qSlicerExtensionsManager.h"
#include "qSlicerExtensionsManagerModel.h"
#include "ui_qSlicerExtensionsManager.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsManagerPrivate: public Ui_qSlicerExtensionsManager
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManager);
protected:
  qSlicerExtensionsManager* const q_ptr;

public:
  qSlicerExtensionsManagerPrivate(qSlicerExtensionsManager& object);
  void init();
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerPrivate::qSlicerExtensionsManagerPrivate(qSlicerExtensionsManager& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerPrivate::init()
{
  Q_Q(qSlicerExtensionsManager);

  this->setupUi(q);

  q->setExtensionsManagerModel(new qSlicerExtensionsManagerModel(q));
  qSlicerExtensionsManagerModel * model = q->extensionsManagerModel();

  QObject::connect(model, SIGNAL(modelUpdated()),
                   q, SLOT(onModelUpdated()));
  QObject::connect(model, SIGNAL(extensionInstalled(QString)),
                   q, SLOT(onModelUpdated()));
  QObject::connect(model, SIGNAL(extensionUninstalled(QString)),
                   q, SLOT(onModelUpdated()));

  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  model->setLauncherSettingsFilePath(coreApp->launcherSettingsFilePath());
  model->setSlicerRequirements(coreApp->repositoryRevision(), coreApp->os(), coreApp->arch());

  model->updateModel();
}

// --------------------------------------------------------------------------
// qSlicerExtensionsManager methods

// --------------------------------------------------------------------------
qSlicerExtensionsManager::qSlicerExtensionsManager(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsManagerPrivate(*this))
{
  Q_D(qSlicerExtensionsManager);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsManager::~qSlicerExtensionsManager()
{
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsManager::extensionsManagerModel()const
{
  Q_D(const qSlicerExtensionsManager);
  return d->ExtensionsManageWidget->extensionsManagerModel();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManager::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsManager);
  d->ExtensionsManageWidget->setExtensionsManagerModel(model);
  d->ExtensionsInstallWidget->setExtensionsManagerModel(model);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManager::onModelUpdated()
{
  Q_D(qSlicerExtensionsManager);

  int manageExtensionsTabIndex = d->tabWidget->indexOf(d->ManageExtensionsTab);
  int numberOfInstalledExtensions = this->extensionsManagerModel()->numberOfInstalledExtensions();

  d->tabWidget->setTabText(manageExtensionsTabIndex,
                           QString("Manage Extensions (%1)").arg(numberOfInstalledExtensions));

  if (numberOfInstalledExtensions == 0)
    {
    d->tabWidget->setTabEnabled(manageExtensionsTabIndex, false);
    d->tabWidget->setCurrentWidget(d->InstallExtensionsTab);
    }
  else
    {
    d->tabWidget->setTabEnabled(manageExtensionsTabIndex, true);
    }
}
