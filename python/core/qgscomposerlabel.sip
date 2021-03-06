/** \ingroup MapComposer
 * A label that can be placed onto a map composition.
 */
class QgsComposerLabel: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerlabel.h"
%End
  public:
    QgsComposerLabel( QgsComposition *composition /TransferThis/);
    ~QgsComposerLabel();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**resizes the widget such that the text fits to the item. Keeps top left point*/
    void adjustSizeToText();

    QString text();
    void setText( const QString& text );

     /**Returns the text as it appears on screen (with replaced data field)
      @note this function was added in version 1.2*/
    QString displayText() const;
    /**Get font color
      @note: this function was added in version 1.4*/
    QFont font() const;
    void setFont( const QFont& f );
    double margin();
    void setMargin( double m );

    /**Sets text color
    @note: this function was added in version 1.4*/
    void setFontColor( const QColor& c);

    QColor fontColor() const;

    /** stores state in Dom node
       * @param node is Dom node corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param node is Dom node corresponding to 'ComposerLabel' tag
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Get label identification number
      @note this method was added in version 1.7*/
    QString id() const;

    /**Set label identification number
      @note this method was added in version 1.7*/
    void setId( const QString& id );
};
