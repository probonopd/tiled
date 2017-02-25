/*
 * changemapobject.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "changemapobject.h"

#include "mapdocument.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeMapObject::ChangeMapObject(MapDocument *mapDocument,
                                 MapObject *mapObject,
                                 MapObject::Property property,
                                 const QVariant &value)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Object"))
    , mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mProperty(property)
    , mValue(value)
{
    switch (property) {
    case MapObject::VisibleProperty:
        if (value.toBool())
            setText(QCoreApplication::translate("Undo Commands", "Show Object"));
        else
            setText(QCoreApplication::translate("Undo Commands", "Hide Object"));
        break;
    default:
        break;
    }
}

void ChangeMapObject::swap()
{
    QVariant oldValue = mMapObject->mapObjectProperty(mProperty);
    mMapDocument->mapObjectModel()->setObjectProperty(mMapObject, mProperty, mValue);
    std::swap(mValue, oldValue);
}


ChangeMapObjectCells::ChangeMapObjectCells(MapDocument *mapDocument,
                                           const QVector<MapObjectCell> &changes,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapObjectModel(mapDocument->mapObjectModel())
    , mChanges(changes)
{
}

static QList<MapObject*> objectList(const QVector<MapObjectCell> &changes)
{
    QList<MapObject*> result;
    result.reserve(changes.size());

    for (const MapObjectCell &change : changes)
        result.append(change.object);

    return result;
}

void ChangeMapObjectCells::swap()
{
    for (MapObjectCell &change : mChanges) {
        auto cell = change.object->cell();
        change.object->setCell(change.cell);
        change.cell = cell;
    }
    emit mMapObjectModel->objectsChanged(objectList(mChanges));
}
