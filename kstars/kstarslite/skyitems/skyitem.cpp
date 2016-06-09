/** *************************************************************************
                          skyitem.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 02/05/2016
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
#include "skyitem.h"
#include "../../skymaplite.h"
#include "rootnode.h"
#include "skynodes/skynode.h"

SkyItem::SkyItem(RootNode* parent)
    :m_rootNode(parent)
{
    parent->appendChildNode(this);
}

void SkyItem::hide() {
    setOpacity(0);
    markDirty(QSGNode::DirtyOpacity);
}

void SkyItem::show() {
    setOpacity(1);
    markDirty(QSGNode::DirtyOpacity);
}
