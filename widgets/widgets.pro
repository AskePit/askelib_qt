include( ../common.pri )
include( ../config.pri )

DESTDIR = $${ASKELIB_QT_LIB_PATH}

QT += core gui widgets

TEMPLATE = lib
CONFIG += staticlib
TARGET = askelib_qt_widgets$${ASKELIB_QT_LIB_SUFFIX}

include( ../askelib/public.pri )

# for std/ includes
INCLUDEPATH += ..
INCLUDEPATH += $${ASKE_INCLUDE_PATH}

LIBS += -L$${ASKE_LIB_PATH} -laskelib_std$${ASKE_LIB_SUFFIX}

include( texteditor/texteditor.pri )
include( mediawidget/mediawidget.pri )

!isEmpty(VLC_PATH) {
    INCLUDEPATH += $${VLC_PATH}/include
}
