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
#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QStringList>
#include <QUrl>

// CTK includes
#include <ctkSetName.h>

// QtGUI includes
#include "qSlicerDirectoryListView.h"

// --------------------------------------------------------------------------
// qSlicerDirectoryListViewPrivate

//-----------------------------------------------------------------------------
class qSlicerDirectoryListViewPrivate
{
  Q_DECLARE_PUBLIC(qSlicerDirectoryListView);
protected:
  qSlicerDirectoryListView* const q_ptr;

public:
  qSlicerDirectoryListViewPrivate(qSlicerDirectoryListView& object);
  void init();

  void addDirectory(const QString& path);

  bool setDirectoryEnabled(const QString& path, bool enabled);

  QListView*         ListView;
  QStandardItemModel DirectoryListModel;
  bool               HasEnableDisableFeature;
};

// --------------------------------------------------------------------------
// qSlicerDirectoryListViewPrivate methods

// --------------------------------------------------------------------------
qSlicerDirectoryListViewPrivate::qSlicerDirectoryListViewPrivate(qSlicerDirectoryListView& object)
  :q_ptr(&object)
{
  this->HasEnableDisableFeature = false;
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListViewPrivate::init()
{
  Q_Q(qSlicerDirectoryListView);

  this->ListView = new QListView() << ctkSetName("DirectoryListView");
  this->ListView->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->ListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->ListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->ListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  QHBoxLayout * layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(this->ListView);
  q->setLayout(layout);

  this->ListView->setModel(&this->DirectoryListModel);

  QObject::connect(this->ListView->selectionModel(),
                   SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   q, SIGNAL(directorySelectionChanged()));

  QObject::connect(&this->DirectoryListModel, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));

  QObject::connect(&this->DirectoryListModel, SIGNAL(itemChanged(QStandardItem*)),
                   q, SIGNAL(directoryListChanged()));
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListViewPrivate::addDirectory(const QString& path)
{
  Q_Q(qSlicerDirectoryListView);
  if (!QFile::exists(path) || q->hasDirectory(path))
    {
    return;
    }
  QStandardItem * item = new QStandardItem(path);
  if (this->HasEnableDisableFeature)
    {
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    }
  item->setData(QVariant(path), Qt::ToolTipRole);
  this->DirectoryListModel.appendRow(item);
}

// --------------------------------------------------------------------------
bool qSlicerDirectoryListViewPrivate::setDirectoryEnabled(const QString& path, bool enabled)
{
  Q_Q(qSlicerDirectoryListView);
  QModelIndexList foundIndexes = this->DirectoryListModel.match(
          this->DirectoryListModel.index(0, 0), Qt::DisplayRole,
          QVariant(path));
  Q_ASSERT(foundIndexes.size() < 2);
  if(!foundIndexes.empty())
    {
    QStandardItem* foundItem = this->DirectoryListModel.findItems(path).at(0);
    bool savedBlockSignals = q->blockSignals(true);
    foundItem->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    q->blockSignals(savedBlockSignals);
    return true;
    }
  return false;
}

// --------------------------------------------------------------------------
// qSlicerDirectoryListView methods

// --------------------------------------------------------------------------
qSlicerDirectoryListView::qSlicerDirectoryListView(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDirectoryListViewPrivate(*this))
{
  Q_D(qSlicerDirectoryListView);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerDirectoryListView::~qSlicerDirectoryListView()
{
}

// --------------------------------------------------------------------------
QStringList qSlicerDirectoryListView::directoryList()const
{
  Q_D(const qSlicerDirectoryListView);
  QStringList directoryList;
  for(int i = 0; i < d->DirectoryListModel.rowCount(); ++i)
    {
    directoryList << d->DirectoryListModel.data(d->DirectoryListModel.index(i, 0)).toString();
    }
  return directoryList;
}

// --------------------------------------------------------------------------
QStringList qSlicerDirectoryListView::encodedDirectoryList()const
{
  QStringList paths;
  foreach(const QString& path, this->directoryList())
    {
    QUrl url(path);
    if (!this->isDirectoryEnabled(path))
      {
      url.addQueryItem("disabled", "1");
      }
    paths << url.toString(QUrl::RemoveScheme | QUrl::RemovePassword
                          | QUrl::RemoveAuthority | QUrl::RemoveFragment
                          | QUrl::StripTrailingSlash);
    }
  return paths;
}

// --------------------------------------------------------------------------
QVariantMap qSlicerDirectoryListView::directoryMap()const
{
  QVariantMap directoryMap;
  foreach(const QString& path, this->directoryList())
    {
    directoryMap.insert(path, this->isDirectoryEnabled(path));
    }
  return directoryMap;
}

// --------------------------------------------------------------------------
QStringList qSlicerDirectoryListView::selectedDirectoryList()const
{
  Q_D(const qSlicerDirectoryListView);
  QStringList directoryList;
  QModelIndexList selectedIndexes = d->ListView->selectionModel()->selectedRows();
  foreach(const QModelIndex& index, selectedIndexes)
    {
    directoryList << d->DirectoryListModel.data(index).toString();
    }
  return directoryList;
}

// --------------------------------------------------------------------------
bool qSlicerDirectoryListView::hasEnableDisableFeature()const
{
  Q_D(const qSlicerDirectoryListView);
  return d->HasEnableDisableFeature;
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::setHasEnableDisableFeature(bool enabled)
{
  Q_D(qSlicerDirectoryListView);
  if (d->HasEnableDisableFeature == enabled)
    {
    return;
    }
  d->HasEnableDisableFeature = enabled;
}

// --------------------------------------------------------------------------
bool qSlicerDirectoryListView::hasDirectory(const QString& path)const
{
  Q_D(const qSlicerDirectoryListView);
  QModelIndexList foundIndexes = d->DirectoryListModel.match(
        d->DirectoryListModel.index(0, 0), Qt::DisplayRole,
        QVariant(path), /* hits = */ 1, Qt::MatchExactly | Qt::MatchWrap);
  Q_ASSERT(foundIndexes.size() < 2);
  return (foundIndexes.size() != 0);
}

// --------------------------------------------------------------------------
bool qSlicerDirectoryListView::isDirectoryEnabled(const QString& path)const
{
  Q_D(const qSlicerDirectoryListView);
  if(!this->hasDirectory(path))
    {
    return false;
    }
  QStandardItem* foundItem = d->DirectoryListModel.findItems(path).at(0);
  return foundItem->checkState() == Qt::Checked;
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::setDirectoryEnabled(const QString& path, bool enabled)
{
  Q_D(qSlicerDirectoryListView);
  if (!this->hasDirectory(path) || this->isDirectoryEnabled(path) == enabled)
    {
    return;
    }
  d->setDirectoryEnabled(path, enabled);
  emit this->directoryListChanged();
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::addDirectory(const QString& path, bool enabled)
{
  Q_D(qSlicerDirectoryListView);

  if(!this->hasDirectory(path))
    {
    d->addDirectory(path);
    d->setDirectoryEnabled(path, enabled);
    emit this->directoryListChanged();
    }
  else
    {
    this->setDirectoryEnabled(path, enabled);
    }
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::removeDirectory(const QString& path)
{
  Q_D(qSlicerDirectoryListView);
  QList<QStandardItem*> foundItems = d->DirectoryListModel.findItems(path);
  Q_ASSERT(foundItems.count() < 2);
  if (foundItems.count() == 1)
    {
    d->DirectoryListModel.removeRow(foundItems.at(0)->row());
    emit this->directoryListChanged();
    }
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::removeSelectedDirectories()
{
  Q_D(qSlicerDirectoryListView);

  QModelIndexList selectedIndexes = d->ListView->selectionModel()->selectedRows();
  bool selectedCount = selectedIndexes.count();
  while(selectedIndexes.count() > 0)
    {
    d->DirectoryListModel.removeRow(selectedIndexes.at(0).row());
    selectedIndexes = d->ListView->selectionModel()->selectedRows();
    }
  if (selectedCount)
    {
    emit this->directoryListChanged();
    }
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::selectAllDirectories()
{
  Q_D(qSlicerDirectoryListView);
  d->ListView->selectAll();
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::clearDirectorySelection()
{
  Q_D(qSlicerDirectoryListView);
  d->ListView->clearSelection();
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::setDirectoryList(const QStringList& paths)
{
  QVariantMap map;
  foreach(const QString& path, paths)
    {
    QUrl url(path);
    map.insert(url.path(), !QVariant(url.queryItemValue("disabled")).toBool());
    }
  this->setDirectoryMap(map);
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::setDirectoryMap(const QVariantMap& map)
{
  Q_D(qSlicerDirectoryListView);

  // Check if map values already belong to the model
  if (map.count() == this->directoryList().count())
    {
    int found = 0;
    foreach(const QString& path, map.keys())
      {
      if (this->hasDirectory(path)
          && this->isDirectoryEnabled(path) == map.value(path).toBool())
        {
        ++found;
        }
      }
    if (found == map.count())
      {
      return;
      }
    }

  d->DirectoryListModel.removeRows(0, d->DirectoryListModel.rowCount());

  foreach(const QString& path, map.keys())
    {
    if(!this->hasDirectory(path))
      {
      d->addDirectory(path);
      }
    d->setDirectoryEnabled(path, map.value(path).toBool());
    }
  emit this->directoryListChanged();
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::toggleSelectedDirectories()
{
  Q_D(qSlicerDirectoryListView);
  foreach(const QString& path, this->selectedDirectoryList())
    {
    d->setDirectoryEnabled(path, !this->isDirectoryEnabled(path));
    }
  emit this->directoryListChanged();
}

// --------------------------------------------------------------------------
void qSlicerDirectoryListView::onItemChanged(QStandardItem * item)
{
  Q_D(qSlicerDirectoryListView);
  bool savedBlockSignals = d->DirectoryListModel.blockSignals(true);
  if (item->checkState() != Qt::Unchecked)
    {
    item->setData(QVariant(Qt::black), Qt::ForegroundRole);
    }
  else
    {
    item->setData(QVariant(Qt::darkGray), Qt::ForegroundRole);
    }
  d->DirectoryListModel.blockSignals(savedBlockSignals);
}
