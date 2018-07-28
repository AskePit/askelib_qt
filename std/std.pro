include( ../common.pri )

DESTDIR = $${ASKELIB_QT_LIB_PATH}

TEMPLATE = lib
CONFIG += staticlib
TARGET = askelib_qt_std$${ASKELIB_QT_LIB_SUFFIX}

# Input
HEADERS += \
    fs.h \
    pitm/pitmarray.h \
    pitm/pitmdocument.h \
    pitm/pitmobject.h \
    pitm/pitmvalue.h

SOURCES += \
    fs.cpp \
    pitm/pitmarray.cpp \
    pitm/pitmdocument.cpp \
    pitm/pitmobject.cpp \
    pitm/pitmvalue.cpp \
    pitm/pitmparser.cpp \
    pitm/pitmwriter.cpp \
    pitm/pitm.cpp
