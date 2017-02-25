/*
 * tilesetdocument.cpp
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilesetdocument.h"

#include "mapdocument.h"
#include "map.h"
#include "terrain.h"
#include "tile.h"
#include "tilesetmanager.h"
#include "tilesetterrainmodel.h"
#include "tmxmapformat.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QUndoStack>

namespace Tiled {
namespace Internal {

class ReloadTileset : public QUndoCommand
{
public:
    ReloadTileset(TilesetDocument *tilesetDocument, const SharedTileset &tileset)
        : mTilesetDocument(tilesetDocument)
        , mTileset(tileset)
    {
        setText(QCoreApplication::translate("Undo Commands", "Reload Tileset"));
    }

    void undo() override { mTilesetDocument->swapTileset(mTileset); }
    void redo() override { mTilesetDocument->swapTileset(mTileset); }

private:
    TilesetDocument *mTilesetDocument;
    SharedTileset mTileset;
};


TilesetDocument::TilesetDocument(const SharedTileset &tileset, const QString &fileName)
    : Document(TilesetDocumentType, fileName)
    , mTileset(tileset)
    , mTerrainModel(new TilesetTerrainModel(this, this))
{
    mCurrentObject = tileset.data();

    // warning: will need to be kept up-to-date
    mFileName = tileset->fileName();

    connect(mTerrainModel, &TilesetTerrainModel::terrainAboutToBeAdded,
            this, &TilesetDocument::onTerrainAboutToBeAdded);
    connect(mTerrainModel, &TilesetTerrainModel::terrainAdded,
            this, &TilesetDocument::onTerrainAdded);
    connect(mTerrainModel, &TilesetTerrainModel::terrainAboutToBeRemoved,
            this, &TilesetDocument::onTerrainAboutToBeRemoved);
    connect(mTerrainModel, &TilesetTerrainModel::terrainRemoved,
            this, &TilesetDocument::onTerrainRemoved);

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReference(tileset);
}

TilesetDocument::~TilesetDocument()
{
    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->removeReference(mTileset);
}

bool TilesetDocument::save(const QString &fileName, QString *error)
{
    TilesetFormat *tilesetFormat = mWriterFormat;

    TsxTilesetFormat tsxTilesetFormat;
    if (!tilesetFormat)
        tilesetFormat = &tsxTilesetFormat;

    // todo: workaround to avoid it writing the tileset like an extenal tileset reference
    mTileset->setFileName(QString());

    if (!tilesetFormat->write(*tileset(), fileName)) {
        if (error)
            *error = tilesetFormat->errorString();
        return false;
    }

    undoStack()->setClean();
    setFileName(fileName);
    mLastSaved = QFileInfo(fileName).lastModified();

    emit saved();
    return true;
}

bool TilesetDocument::reload(QString *error)
{
    auto format = mReaderFormat;
    if (!format)
        format = mWriterFormat;

    // Either the file was saved or it was loaded, so now we must have a format
    Q_ASSERT(format);

    SharedTileset tileset = format->read(fileName());

    if (tileset.isNull()) {
        if (error)
            *error = format->errorString();
        return false;
    }

    mUndoStack->push(new ReloadTileset(this, tileset));
    mUndoStack->setClean();
    mLastSaved = QFileInfo(fileName()).lastModified();

    return true;
}

TilesetDocument *TilesetDocument::load(const QString &fileName,
                                       TilesetFormat *format,
                                       QString *error)
{
    SharedTileset tileset = format->read(fileName);

    if (tileset.isNull()) {
        if (error)
            *error = format->errorString();
        return nullptr;
    }

    auto *document = new TilesetDocument(tileset, fileName);
    document->setReaderFormat(format);
    if (format->hasCapabilities(MapFormat::Write))
        document->setWriterFormat(format);

    return document;
}

TilesetFormat *TilesetDocument::readerFormat() const
{
    return mReaderFormat;
}

void TilesetDocument::setReaderFormat(TilesetFormat *format)
{
    mReaderFormat = format;
}

FileFormat *TilesetDocument::writerFormat() const
{
    return mWriterFormat;
}

void TilesetDocument::setWriterFormat(TilesetFormat *format)
{
    mWriterFormat = format;
}

QString TilesetDocument::displayName() const
{
    QString displayName;

    if (isEmbedded()) {
        MapDocument *mapDocument = mMapDocuments.first();
        displayName = mapDocument->displayName();
        displayName += QLatin1String("#");
        displayName += mTileset->name();
    } else {
        displayName = QFileInfo(mFileName).fileName();
        if (displayName.isEmpty())
            displayName = tr("untitled.tsx");
    }

    return displayName;
}

/**
 * Exchanges the tileset data of the tileset wrapped by this document with the
 * data in the given \a tileset, and vica-versa.
 */
void TilesetDocument::swapTileset(SharedTileset &tileset)
{
    // Bring pointers to safety
    setSelectedTiles(QList<Tile*>());
    setCurrentObject(mTileset.data());

    mTileset->swap(*tileset);
    emit tilesetChanged(mTileset.data());
}

/**
 * Used when a map that has this tileset embedded is saved.
 */
void TilesetDocument::setClean()
{
    undoStack()->setClean();
}

void TilesetDocument::addMapDocument(MapDocument *mapDocument)
{
    Q_ASSERT(!mMapDocuments.contains(mapDocument));
    mMapDocuments.append(mapDocument);
}

void TilesetDocument::removeMapDocument(MapDocument *mapDocument)
{
    Q_ASSERT(mMapDocuments.contains(mapDocument));
    mMapDocuments.removeOne(mapDocument);
}

void TilesetDocument::setTilesetName(const QString &name)
{
    mTileset->setName(name);
    emit tilesetNameChanged(mTileset.data());

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetNameChanged(mTileset.data());
}

void TilesetDocument::setTilesetTileOffset(const QPoint &tileOffset)
{
    mTileset->setTileOffset(tileOffset);

    // Invalidate the draw margins of the maps using this tileset
    for (MapDocument *mapDocument : mapDocuments())
        mapDocument->map()->invalidateDrawMargins();

    emit tilesetTileOffsetChanged(mTileset.data());
}

void TilesetDocument::addTiles(const QList<Tile *> &tiles)
{
    mTileset->addTiles(tiles);
    emit tilesetChanged(mTileset.data());
}

void TilesetDocument::removeTiles(const QList<Tile *> &tiles)
{
    // Switch current object to the tileset when it is one of the removed tiles
    for (Tile *tile : tiles) {
        if (tile == currentObject()) {
            setCurrentObject(mTileset.data());
            break;
        }
    }

    mTileset->removeTiles(tiles);
    emit tilesetChanged(mTileset.data());
}

void TilesetDocument::setSelectedTiles(const QList<Tile*> &selectedTiles)
{
    mSelectedTiles = selectedTiles;
    emit selectedTilesChanged();
}

QList<Object *> TilesetDocument::currentObjects() const
{
    if (mCurrentObject->typeId() == Object::TileType && !mSelectedTiles.isEmpty()) {
        QList<Object*> objects;
        for (Tile *tile : mSelectedTiles)
            objects.append(tile);
        return objects;
    }

    return Document::currentObjects();
}

void TilesetDocument::onTerrainAboutToBeAdded(Tileset *tileset, int terrainId)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainAboutToBeAdded(tileset, terrainId);
}

void TilesetDocument::onTerrainAdded(Tileset *tileset, int terrainId)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainAdded(tileset, terrainId);
}

void TilesetDocument::onTerrainAboutToBeRemoved(Terrain *terrain)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainAboutToBeRemoved(mTileset.data(), terrain);
}

void TilesetDocument::onTerrainRemoved(Terrain *terrain)
{
    if (terrain == mCurrentObject)
        setCurrentObject(nullptr);

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainRemoved(mTileset.data(), terrain);
}

} // namespace Internal
} // namespace Tiled
