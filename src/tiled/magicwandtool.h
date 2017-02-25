/*
 * magicwandtool.h
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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


#pragma once


#include "abstracttiletool.h"

#include "tilelayer.h"

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * Implements a tool that selects a region of connected similar tiles on the layer.
 */
class MagicWandTool : public AbstractTileTool
{
    Q_OBJECT

public:
    MagicWandTool(QObject *parent = nullptr);

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

protected:
    void tilePositionChanged(const QPoint &tilePos) override;

private:

    QRegion mSelectedRegion;
};

} // namespace Internal
} // namespace Tiled
