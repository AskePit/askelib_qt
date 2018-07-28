CONFIG(debug, debug|release) {
    ASKELIB_QT_BUILD_FLAG = debug
    ASKELIB_QT_LIB_SUFFIX = d
} else {
    ASKELIB_QT_BUILD_FLAG = release
}

ASKELIB_QT_LIB_PATH = $$PWD/libs/
ASKELIB_QT_INCLUDE_PATH = $$PWD
