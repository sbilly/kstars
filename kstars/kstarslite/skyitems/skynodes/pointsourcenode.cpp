/** *************************************************************************
                          pointsourcenode.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 16/05/2016
    copyright            : (C) 2016 by Artem Fedoskin
    email                : afedoskin3@gmail.com
 ***************************************************************************/
/** *************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "skyobject.h"
#include "Options.h"

#include <QSGSimpleTextureNode>

#include "pointsourcenode.h"
#include "nodes/pointnode.h"

PointSourceNode::PointSourceNode(SkyObject * skyObject, RootNode* p, char sp, float size)
    :SkyNode(skyObject), m_point(0), m_sizeMagLim(10.) // has to be changed when stars will be introduced
{
    m_point = new PointNode(p,starWidth(size),sp);
    appendChildNode(m_point);
}

float PointSourceNode::starWidth(float mag) const
{
    //adjust maglimit for ZoomLevel
    const double maxSize = 10.0;

    double lgmin = log10(MINZOOM);
//    double lgmax = log10(MAXZOOM);
    double lgz = log10(Options::zoomFactor());

    float sizeFactor = maxSize + (lgz - lgmin);

    float size = ( sizeFactor*( m_sizeMagLim - mag ) / m_sizeMagLim ) + 1.;
    if( size <= 1.0 ) size = 1.0;
    if( size > maxSize ) size = maxSize;

    return size;
}

void PointSourceNode::changePos(QPointF pos) {
    QSizeF size = m_point->size();
    QMatrix4x4 m (1,0,0,pos.x(),
                  0,1,0,pos.y(),
                  0,0,1,0,
                  0,0,0,1);
    m.translate(-0.5*size.width(), -0.5*size.height());

    setMatrix(m);
    markDirty(QSGNode::DirtyMatrix);
}

void PointSourceNode::update() {
    if( !projector()->checkVisibility(m_skyObject) ) {
        m_point->hide();
        return;
    }

    bool visible = false;
    QPointF pos = projector()->toScreen(m_skyObject,true,&visible);
    if( visible && projector()->onScreen(pos) ) { // FIXME: onScreen here should use canvas size rather than SkyMap size, especially while printing in portrait mode!
        m_point->setSize(starWidth(m_skyObject->mag()));
        changePos(pos);
        m_point->show();
    } else {
        m_point->hide();
    }
}

void PointSourceNode::hide() {
    m_point->hide();
}
