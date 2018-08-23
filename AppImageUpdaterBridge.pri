INCLUDEPATH += $$system(pwd) $$system(pwd)/include
QT += core concurrent network
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
