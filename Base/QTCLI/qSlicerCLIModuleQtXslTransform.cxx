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

#include "qSlicerCLIModuleQtXslTransform.h"

// Qt includes
#include <QFile>
#include <QStringList>

//-----------------------------------------------------------------------------
struct qSlicerCLIModuleQtXslTransformPrivate
{
  qSlicerCLIModuleQtXslTransformPrivate()
  {}

  QScopedPointer<QIODevice> xslExtraFile;
};

//-----------------------------------------------------------------------------
qSlicerCLIModuleQtXslTransform::qSlicerCLIModuleQtXslTransform(QIODevice* input, QIODevice* output)
  : Superclass(input, output), d_ptr(new qSlicerCLIModuleQtXslTransformPrivate)
{
  Q_D(qSlicerCLIModuleQtXslTransform);

  d->xslExtraFile.reset(new QFile(":/qSlicerCLIModuleXmlToQtGui.xsd"));
  this->setXslExtraTransformation(d->xslExtraFile.data());

  foreach(const QString& xslParamName, QStringList()
          << "imageInput" << "imageOutput"
          << "geometryInput" << "geometryOutput"
          << "transformInput" << "transformOutput"
          << "tableInput" << "tableOutput"
          << "measurementInput" << "measurementOutput"
          )
    {
    this->bindVariable(QString("%1Widget").arg(xslParamName), "qMRMLNodeComboBox");
    this->bindVariable(QString("%1SetProperty").arg(xslParamName), "");
    this->bindVariable(QString("%1ValueProperty").arg(xslParamName), "currentNodeId");
    }

  this->bindVariable("pointWidget", "qMRMLNodeComboBox");
  this->bindVariable("pointValueProperty", "currentNodeId");

  this->bindVariable("directoryWidget", "ctkDirectoryButton");
}

//-----------------------------------------------------------------------------
qSlicerCLIModuleQtXslTransform::~qSlicerCLIModuleQtXslTransform()
{
}
