INCLUDEPATH += $$system(pwd) $$system(pwd)/include
QT += core widgets network
HEADERS += $$system(pwd)/include/AppImageUpdateInformation_p.hpp \ 
	   $$system(pwd)/include/ZsyncRemoteControlFileParser_p.hpp \
	   $$system(pwd)/include/ZsyncInternalStructures_p.hpp \
	   $$system(pwd)/include/ZsyncWriter_p.hpp \
	   $$system(pwd)/include/ZsyncBlockRangeReply_p.hpp \
           $$system(pwd)/include/ZsyncBlockRangeDownloader_p.hpp \
           $$system(pwd)/include/AppImageDeltaRevisioner.hpp \
           $$system(pwd)/include/AppImageUpdaterBridge.hpp  

SOURCES += $$system(pwd)/src/AppImageUpdateInformation_p.cc \
	   $$system(pwd)/src/ZsyncRemoteControlFileParser_p.cc \
	   $$system(pwd)/src/ZsyncWriter_p.cc \
           $$system(pwd)/src/ZsyncBlockRangeReply_p.cc \
           $$system(pwd)/src/ZsyncBlockRangeDownloader_p.cc \
           $$system(pwd)/src/AppImageDeltaRevisioner.cc

without_widgets {
	message(AppImage Updater Widget will not be available for this build.)
	QT -= widgets
} else {
	HEADERS += $$system(pwd)/include/AppImageUpdaterWidget.hpp
	SOURCES += $$system(pwd)/src/AppImageUpdaterWidget.cc
	DEFINES += WITH_WIDGETS
}

logging_disabled {
	message(Logging will be disabled for this build.)
	DEFINES += LOGGING_DISABLED
}
