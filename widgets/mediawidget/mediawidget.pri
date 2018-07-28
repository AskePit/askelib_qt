SOURCES += mediawidget/mediawidget.cpp

HEADERS += mediawidget/mediawidget.h \
    mediawidget/config.h

!isEmpty(VLC_PATH) {
    SOURCES +=  mediawidget/videoplayer.cpp

    HEADERS += mediawidget/videoplayer.h \
        mediawidget/ui_videoplayer.h
}
