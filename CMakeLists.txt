# BSD-3 Clause.
# Copyright (C) 2018 Antony Jr.
# -------------------------------

CMAKE_MINIMUM_REQUIRED( VERSION 3.2)
project(AppImageUpdaterBridge)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
option(LG "LOGGING_DISABLED" OFF)

# Let cmake know that this is a release build.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find the QtCore library.
find_package(Qt5Core)
find_package(Qt5Network)
find_package(Qt5Widgets)

# Include Directories.
include_directories(.)
include_directories(include)


add_library(AppImageUpdaterBridge src/AppImageDeltaRevisioner.cc
       				  src/AppImageDeltaRevisioner_p.cc	
	    			  src/ZsyncBlockRangeDownloader_p.cc 
				  src/ZsyncRemoteControlFileParser_p.cc
				  src/AppImageUpdateInformation_p.cc  
				  src/ZsyncBlockRangeReply_p.cc 
				  src/ZsyncWriter_p.cc
				  src/AppImageUpdaterDialog.cc
				  include/AppImageDeltaRevisioner.hpp
				  include/AppImageUpdaterBridge.hpp
				  include/AppImageUpdaterDialog.hpp
				  include/AppImageDeltaRevisioner_p.hpp
				  include/ZsyncBlockRangeReply_p.hpp
				  include/ZsyncRemoteControlFileParser_p.hpp
				  include/AppImageUpdateInformation_p.hpp
				  include/ZsyncBlockRangeDownloader_p.hpp
				  include/ZsyncInternalStructures_p.hpp 
				  include/ZsyncWriter_p.hpp)

target_compile_definitions(AppImageUpdaterBridge PUBLIC LOGGING_DISABLED=LG)
target_link_libraries(AppImageUpdaterBridge PUBLIC Qt5::Core Qt5::Network Qt5::Widgets)
target_include_directories(AppImageUpdaterBridge PUBLIC . include)
