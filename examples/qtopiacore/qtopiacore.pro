TEMPLATE      = subdirs
SUBDIRS       = framebuffer mousecalibration 

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtopiacore
INSTALLS += sources
