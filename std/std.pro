include( ../common.pri )

DESTDIR = $${ASKELIBQT_LIB_PATH}

TEMPLATE = lib
CONFIG += staticlib
TARGET = askelib_qt_std$${ASKELIBQT_LIB_SUFFIX}

# Input
HEADERS += \
    fs.h

SOURCES += \
    fs.cpp
