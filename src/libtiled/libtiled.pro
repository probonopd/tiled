include(../../tiled.pri)

TEMPLATE = lib
TARGET = tiled
target.path = $${LIBDIR}
INSTALLS += target
macx {
    DESTDIR = ../../bin/Tiled.app/Contents/Frameworks
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
} else {
    DESTDIR = ../../lib
}
DLLDESTDIR = ../..

win32 {
    QMAKE_PROJECT_NAME = libtiled
} else {
    # On other platforms it is necessary to link to zlib explicitly
    LIBS += -lz
}

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
DEFINES += TILED_LIBRARY

!isEmpty(TILED_LINUX_ARCHIVE):DEFINES += TILED_LINUX_ARCHIVE

contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols

SOURCES += compression.cpp \
    filesystemwatcher.cpp \
    gidmapper.cpp \
    grouplayer.cpp \
    hexagonalrenderer.cpp \
    imagelayer.cpp \
    imagereference.cpp \
    isometricrenderer.cpp \
    layer.cpp \
    map.cpp \
    mapobject.cpp \
    mapreader.cpp \
    maprenderer.cpp \
    maptovariantconverter.cpp \
    mapwriter.cpp \
    objectgroup.cpp \
    orthogonalrenderer.cpp \
    plugin.cpp \
    pluginmanager.cpp \
    properties.cpp \
    savefile.cpp \
    staggeredrenderer.cpp \
    tile.cpp \
    tileanimationdriver.cpp \
    tilelayer.cpp \
    tileset.cpp \
    tilesetformat.cpp \
    tilesetmanager.cpp \
    varianttomapconverter.cpp
HEADERS += compression.h \
    filesystemwatcher.h \
    gidmapper.h \
    grouplayer.h \
    hexagonalrenderer.h \
    imagelayer.h \
    imagereference.h \
    isometricrenderer.h \
    layer.h \
    logginginterface.h \
    map.h \
    mapformat.h \
    mapobject.h \
    mapreader.h \
    maprenderer.h \
    maptovariantconverter.h \
    mapwriter.h \
    object.h \
    objectgroup.h \
    orthogonalrenderer.h \
    plugin.h \
    pluginmanager.h \
    properties.h \
    savefile.h \
    staggeredrenderer.h \
    terrain.h \
    tile.h \
    tileanimationdriver.h \
    tiled.h \
    tiled_global.h \
    tilelayer.h \
    tileset.h \
    tilesetformat.h \
    tilesetmanager.h \
    varianttomapconverter.h

contains(INSTALL_HEADERS, yes) {
    headers.files = $${HEADERS}
    headers.path = $${PREFIX}/include/tiled
    INSTALLS += headers
}
