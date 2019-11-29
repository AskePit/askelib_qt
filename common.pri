CONFIG(debug, debug|release) {
    ASKELIBQT_BUILD_FLAG = debug
    ASKELIBQT_LIB_SUFFIX = d
} else {
    ASKELIBQT_BUILD_FLAG = release
}

ASKELIBQT_LIB_PATH = $$PWD/libs/
ASKELIBQT_INCLUDE_PATH = $$PWD
