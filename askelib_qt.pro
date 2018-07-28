TEMPLATE = subdirs

SUBDIRS += \
    askelib \
    std \
    widgets

askelib.subdir = askelib
std.subdir = std
widgets.subdir = widgets

widgets.depends = askelib
