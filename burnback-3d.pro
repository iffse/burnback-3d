# QT += qml
QT += quick
QT += widgets
CONFIG += c++17

QMAKE_LINK=clang++
QMAKE_CXX = clang++
CONFIG(sanitizer) {
	message("Sanitizer enabled")
	CONFIG += sanitizer sanitize_address sanitize_undefined
}


CONFIG(debug, debug|release) {
	DESTDIR = target/debug
	DEFINES += DEBUG
} else {
	DESTDIR = target/release
	DEFINES += RELEASE
}

INCLUDEPATH += include

OBJECTS_DIR = $$DESTDIR/objects
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc
UI_DIR = $$DESTDIR/ui

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

HEADERS += \
	./src/headers/types.h \
	./src/headers/globals.h \
	./src/headers/iosystem.h \
	./src/headers/operations.h \
	./src/headers/interface.h \
	./src/headers/plotData.h
SOURCES += \
	./src/main.cpp \
	./src/iosystem.cpp \
	./src/operations.cpp \
	./src/interface.cpp \
	./src/plotData.cpp
RESOURCES += src-qml/qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
