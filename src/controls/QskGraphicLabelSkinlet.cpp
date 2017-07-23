/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#include "QskGraphicLabelSkinlet.h"
#include "QskGraphicLabel.h"
#include "QskGraphic.h"
#include "QskGraphicTextureFactory.h"
#include "QskAspect.h"
#include "QskSkin.h"
#include "QskTextureNode.h"
#include "QskFunctions.h"

QskGraphicLabelSkinlet::QskGraphicLabelSkinlet( QskSkin* skin ):
    Inherited( skin )
{
    setNodeRoles( { GraphicRole } );
}

QskGraphicLabelSkinlet::~QskGraphicLabelSkinlet() = default;

QRectF QskGraphicLabelSkinlet::subControlRect(
    const QskSkinnable* skinnable, QskAspect::Subcontrol subControl ) const
{
    const auto label = static_cast< const QskGraphicLabel* >( skinnable );

    if ( subControl == QskGraphicLabel::Graphic )
    {
        return graphicRect( label );
    }

    return Inherited::subControlRect( skinnable, subControl );
}

QRect QskGraphicLabelSkinlet::graphicRect( const QskGraphicLabel* label ) const
{
    // textures are in integers, to avoid useless recalculations
    // that finally will be rounded anyway, we calculate in integers

    const auto fillMode = label->fillMode();

    const QRect graphicRect = label->contentsRect().toAlignedRect();

    if ( fillMode == QskGraphicLabel::Stretch )
    {
        return graphicRect;
    }

    QSizeF sz = label->effectiveSourceSize();

    if ( fillMode == QskGraphicLabel::PreserveAspectFit )
    {
        sz.scale( graphicRect.size(), Qt::KeepAspectRatio );
    }
    else if ( fillMode == QskGraphicLabel::PreserveAspectCrop )
    {
        sz.scale( graphicRect.size(), Qt::KeepAspectRatioByExpanding );
    }

    return qskAlignedRect( graphicRect,
        ( int )sz.width(), ( int )sz.height(), label->alignment() );
}

QSGNode* QskGraphicLabelSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    const auto label = static_cast< const QskGraphicLabel* >( skinnable );

    switch( nodeRole )
    {
        case GraphicRole:
            return updateGraphicNode( label, node );

        default:
            return nullptr;
    }
}

QSGNode* QskGraphicLabelSkinlet::updateGraphicNode(
    const QskGraphicLabel* label, QSGNode* node ) const
{
    const QRectF rect = subControlRect( label, QskGraphicLabel::Graphic );

    node = QskSkinlet::updateGraphicNode( label, node,
        label->graphic(), label->graphicFilter(), rect, Qt::AlignCenter );

    if ( node && label->mirror() )
    {
        auto textureNode = static_cast< QskTextureNode* >( node );
        textureNode->setMirrored( Qt::Horizontal );
    }

    return node;
}

#include "moc_QskGraphicLabelSkinlet.cpp"