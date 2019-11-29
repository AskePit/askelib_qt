include( ../common.pri )
include( ../config.pri )

DESTDIR = $${ASKELIBQT_LIB_PATH}

QT += core gui widgets

TEMPLATE = lib
CONFIG += staticlib
TARGET = askelib_qt_widgets$${ASKELIBQT_LIB_SUFFIX}

include( ../askelib/public.pri )

# for std/ includes
INCLUDEPATH += ..
INCLUDEPATH += $${ASKELIB_INCLUDE_PATH}

LIBS += -L$${ASKELIB_LIB_PATH} -laskelib_std$${ASKELIB_LIB_SUFFIX}

include( texteditor/texteditor.pri )
include( mediawidget/mediawidget.pri )

!isEmpty(VLC_PATH) {
    INCLUDEPATH += $${VLC_PATH}/include
}
