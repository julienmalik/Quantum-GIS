#ifndef QGSRENDERERV2REGISTRY_H
#define QGSRENDERERV2REGISTRY_H

#include <QIcon>
#include <QMap>
#include <QStringList>

class QgsFeatureRendererV2;
class QDomElement;
class QgsVectorLayer;
class QgsStyleV2;
class QgsRendererV2Widget;

/**
 Stores metadata about one renderer class.

 @note It's necessary to implement createRenderer() function.
   In C++ you can use QgsRendererV2Metadata convenience class.
 */
class CORE_EXPORT QgsRendererV2AbstractMetadata
{
  public:
    QgsRendererV2AbstractMetadata( QString name, QString visibleName, QIcon icon = QIcon() )
        : mName( name ), mVisibleName( visibleName ), mIcon( icon ) {}

    QString name() const { return mName; }
    QString visibleName() const { return mVisibleName; }

    QIcon icon() const { return mIcon; }
    void setIcon( const QIcon& icon ) { mIcon = icon; }

    /** Return new instance of the renderer given the DOM element. Returns NULL on error.
     * Pure virtual function: must be implemented in derived classes.  */
    virtual QgsFeatureRendererV2* createRenderer( QDomElement& elem ) = 0;
    /** Return new instance of settings widget for the renderer. Returns NULL on error. */
    virtual QgsRendererV2Widget* createRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    { return NULL; }

  protected:
    //! name used within QGIS for identification (the same what renderer's type() returns)
    QString mName;
    //! name visible for users (translatable)
    QString mVisibleName;
    //! icon to be shown in the renderer properties dialog
    QIcon mIcon;
};


typedef QgsFeatureRendererV2*( *QgsRendererV2CreateFunc )( QDomElement& );
typedef QgsRendererV2Widget*( *QgsRendererV2WidgetFunc )( QgsVectorLayer*, QgsStyleV2*, QgsFeatureRendererV2* );

/**
 Convenience metadata class that uses static functions to create renderer and its widget.
 */
class CORE_EXPORT QgsRendererV2Metadata : public QgsRendererV2AbstractMetadata
{
  public:

    /** construct metadata */
    QgsRendererV2Metadata( QString name,
                           QString visibleName,
                           QgsRendererV2CreateFunc pfCreate,
                           QIcon icon = QIcon(),
                           QgsRendererV2WidgetFunc pfWidget = NULL )
        : QgsRendererV2AbstractMetadata( name, visibleName, icon ), mCreateFunc( pfCreate ), mWidgetFunc( pfWidget ) {}

    virtual QgsFeatureRendererV2* createRenderer( QDomElement& elem ) { return mCreateFunc ? mCreateFunc( elem ) : NULL; }
    virtual QgsRendererV2Widget* createRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    { return mWidgetFunc ? mWidgetFunc( layer, style, renderer ) : NULL; }

    QgsRendererV2CreateFunc createFunction() const { return mCreateFunc; }
    QgsRendererV2WidgetFunc widgetFunction() const { return mWidgetFunc; }

    void setWidgetFunction( QgsRendererV2WidgetFunc f ) { mWidgetFunc = f; }

  protected:
    //! pointer to function that creates an instance of the renderer when loading project / style
    QgsRendererV2CreateFunc mCreateFunc;
    //! pointer to function that creates a widget for configuration of renderer's params
    QgsRendererV2WidgetFunc mWidgetFunc;
};

/**
  Registry of renderers.

  This is a singleton, renderers can be added / removed at any time
 */
class CORE_EXPORT QgsRendererV2Registry
{
  public:

    static QgsRendererV2Registry* instance();

    //! add a renderer to registry. Takes ownership of the metadata object.
    bool addRenderer( QgsRendererV2AbstractMetadata* metadata );

    //! remove renderer from registry
    bool removeRenderer( QString rendererName );

    //! get metadata for particular renderer. Returns NULL if not found in registry.
    QgsRendererV2AbstractMetadata* rendererMetadata( QString rendererName );

    //! return a list of available renderers
    QStringList renderersList();

  protected:
    //! protected constructor
    QgsRendererV2Registry();
    ~QgsRendererV2Registry();

    static QgsRendererV2Registry* mInstance;

    QMap<QString, QgsRendererV2AbstractMetadata*> mRenderers;

    //! list to keep order in which renderers have been added
    QStringList mRenderersOrder;
};

#endif // QGSRENDERERV2REGISTRY_H
