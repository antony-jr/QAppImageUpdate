INCLUDEPATH += $$system(pwd) $$system(pwd)/include
QT += core widgets network
HEADERS += $$system(pwd)/include/AppImageUpdateInformation_p.hpp \ 
	   $$system(pwd)/include/ZsyncRemoteControlFileParser_p.hpp \
	   $$system(pwd)/include/ZsyncInternalStructures_p.hpp \
	   $$system(pwd)/include/ZsyncWriter_p.hpp \
	   $$system(pwd)/include/ZsyncBlockRangeReply_p.hpp \
           $$system(pwd)/include/ZsyncBlockRangeDownloader_p.hpp \
	   $$system(pwd)/include/AppImageDeltaRevisioner_p.hpp \
	   $$system(pwd)/include/AppImageDeltaRevisioner.hpp \
           $$system(pwd)/include/AppImageUpdaterWidget.hpp \
	   $$system(pwd)/include/AppImageUpdaterBridgeErrorCodes.hpp \
	   $$system(pwd)/include/AppImageUpdaterBridgeStatusCodes.hpp \
           $$system(pwd)/include/AppImageUpdaterBridge.hpp

SOURCES += $$system(pwd)/src/AppImageUpdateInformation_p.cc \
	   $$system(pwd)/src/ZsyncRemoteControlFileParser_p.cc \
	   $$system(pwd)/src/ZsyncWriter_p.cc \
           $$system(pwd)/src/ZsyncBlockRangeReply_p.cc \
           $$system(pwd)/src/ZsyncBlockRangeDownloader_p.cc \
	   $$system(pwd)/src/AppImageDeltaRevisioner_p.cc \
           $$system(pwd)/src/AppImageDeltaRevisioner.cc \
           $$system(pwd)/src/AppImageUpdaterWidget.cc

logging_disabled {
	message(Logging will be disabled for this build.)
	DEFINES += LOGGING_DISABLED
}
