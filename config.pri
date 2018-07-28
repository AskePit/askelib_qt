# function tries to find askelib.config file somwhere near askelib folder
defineReplace(findAskelibQtConfig) {
    for(level, 1..4) {
        level_str = $$level_str"/.."
        path = $$PWD$${level_str}/askelib_qt.config
        exists( $$clean_path($$path) ) {
            #message("found askelib_qt.config!")
            return($$path)
        }
    }

    return("")
}

!defined(VLC_PATH) {
    askelib_qt_config = $$findAskelibQtConfig()

    # parse askelib_qt.config file in case of it's presence
    !isEmpty(askelib_qt_config) {
        lines = $$cat($$askelib_qt_config, lines)
        for (line, lines) {
            # vlc_use option
            t = $$lower( $$replace(line, "^vlc_use\\s*=\\s*(\\S+)$", "\\1") )
            !isEqual(t, $$line) {
                # vlc_path option
                t = $$replace(line, "^vlc_path\\s*=\\s*(\\S+)$", "\\1")
                !isEqual(t, $$line) {
                    DEFINES *= ASKELIB_QT_USE_VLC
                    VLC_PATH = $$clean_path($$t)
                    #message("libvlc is defined here: $${VLC_PATH}")
                    LIBS += -L$${VLC_PATH}/lib -llibvlccore -llibvlc
                }
            }
        }
    }
}
