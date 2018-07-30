INCLUDEPATH += $$system(pwd) $$system(pwd)/include
QT += core concurrent network
HEADERS += $$system(pwd)/include/AppImageUpdaterBridge.hpp \
	   $$system(pwd)/include/AppImageUpdateInformation_p.hpp \ 
	   $$system(pwd)/include/AppImageUpdateInformation.hpp \
#	   $$system(pwd)/include/ZsyncCore_p.hpp \
	   $$system(pwd)/include/ZsyncRemoteControlFileParser_p.hpp \
	   $$system(pwd)/include/ZsyncInternalStructures_p.hpp \
	   $$system(pwd)/include/AppImageInspector.hpp

SOURCES += $$system(pwd)/src/AppImageUpdaterBridge.cc \
	   $$system(pwd)/src/AppImageUpdateInformation_p.cc \
	   $$system(pwd)/src/AppImageUpdateInformation.cc \
#	   $$system(pwd)/src/ZsyncCore_p.cc \
	   $$system(pwd)/src/ZsyncRemoteControlFileParser_p.cc \
	   $$system(pwd)/src/AppImageInspector.cc

