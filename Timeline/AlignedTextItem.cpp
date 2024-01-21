#include "AlignedTextItem.h"

AlignedTextItem::AlignedTextItem(QGraphicsItem *parent, QGraphicsScene *scene) : QGraphicsTextItem(parent)
{
    myBoundRect.setRect(0, 0, 0, 0);
    textOp.setAlignment(Qt::AlignCenter);
    textOp.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
}

void AlignedTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(this->defaultTextColor());
    painter->drawText(boundingRect(), text, textOp);
    QGraphicsTextItem::paint(painter, option, widget);
}

QRectF AlignedTextItem::boundingRect() const
{
    return myBoundRect;
}

void AlignedTextItem::setBoundingRect( qreal x, qreal y, qreal w, qreal h)
{
    myBoundRect.setRect( x, y, w, h );
}

void AlignedTextItem::setText( const QString &inText )
{
    text = inText;
    update();
}

void AlignedTextItem::setAligment( const Qt::Alignment flags ) {
    textOp.setAlignment(flags);
}


