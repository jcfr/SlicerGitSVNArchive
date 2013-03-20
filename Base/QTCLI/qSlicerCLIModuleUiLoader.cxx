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

#include "qSlicerCLIModuleUiLoader.h"

// Qt includes
#include <QIODevice>

// CTK includes
#include <ctkCmdLineModuleParameter.h>
#include <ctkDirectoryButton.h>

// MRML includes
#include "qMRMLNodeComboBox.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
struct qSlicerCLIModuleUiLoaderPrivate
{
  typedef qSlicerCLIModuleUiLoaderPrivate Self;
  qSlicerCLIModuleUiLoaderPrivate()
  {}

  /// Returns the node classname associated with the parameter \a tag and \a type.
  /// If \a tag is not found, value associated with "default" key is returned.
  /// \see nodeType(const QHash<QString, QString>& map, const QString& attribute);
  QString nodeType(const QString& tag, const QString& type)const;

  /// Initiliaze the maps containing the mapping
  /// parameter type -> MRML node type (classname)
  static void initializeMaps();

  /// Map used to store the different relation
  ///  parameter type -> MRML node type
  static bool MapInitialized;
  static QHash<QString, QHash<QString, QString> > TypeAttributeToNodeType;

  vtkSmartPointer<vtkMRMLScene> MRMLScene;
};

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleUiLoaderPrivate::MapInitialized = false;
QHash<QString, QHash<QString, QString> > qSlicerCLIModuleUiLoaderPrivate::TypeAttributeToNodeType;

//-----------------------------------------------------------------------------
QString qSlicerCLIModuleUiLoaderPrivate::nodeType(const QString& tag, const QString& type)const
{
  QHash<QString, QString> map = Self::TypeAttributeToNodeType[tag];
  QHash<QString, QString>::const_iterator i = map.constFind(type);

  if (i == map.constEnd())
    {
    return map.value("default");
    }
  return i.value();
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUiLoaderPrivate::initializeMaps()
{
  if (Self::MapInitialized)
    {
    return;
    }

  // Image type attribute mapping
  {
    QHash<QString, QString> map;
    map["default"] = "vtkMRMLScalarVolumeNode";
    map["scalar"] = "vtkMRMLScalarVolumeNode";
    map["label"] = "vtkMRMLScalarVolumeNode";
    map["vector"] = "vtkMRMLVectorVolumeNode";
    map["tensor"] = "vtkMRMLDiffusionTensorVolumeNode";
    map["diffusion-weighted"] = "vtkMRMLDiffusionWeightedVolumeNode";
    map["signal"] = "vtkMRMLMultiVolumeNode";
    map["multichannel"] = "vtkMRMLMultiVolumeNode";
    map["dynamic-contrast-enhanced"] = "vtkMRMLMultiVolumeNode";

    Self::TypeAttributeToNodeType["image"] = map;
  }


  // Geometry type attribute mapping
  {
    QHash<QString, QString> map;
    map["default"] = "vtkMRMLModelNode";
    map["fiberbundle"] = "vtkMRMLFiberBundleNode";
    map["model"] = "vtkMRMLModelNode";

    Self::TypeAttributeToNodeType["geometry"] = map;
  }

  // Table type attribute mapping
  {
    QHash<QString, QString> map;
    map["color"] = "vtkMRMLColorNode";

    Self::TypeAttributeToNodeType["table"] = map;
  }

  // Transform type attribute mapping
  {
    QHash<QString, QString> map;
    map["default"] = "vtkMRMLTransformNode";
    map["linear"] = "vtkMRMLLinearTransformNode";
    map["nonlinear"] = "vtkMRMLGridTransformNode";
    map["bspline"] = "vtkMRMLBSplineTransformNode";

    Self::TypeAttributeToNodeType["transform"] = map;
  }

  Self::MapInitialized = true;
}

//-----------------------------------------------------------------------------
qSlicerCLIModuleUiLoader::qSlicerCLIModuleUiLoader(QObject* parent)
  : Superclass(parent), d_ptr(new qSlicerCLIModuleUiLoaderPrivate)
{
  Q_D(qSlicerCLIModuleUiLoader);
  d->initializeMaps();
}

//-----------------------------------------------------------------------------
qSlicerCLIModuleUiLoader::~qSlicerCLIModuleUiLoader()
{
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerCLIModuleUiLoader::mrmlScene() const
{
  Q_D(const qSlicerCLIModuleUiLoader);
  return d->MRMLScene;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModuleUiLoader::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerCLIModuleUiLoader);
  if (scene == d->MRMLScene)
    {
    return ;
    }
  d->MRMLScene = scene;
  emit mrmlSceneChanged(scene);
}

#include <QDebug>
//-----------------------------------------------------------------------------
QWidget* qSlicerCLIModuleUiLoader::createWidget(const QString & className, QWidget * parent, const QString & name)
{
  Q_D(qSlicerCLIModuleUiLoader);
  QWidget * widget = this->Superclass::createWidget(className, parent, name);
  QString parameterName = QString(name).replace("parameter:", "");
  if (className == "qMRMLNodeComboBox")
    {
    qMRMLNodeComboBox * mrmlNodeComboBox = qobject_cast<qMRMLNodeComboBox*>(widget);
    mrmlNodeComboBox->setObjectName(name);
    mrmlNodeComboBox->setRenameEnabled(true);
    mrmlNodeComboBox->setMRMLScene(this->mrmlScene());
    QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                     mrmlNodeComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));

    if (this->description().hasParameter(parameterName))
      {
      ctkCmdLineModuleParameter param = this->description().parameter(parameterName);

      mrmlNodeComboBox->setBaseName(param.label());
      mrmlNodeComboBox->setNoneEnabled(param.index() == -1);

      if(param.tag() == "image")
        {
        mrmlNodeComboBox->setNodeTypes(QStringList() << d->nodeType(param.tag(), param.type()));
        // If "type" is specified, only display nodes of type nodeType
        // (e.g. vtkMRMLScalarVolumeNode), don't display subclasses
        // (e.g. vtkMRMLDiffusionTensorVolumeNode)
        mrmlNodeComboBox->setShowChildNodeTypes(param.type().isEmpty() || param.type() == "any");
        // Being able to create an image for the input is meaningless as the created
        // volume would be empty (useless as an input).
        // However, if it's an output, the result would be saved into the newly
        // created node.
        mrmlNodeComboBox->setAddEnabled(param.channel() != "input");

        if (param.type() == "label")
          {
          mrmlNodeComboBox->addAttribute(d->nodeType(param.tag(), param.type()), "LabelMap",QString("1"));
          }
        }
      else if(param.tag() == "geometry")
        {
        QString nodeType = d->nodeType(param.tag(), param.type());
        bool showHidden = param.multiple() /* TODO && param.aggregate()*/;
        if (showHidden)
          {
          nodeType = "vtkMRMLModelHierarchyNode";
          }
        // TODO - title + " Model"
        mrmlNodeComboBox->setShowHidden(showHidden);
        mrmlNodeComboBox->setNodeTypes(QStringList() << nodeType);
        }
      else if(param.tag() == "table")
        {
        // TODO - title + " Table"
        mrmlNodeComboBox->setNodeTypes(QStringList() << d->nodeType(param.tag(), param.type()));
        }
      else if(param.tag() == "measurement")
        {
        // TODO - title + " Measurement"
        mrmlNodeComboBox->setNodeTypes(QStringList() << "vtkMRMLDoubleArrayNode");
        }
      else if(param.tag() == "transform")
        {
        // Note: TransformNode is abstract making it inappropriate for
        // an output type since the node selector must be able to make
        // an instance of the class.  For now, revert to LinearTransformNode.
        QString nodeType = param.channel() == "input" ?
              d->nodeType(param.tag(), param.type()) : "vtkMRMLLinearTransformNode";
        mrmlNodeComboBox->setAddEnabled(nodeType != "vtkMRMLTransformNode" && param.channel() != "input");
        mrmlNodeComboBox->setNodeTypes(QStringList() << nodeType);
        }
      }
    }
  else if (className == "ctkDirectoryButton")
    {
    ctkDirectoryButton * directoryButton = qobject_cast<ctkDirectoryButton*>(widget);
    directoryButton->setCaption(QString("Select %1 ...").arg(parameterName));
    }
  return widget;
}
