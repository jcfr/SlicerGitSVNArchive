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
#include <QApplication>
#include <QDir>
#include <QListView>
#include <QSignalSpy>
#include <QStandardItemModel>

#include <QDebug>

// SlicerQt includes
#include "qSlicerDirectoryListView.h"

// STD includes
#include <iostream>

//                                      void hasDirectory();
//                                      void isDirectoryEnabled();
//                                      void setDirectoryEnabled();
//                                      void directoryList();
//                                      void directoryMap();
//                                      void selectedDirectoryList();
//                                      void addDirectory();
//                                      void setDirectoryList();
//  void setDirectoryMap();
//                                      void removeDirectory();
//                                      void removeSelectedDirectories();
//                                      void selectAllDirectories();
//                                      void clearDirectorySelection();
//                                      void toggleSelectedDirectories();
//};


// ----------------------------------------------------------------------------
int qSlicerDirectoryListViewTest1(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  qSlicerDirectoryListView widget;

  QSignalSpy spy(&widget, SIGNAL(directoryListChanged()));

  //
  // Test directoryList() / setDirectoryList()
  //

  if (widget.directoryList().count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  QStringList paths;
  widget.setDirectoryList(paths);

  if (spy.count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  paths << "." << "/should-not-exist" << ".";
  widget.setDirectoryList(paths);

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.directoryList().count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryList/setDirectoryList methods !" << std::endl;
    return EXIT_FAILURE;
    }

  QString current = widget.directoryList().at(0);
  QString expected = QString(".");
  if (current != expected)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryList/setDirectoryList methods !\n"
              << " current[0]:" << qPrintable(current) << "\n"
              << " expected[0]:" << qPrintable(expected) << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test hasDirectory()
  //

  spy.clear();

  bool expectedAsBool = false;
  bool currentAsBool = widget.hasDirectory("/should-not-exist");
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with hasDirectory() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  expectedAsBool = true;
  currentAsBool = widget.hasDirectory(".");
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with hasDirectory() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  widget.removeDirectory(".");

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeDirectory() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.directoryList().count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeDirectory() method !" << std::endl;
    return EXIT_FAILURE;
    }

  expectedAsBool = false;
  currentAsBool = widget.hasDirectory(".");
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with hasDirectory() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Set 2 directories
  //

  spy.clear();
  paths.clear();
  paths << "." << QDir::tempPath();
  widget.setDirectoryList(paths);

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test selectAllDirectories() / selectedDirectoryList()
  //

  if (widget.selectedDirectoryList().size() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectedDirectoryList/setDirectoryList  method !" << std::endl;
    return EXIT_FAILURE;
    }

  widget.selectAllDirectories();

  if (widget.selectedDirectoryList().size() != 2)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectAllDirectories/selectedDirectoryList methods !" << std::endl;
    return EXIT_FAILURE;
    }

  QStringList currentAsList = widget.selectedDirectoryList();
  QStringList expectedAsList = QStringList() << "." << QDir::tempPath();
  if (currentAsList != expectedAsList)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectAllDirectories/selectedDirectoryList methods !\n"
              << " currentAsList:" << qPrintable(currentAsList.join(" ")) << "\n"
              << " expectedAsList:" << qPrintable(expectedAsList.join(" ")) << std::endl;
    return EXIT_FAILURE;
    }

  currentAsList = widget.selectedDirectoryList();
  expectedAsList = QStringList() << "." << QDir::tempPath();
  if (currentAsList != expectedAsList)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with selectAllDirectories/selectedDirectoryList methods !\n"
              << " currentAsList:" << qPrintable(currentAsList.join(" ")) << "\n"
              << " expectedAsList:" << qPrintable(expectedAsList.join(" ")) << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test clearDirectorySelection()
  //

  spy.clear();

  widget.clearDirectorySelection();

  if (spy.count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeSelectedDirectories() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.selectedDirectoryList().size() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with clearDirectorySelection() method !" << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test removeSelectedDirectories()
  //

  spy.clear();

  widget.selectAllDirectories();
  widget.removeSelectedDirectories();

  if (widget.directoryList().count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeSelectedDirectories() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeSelectedDirectories() method !" << std::endl;
    return EXIT_FAILURE;
    }

  paths.clear();

  //
  // Test addDirectory()
  //

  QDir currentDirParent = QDir::current();
  currentDirParent.cdUp();

  spy.clear();

  widget.addDirectory(".");
  widget.addDirectory(QDir::tempPath());
  widget.addDirectory(currentDirParent.absolutePath());

  if (spy.count() != 3)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with removeSelectedDirectories() method !" << std::endl;
    return EXIT_FAILURE;
    }

  currentAsList = widget.directoryList();
  expectedAsList = QStringList() << "." << QDir::tempPath() << currentDirParent.absolutePath();
  if (currentAsList != expectedAsList)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with addDirectory() method !\n"
              << " currentAsList:" << qPrintable(currentAsList.join(" ")) << "\n"
              << " expectedAsList:" << qPrintable(expectedAsList.join(" ")) << std::endl;
    return EXIT_FAILURE;
    }

  currentAsList = widget.directoryList();
  expectedAsList = QStringList() << "." << QDir::tempPath() << currentDirParent.absolutePath();
  if (currentAsList != expectedAsList)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with addDirectory() method !\n"
              << " currentAsList:" << qPrintable(currentAsList.join(" ")) << "\n"
              << " expectedAsList:" << qPrintable(expectedAsList.join(" ")) << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Get reference to DirectoryListModel
  //
  QStandardItemModel * directoryListModel =
      qobject_cast<QStandardItemModel*>(widget.findChild<QListView*>("DirectoryListView")->model());
  if (!directoryListModel)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem getting reference to the model associated with 'DirectoryListView' !" << std::endl;
    return EXIT_FAILURE;
    }

  // State of all the items is expected to be checked
  for(int idx = 0; idx < directoryListModel->rowCount(); ++idx)
    {
    if (directoryListModel->item(idx)->checkState() != Qt::Checked)
      {
      std::cerr << "Line " << __LINE__ << " - Item[" << idx <<"] is expected to be checked !" << std::endl;
      return EXIT_FAILURE;
      }
    }

  //
  // Check that signal 'directoryListChanged()' is emitted when directly changing the
  // check state of an items.
  //
  spy.clear();
  directoryListModel->item(1)->setCheckState(Qt::Unchecked);

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setting state of directoryListModel item !" << std::endl;
    return EXIT_FAILURE;
    }

  if (directoryListModel->item(0)->checkState() != Qt::Checked)
    {
    std::cerr << "Line " << __LINE__ << " - Item[0] is expected to be checked !" << std::endl;
    return EXIT_FAILURE;
    }

  if (directoryListModel->item(1)->checkState() != Qt::Unchecked)
    {
    std::cerr << "Line " << __LINE__ << " - Item[1] is expected to be unchecked !" << std::endl;
    return EXIT_FAILURE;
    }

  if (directoryListModel->item(2)->checkState() != Qt::Checked)
    {
    std::cerr << "Line " << __LINE__ << " - Item[2] is expected to be checked !" << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test isDirectoryEnabled()
  //

  currentAsBool = widget.isDirectoryEnabled(".");
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with isDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(QDir::tempPath());
  expectedAsBool = false;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with isDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(currentDirParent.absolutePath());
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with isDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }


  //
  // Test toggleSelectedDirectories()
  //

  spy.clear();
  widget.selectAllDirectories();
  widget.toggleSelectedDirectories();

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with toggleSelectedDirectories() method !" << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(".");
  expectedAsBool = false;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with isDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(QDir::tempPath());
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with isDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(currentDirParent.absolutePath());
  expectedAsBool = false;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with isDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test setDirectoryEnabled()
  //

  spy.clear();
  widget.setDirectoryEnabled("/should-not-exist", false);
  widget.setDirectoryEnabled("/should-not-exist", true);
  if (spy.count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryEnabled() method !" << std::endl;
    return EXIT_FAILURE;
    }

  spy.clear();
  widget.setDirectoryEnabled(".", false);
  if (spy.count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryEnabled() method !" << std::endl;
    return EXIT_FAILURE;
    }
  currentAsBool = widget.isDirectoryEnabled(".");
  expectedAsBool = false;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  spy.clear();
  widget.setDirectoryEnabled(QDir::tempPath(), false);
  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryEnabled() method !" << std::endl;
    return EXIT_FAILURE;
    }
  currentAsBool = widget.isDirectoryEnabled(QDir::tempPath());
  expectedAsBool = false;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  spy.clear();
  widget.setDirectoryEnabled(currentDirParent.absolutePath(), true);
  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryEnabled() method !" << std::endl;
    return EXIT_FAILURE;
    }
  currentAsBool = widget.isDirectoryEnabled(currentDirParent.absolutePath());
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryEnabled() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test directoryMap()
  //
  spy.clear();
  QVariantMap map = widget.directoryMap();
  if (spy.count() != 0)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryMap() method !" << std::endl;
    return EXIT_FAILURE;
    }
  if (map.count() != 3)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryMap() method !" << std::endl;
    return EXIT_FAILURE;
    }
  if (!map.contains(".")
      || !map.contains(QDir::tempPath())
      || !map.contains(currentDirParent.absolutePath()))
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryMap() method !" << std::endl;
    return EXIT_FAILURE;
    }
  if (map.value(".").toBool() != false
      || map.value(QDir::tempPath()).toBool() != false
      || map.value(currentDirParent.absolutePath()).toBool() != true)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with directoryMap() method !" << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test setDirectoryMap()
  //
  widget.setDirectoryList(QStringList());

  spy.clear();

  QVariantMap directoryMap;
  directoryMap.insert(".", true);
  directoryMap.insert(QDir::tempPath(), true);
  directoryMap.insert(currentDirParent.absolutePath(), false);

  widget.setDirectoryMap(directoryMap);

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryMap() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.directoryList().count() != 3)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryMap() method !" << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(".");
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryMap() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(QDir::tempPath());
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryMap() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(currentDirParent.absolutePath());
  expectedAsBool = false;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryMap() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test encodedDirectoryList()
  //

  currentAsList = widget.encodedDirectoryList();
  expectedAsList = QStringList()
      << "."
      << QDir::tempPath()
      << QString("%1?disabled=1").arg(currentDirParent.absolutePath());
  expectedAsList.sort();
  if (currentAsList != expectedAsList)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with encodedDirectoryList() method !\n"
              << " currentAsList:" << qPrintable(currentAsList.join(" ")) << "\n"
              << " expectedAsList:" << qPrintable(expectedAsList.join(" ")) << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Test setDirectoryList() with path having "?disabled="
  //

  widget.setDirectoryList(QStringList());

  spy.clear();

  QStringList urls = QStringList()
        << "."
        << QDir::tempPath()
        << QString("%1?disabled=1").arg(currentDirParent.absolutePath());

  widget.setDirectoryList(urls);

  if (spy.count() != 1)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  if (widget.directoryList().count() != 3)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !" << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(".");
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setDirectoryList() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(QDir::tempPath());
  expectedAsBool = true;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setUrls() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  currentAsBool = widget.isDirectoryEnabled(currentDirParent.absolutePath());
  expectedAsBool = false;
  if (currentAsBool != expectedAsBool)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with setUrls() method !\n"
              << " currentAsBool:" << currentAsBool << "\n"
              << " expectedAsBool:" << expectedAsBool << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
