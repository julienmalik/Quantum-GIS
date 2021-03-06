#include <QDir>
#include <QApplication>
#include <QStyle>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"

#include "qgsbrowsermodel.h"


QgsBrowserModel::QgsBrowserModel( QObject *parent ) :
    QAbstractItemModel( parent )
{
  QStyle *style = QApplication::style();
  mIconDirectory = QIcon( style->standardPixmap( QStyle::SP_DirClosedIcon ) );
  mIconDirectory.addPixmap( style->standardPixmap( QStyle::SP_DirOpenIcon ),
                            QIcon::Normal, QIcon::On );

  foreach( QFileInfo drive, QDir::drives() )
  {
    QString path = drive.absolutePath();
    QgsDirectoryItem *item = new QgsDirectoryItem( NULL, path, path );

    connectItem( item );
    mRootItems << item;
  }

  // Add non file top level items
  foreach( QString key, QgsProviderRegistry::instance()->providerList() )
  {
    QLibrary *library = QgsProviderRegistry::instance()->providerLibrary( key );
    if ( !library )
      continue;

    dataCapabilities_t * dataCapabilities = ( dataCapabilities_t * ) cast_to_fptr( library->resolve( "dataCapabilities" ) );
    if ( !dataCapabilities )
    {
      QgsDebugMsg( library->fileName() + " does not have dataCapabilities" );
      continue;
    }

    int capabilities = dataCapabilities();
    if ( capabilities == QgsDataProvider::NoDataCapabilities )
    {
      QgsDebugMsg( library->fileName() + " does not have any dataCapabilities" );
      continue;
    }

    dataItem_t * dataItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dataItem )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      continue;
    }

    QgsDataItem *item = dataItem( "", NULL );  // empty path -> top level
    if ( item )
    {
      QgsDebugMsg( "Add new top level item : " + item->name() );
      connectItem( item );
      mRootItems << item;
    }
  }
}

QgsBrowserModel::~QgsBrowserModel()
{
  foreach( QgsDataItem* item, mRootItems )
  {
    delete item;
  }
}


Qt::ItemFlags QgsBrowserModel::flags( const QModelIndex & index ) const
{
  if ( !index.isValid() )
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QgsBrowserModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsDataItem *item = dataItem( index );
  if ( !item )
  {
    return QVariant();
  }
  else if ( role == Qt::DisplayRole )
  {
    return item->name();
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 )
  {
    return item->icon();
  }
  else
  {
    // unsupported role
    return QVariant();
  }
}

QVariant QgsBrowserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    return QVariant( "header" );
  }

  return QVariant();
}

int QgsBrowserModel::rowCount( const QModelIndex &parent ) const
{
  //qDebug("rowCount: idx: (valid %d) %d %d", parent.isValid(), parent.row(), parent.column());

  if ( !parent.isValid() )
  {
    // root item: its children are top level items
    return mRootItems.count(); // mRoot
  }
  else
  {
    // ordinary item: number of its children
    QgsDataItem *item = dataItem( parent );
    return item ? item->rowCount() : 0;
  }
}

bool QgsBrowserModel::hasChildren( const QModelIndex & parent ) const
{
  if ( !parent.isValid() )
    return true; // root item: its children are top level items

  QgsDataItem *item = dataItem( parent );
  return item && item->hasChildren();
}

int QgsBrowserModel::columnCount( const QModelIndex & parent ) const
{
  return 1;
}

/* Refresh dir path */
void QgsBrowserModel::refresh( QString path, const QModelIndex &theIndex )
{
  QStringList paths = path.split( '/' );
  for ( int i = 0; i < rowCount( theIndex ); i++ )
  {
    QModelIndex idx = index( i, 0, theIndex );
    QgsDataItem *item = dataItem( idx );
    if ( !item )
      break;

    if ( item->path() == path )
    {
      QgsDebugMsg( "Arrived " + item->path() );
      item->refresh();
      return;
    }

    if ( path.indexOf( item->path() ) == 0 )
    {
      refresh( path, idx );
      break;
    }
  }
}

QModelIndex QgsBrowserModel::index( int row, int column, const QModelIndex &parent ) const
{
  QgsDataItem *p = dataItem( parent );
  const QVector<QgsDataItem*> &items = p ? p->children() : mRootItems;
  QgsDataItem *item = items.value( row, 0 );
  return item ? createIndex( row, column, item ) : QModelIndex();
}

QModelIndex QgsBrowserModel::parent( const QModelIndex &index ) const
{
  QgsDataItem *item = dataItem( index );
  if ( !item )
    return QModelIndex();

  return findItem( item->parent() );
}

QModelIndex QgsBrowserModel::findItem( QgsDataItem *item, QgsDataItem *parent ) const
{
  const QVector<QgsDataItem*> &items = parent ? parent->children() : mRootItems;

  for ( int i = 0; i < items.size(); i++ )
  {
    if ( items[i] == item )
      return createIndex( i, 0, item );

    QModelIndex childIndex = findItem( item, items[i] );
    if ( childIndex.isValid() )
      return childIndex;
  }

  return QModelIndex();
}

/* Refresh item */
void QgsBrowserModel::refresh( const QModelIndex& theIndex )
{
  QgsDataItem *item = dataItem( theIndex );
  if ( !item )
    return;

  QgsDebugMsg( "Refresh " + item->path() );
  item->refresh();
}

void QgsBrowserModel::beginInsertItems( QgsDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  QgsDebugMsg( "valid" );
  beginInsertRows( idx, first, last );
  QgsDebugMsg( "end" );
}
void QgsBrowserModel::endInsertItems()
{
  QgsDebugMsg( "Entered" );
  endInsertRows();
}
void QgsBrowserModel::beginRemoveItems( QgsDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  beginRemoveRows( idx, first, last );
}
void QgsBrowserModel::endRemoveItems()
{
  QgsDebugMsg( "Entered" );
  endRemoveRows();
}
void QgsBrowserModel::connectItem( QgsDataItem* item )
{
  connect( item, SIGNAL( beginInsertItems( QgsDataItem*, int, int ) ),
           this, SLOT( beginInsertItems( QgsDataItem*, int, int ) ) );
  connect( item, SIGNAL( endInsertItems() ),
           this, SLOT( endInsertItems() ) );
  connect( item, SIGNAL( beginRemoveItems( QgsDataItem*, int, int ) ),
           this, SLOT( beginRemoveItems( QgsDataItem*, int, int ) ) );
  connect( item, SIGNAL( endRemoveItems() ),
           this, SLOT( endRemoveItems() ) );
}

QgsDataItem *QgsBrowserModel::dataItem( const QModelIndex &idx ) const
{
  void *v = idx.internalPointer();
  QgsDataItem *d = reinterpret_cast<QgsDataItem*>( v );
  Q_ASSERT( !v || d );
  return d;
}
