INCLUDEPATH += $$PWD $$PWD/include
QT += core widgets network
CONFIG += staticlib
HEADERS += \
    $$PWD/include/appimageupdateinformation_p.hpp \
    $$PWD/include/zsyncremotecontrolfileparser_p.hpp \
    $$PWD/include/zsyncinternalstructures_p.hpp \
    $$PWD/include/zsyncwriter_p.hpp \
    $$PWD/include/rangereply_p.hpp \
    $$PWD/include/rangedownloader_p.hpp \
    $$PWD/include/rangedownloader.hpp \
    $$PWD/include/qappimageupdate_enums.hpp \
    $$PWD/include/qappimageupdate.hpp \
    $$PWD/include/qappimageupdate_p.hpp
    $$PWD/include/helpers_p.hpp

SOURCES += \
    $$PWD/src/appimageupdateinformation_p.cc \
    $$PWD/src/zsyncremotecontrolfileparser_p.cc \
    $$PWD/src/zsyncwriter_p.cc \
    $$PWD/src/rangereply_p.cc \
    $$PWD/src/rangedownloader_p.cc \
    $$PWD/src/rangedownloader.cc \
    $$PWD/src/qappimageupdate_enums.cc \
    $$PWD/src/qappimageupdate.cc \
    $$PWD/src/qappimageupdate_p.cc \
    $$PWD/src/helpers_p.cc


# FORMS += $$PWD/src/AppImageUpdaterDialog.ui \
#         $$PWD/include/SoftwareUpdateDialog.ui

#NO_GUI {
#	message(QAppImageUpdate widgets will be disabled for this build.)
#	QT -= widgets
#	HEADERS -= $$PWD/include/appimageupdaterdialog_p.hpp
#       HEADERS -= $$PWD/include/softwareupdatedialog_p.hpp
#	SOURCES -= $$PWD/src/appimageupdaterdialog_p.cc
#	SOURCES -= $$PWD/src/softwareupdatedialog_p.cc
#
#       FORMS -= $$PWD/src/AppImageUpdaterDialog.ui \
#                $$PWD/include/SoftwareUpdateDialog.ui
#}

LOGGING_DISABLED {
	message(Logging will be disabled for this build.)
	DEFINES += LOGGING_DISABLED
}

BUILD_AS_PLUGIN {
	message(QAppImageUpdate will be built as an Qt Plugin)
	CONFIG -= staticlib
	CONFIG += plugin
	DEFINES += BUILD_AS_PLUGIN
	OTHER_FILES = $$PWD/QAppImageUpdate.json
}
