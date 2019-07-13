INCLUDEPATH += $$PWD
QT += core widgets network
HEADERS += \
    $$PWD/include/appimageupdateinformation_p.hpp \
    $$PWD/include/zsyncremotecontrolfileparser_p.hpp \
    $$PWD/include/zsyncinternalstructures_p.hpp \
    $$PWD/include/zsyncwriter_p.hpp \
    $$PWD/include/zsyncblockrangereply_p.hpp \
    $$PWD/include/zsyncblockrangedownloader_p.hpp \
    $$PWD/include/appimagedeltarevisioner_p.hpp \
    $$PWD/include/appimagedeltarevisioner.hpp \
    $$PWD/include/appimageupdaterbridge_enums.hpp \
    $$PWD/include/appimageupdaterbridge.hpp \
    $$PWD/include/appimageupdaterdialog.hpp \
    $$PWD/include/softwareupdatedialog_p.hpp \
    $$PWD/include/helpers_p.hpp

SOURCES += \
    $$PWD/src/appimageupdateinformation_p.cc \
    $$PWD/src/zsyncremotecontrolfileparser_p.cc \
    $$PWD/src/zsyncwriter_p.cc \
    $$PWD/src/zsyncblockrangereply_p.cc \
    $$PWD/src/zsyncblockrangedownloader_p.cc \
    $$PWD/src/appimagedeltarevisioner_p.cc \
    $$PWD/src/appimagedeltarevisioner.cc \
    $$PWD/src/appimageupdaterdialog.cc \
    $$PWD/src/appimageupdaterbridge_enums.cc \
    $$PWD/src/softwareupdatedialog_p.cc \ 
    $$PWD/src/helpers_p.cc

# FORMS += $$PWD/include/AppImageUpdaterDialog.ui \
#         $$PWD/include/SoftwareUpdateDialog.ui

NO_GUI {
	message(AppImage Updater Bridge widgets will be disabled for this build.)
	QT -= widgets
	HEADERS -= $$PWD/include/appimageupdaterdialog.hpp
        HEADERS -= $$PWD/include/softwareupdatedialog_p.hpp
	SOURCES -= $$PWD/src/appimageupdaterdialog.cc
	SOURCES -= $$PWD/src/softwareupdatedialog_p.cc

#       FORMS -= $$PWD/include/AppImageUpdaterDialog.ui \
#                $$PWD/include/SoftwareUpdateDialog.ui
}

LOGGING_DISABLED {
	message(Logging will be disabled for this build.)
	DEFINES += LOGGING_DISABLED
}
