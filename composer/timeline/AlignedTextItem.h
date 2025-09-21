#ifndef ALIGNEDTEXTITEM_H
#define ALIGNEDTEXTITEM_H

#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QTextOption>

class AlignedTextItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    enum { Type = UserType + 3 };
    AlignedTextItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    void setBoundingRect( qreal x, qreal y, qreal w, qreal h);
    void setText( const QString &inText );
    void setAligment( const Qt::Alignment flags );

signals:

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QRectF boundingRect() const;

private:
    QRectF myBoundRect;
    QTextOption textOp;
    QString text;
};

#endif // ALIGNEDTEXTITEM_H
