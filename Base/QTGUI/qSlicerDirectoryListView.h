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

#ifndef __qSlicerDirectoryListView_h
#define __qSlicerDirectoryListView_h

// Qt includes
#include <QVariant>
#include <QWidget>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerDirectoryListViewPrivate;
class QStandardItem;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerDirectoryListView : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(QVariantMap directoryMap READ directoryMap WRITE setDirectoryMap NOTIFY directoryListChanged)
  Q_PROPERTY(QStringList directoryList READ directoryList WRITE setDirectoryList NOTIFY directoryListChanged)
  Q_PROPERTY(QStringList encodedDirectoryList READ encodedDirectoryList WRITE setDirectoryList NOTIFY directoryListChanged)
  Q_PROPERTY(bool hasEnableDisableFeature READ hasEnableDisableFeature WRITE setHasEnableDisableFeature)
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerDirectoryListView(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerDirectoryListView();

  /// Return True if the \a path has already been added
  bool hasDirectory(const QString& path)const;

  /// Return True if the \a path has been added and is enabled
  /// \sa setDirectoryEnabled()
  bool isDirectoryEnabled(const QString& path)const;

  /// \sa isDirectoryEnabled()
  void setDirectoryEnabled(const QString& path, bool enabled);

  /// Return list of all directories.
  /// \sa setDirectoryList() isDirectoryEnabled()
  QStringList directoryList()const;

  /// Return list of all directories appended with "?disabled=1" if it applies.
  /// \sa setDirectoryList() isDirectoryEnabled()
  QStringList encodedDirectoryList()const;

  /// \sa setDirectoryMap()
  QVariantMap directoryMap()const;

  /// \sa selectAllDirectories()
  QStringList selectedDirectoryList()const;

  /// \sa setHasEnableDisableFeature()
  bool hasEnableDisableFeature()const;

public slots:

  /// If \a path exists, add it to the view and emit signal directoryListChanged().
  /// \sa directoryListChanged()
  void addDirectory(const QString& path, bool enabled = true);

  /// Remove all entries and set \a paths has current list.
  /// \a path ending with "?disabled=1" or "?disabled=0" will be disabled
  /// or enabled accordingly.
  /// The signal directoryListChanged() is emitted if the current list of directories is
  /// different from the provided one.
  /// \sa addDirectory(), directoryListChanged(), setDirectoryMap()
  void setDirectoryList(const QStringList& paths);

  /// Remove all entries and set \a map has current list.
  /// The map keys are path and the associated value should be a boolean indicating
  /// if the corresponding path should enabled or disabled.
  /// The signal directoryListChanged() is emitted if the current list of directories is
  /// different from the provided one.
  /// addDirectory(), directoryListChanged(), setDirectoryList()
  void setDirectoryMap(const QVariantMap& map);

  /// Remove \a path from the list.
  /// The signal directoryListChanged() is emitted if the path was in the list.
  /// \sa directoryListChanged()
  void removeDirectory(const QString& path);

  /// \sa selectAllDirectories()
  void removeSelectedDirectories();

  /// Select all directories.
  void selectAllDirectories();

  /// Clear the current directory selection.
  void clearDirectorySelection();

  /// Enable or disable the selected directories.
  void toggleSelectedDirectories();

  /// \sa hasEnableDisableFeature()
  void setHasEnableDisableFeature(bool value);

protected slots:

  void onItemChanged(QStandardItem * item);

signals:
  /// This signal is emitted when a directory is added to the view.
  void directoryListChanged();

  /// This signal is emitted when the user select/unselect a directory.
  void directorySelectionChanged();

protected:
  QScopedPointer<qSlicerDirectoryListViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDirectoryListView);
  Q_DISABLE_COPY(qSlicerDirectoryListView);

};

#endif
