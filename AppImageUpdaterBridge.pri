INCLUDEPATH += $$PWD $$PWD/include
QT += core widgets network
HEADERS += $$PWD/include/AppImageUpdateInformation_p.hpp \
       $$PWD/include/ZsyncRemoteControlFileParser_p.hpp \
       $$PWD/include/ZsyncInternalStructures_p.hpp \
       $$PWD/include/ZsyncWriter_p.hpp \
       $$PWD/include/ZsyncBlockRangeReply_p.hpp \
           $$PWD/include/ZsyncBlockRangeDownloader_p.hpp \
       $$PWD/include/AppImageDeltaRevisioner_p.hpp \
       $$PWD/include/AppImageDeltaRevisioner.hpp \
           $$PWD/include/AppImageUpdaterDialog.hpp \
       $$PWD/include/AppImageUpdaterBridgeErrorCodes.hpp \
       $$PWD/include/AppImageUpdaterBridgeStatusCodes.hpp \
           $$PWD/include/AppImageUpdaterBridge.hpp

SOURCES += $$PWD/src/AppImageUpdateInformation_p.cc \
       $$PWD/src/ZsyncRemoteControlFileParser_p.cc \
       $$PWD/src/ZsyncWriter_p.cc \
           $$PWD/src/ZsyncBlockRangeReply_p.cc \
           $$PWD/src/ZsyncBlockRangeDownloader_p.cc \
       $$PWD/src/AppImageDeltaRevisioner_p.cc \
           $$PWD/src/AppImageDeltaRevisioner.cc \
           $$PWD/src/AppImageUpdaterDialog.cc

logging_disabled {
	message(Logging will be disabled for this build.)
	DEFINES += LOGGING_DISABLED
}
