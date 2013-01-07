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

#include "qSlicerCLIModule.h"

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkWidgetsUtils.h>

// Slicer includes
#include "qMRMLNodeComboBox.h"
#include "qSlicerCLIModuleWidget.h"
#include "vtkSlicerCLIModuleLogic.h"

// SlicerExecutionModel includes
#include <ModuleDescription.h>
#include <ModuleDescriptionParser.h>
#include <ModuleLogo.h>
#include <ModuleProcessInformation.h>

//-----------------------------------------------------------------------------
class qSlicerCLIModulePrivate
{
public:
  typedef qSlicerCLIModulePrivate Self;
  qSlicerCLIModulePrivate();

  QString           Title;
  QString           Acknowledgement;
  QString           Help;
  QStringList       Categories;
  QStringList       Contributors;
  QImage            Logo;
  int               Index;

  QString           EntryPoint;
  QString           ModuleType;
  QString           TempDirectory;

  ModuleDescription                 Desc;
  ModuleProcessInformation*         ProcessInformation;

  ctkCmdLineModuleReference CmdLineModuleReference;
};

//-----------------------------------------------------------------------------
// qSlicerCLIModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIModulePrivate::qSlicerCLIModulePrivate()
{
  this->ProcessInformation = 0;
  this->Index = -1;
}

//-----------------------------------------------------------------------------
// qSlicerCLIModule methods

//-----------------------------------------------------------------------------
qSlicerCLIModule::qSlicerCLIModule(QWidget* _parent):Superclass(_parent)
  , d_ptr(new qSlicerCLIModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCLIModule::~qSlicerCLIModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setup()
{
  Q_D(qSlicerCLIModule);
  
  // Temporary directory should be set before the module is initialized
  Q_ASSERT(!d->TempDirectory.isEmpty());

  // Set temp directory 
  vtkSlicerCLIModuleLogic* myLogic = vtkSlicerCLIModuleLogic::SafeDownCast(this->logic());
  myLogic->SetTemporaryDirectory(d->TempDirectory.toLatin1());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerCLIModule::createWidgetRepresentation()
{
  Q_D(qSlicerCLIModule);
  qSlicerCLIModuleWidget * widget = new qSlicerCLIModuleWidget;
  widget->setCmdLineModuleReference(d->CmdLineModuleReference);
  return widget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCLIModule::createLogic()
{
  Q_D(qSlicerCLIModule);
  vtkSlicerCLIModuleLogic* logic = vtkSlicerCLIModuleLogic::New();
  logic->SetDefaultModuleDescription(d->Desc);
  return logic;
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCLIModule, QString, title, Title);
CTK_GET_CPP(qSlicerCLIModule, QStringList, categories, Categories);
CTK_GET_CPP(qSlicerCLIModule, QStringList, contributors, Contributors);
CTK_GET_CPP(qSlicerCLIModule, int, index, Index);
CTK_GET_CPP(qSlicerCLIModule, QString, acknowledgementText, Acknowledgement);
CTK_GET_CPP(qSlicerCLIModule, QImage, logo, Logo);
CTK_GET_CPP(qSlicerCLIModule, QString, helpText, Help);
CTK_SET_CPP(qSlicerCLIModule, const QString&, setTempDirectory, TempDirectory);
CTK_GET_CPP(qSlicerCLIModule, QString, tempDirectory, TempDirectory);
CTK_SET_CPP(qSlicerCLIModule, const QString&, setEntryPoint, EntryPoint);
CTK_GET_CPP(qSlicerCLIModule, QString, entryPoint, EntryPoint);
CTK_SET_CPP(qSlicerCLIModule, const QString&, setModuleType, ModuleType);
CTK_GET_CPP(qSlicerCLIModule, QString, moduleType, ModuleType);

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setCmdLineModuleReference(const ctkCmdLineModuleReference& ref)
{
  Q_D(qSlicerCLIModule);
  d->CmdLineModuleReference = ref;

  //qDebug() << "xmlModuleDescription:" << xmlModuleDescription;
  Q_ASSERT(!this->entryPoint().isEmpty());

  // Set properties
  ctkCmdLineModuleDescription desc = d->CmdLineModuleReference.description();
  d->Categories = QStringList() << desc.category();
  d->Index = desc.index();
  d->Title = desc.title();
  d->Acknowledgement = desc.acknowledgements();
  d->Contributors = QStringList() << desc.contributor();

  QString help =
    "%1<br>"
    "For more detailed documentation see the online documentation at"
    "<a href=\"%2\">%2</a>";
  d->Help = help.arg(desc.description()).arg(desc.documentationURL());

  // ****************************************************************
  // *                  --=[ IMPORTANT ]=--                         *
  // *                                                              *
  // *   Waiting the CLI framework is fully redesigned to make use  *
  // *   of ctkCommandLineModules library, the ModuleDescription    *
  // *   structure is populated parsing the raw xml.                *
  // *                                                              *
  // ****************************************************************

  // Parse module description
  ModuleDescription cliDesc;
  ModuleDescriptionParser parser;
  if (parser.Parse(std::string(d->CmdLineModuleReference.rawXmlDescription()), cliDesc) != 0)
    {
    qWarning() << "Failed to parse xml module description:\n"
               << d->CmdLineModuleReference.rawXmlDescription();
    return;
    }

  d->ProcessInformation = cliDesc.GetProcessInformation();

  d->Logo = this->moduleLogoToImage(cliDesc.GetLogo());

  // Set module type
  cliDesc.SetType(this->moduleType().toStdString());

  // Set module entry point
  cliDesc.SetTarget(this->entryPoint().toStdString());

  // Register the module description in the master list
  vtkMRMLCommandLineModuleNode::RegisterModuleDescription(cliDesc);

  d->Desc = cliDesc;
}

//-----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic* qSlicerCLIModule::cliModuleLogic()
{
  vtkSlicerCLIModuleLogic* myLogic = vtkSlicerCLIModuleLogic::SafeDownCast(this->logic());
  return myLogic;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setLogo(const ModuleLogo& logo)
{
  Q_D(qSlicerCLIModule);
  d->Logo = this->moduleLogoToImage(logo);
}

//-----------------------------------------------------------------------------
QImage qSlicerCLIModule::moduleLogoToImage(const ModuleLogo& logo)
{
  if (logo.GetBufferLength() == 0)
    {
    return QImage();
    }
  return ctk::kwIconToQImage(reinterpret_cast<const unsigned char*>(logo.GetLogo()),
                             logo.GetWidth(), logo.GetHeight(),
                             logo.GetPixelSize(), logo.GetBufferLength(),
                             logo.GetOptions());
}

