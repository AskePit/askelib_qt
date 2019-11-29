include( ../common.pri )

DESTDIR = $${ASKELIBQT_LIB_PATH}

TEMPLATE = lib
CONFIG += staticlib
TARGET = askelib_qt_std$${ASKELIBQT_LIB_SUFFIX}

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
