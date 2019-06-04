CMAKE_MINIMUM_REQUIRED( VERSION 3.2)
project(AppImageUpdaterBridge)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
option(LG "LOGGING_DISABLED" OFF)
option(NG "NO_GUI" OFF)

# Let cmake know that this is a release build.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(Qt5Core)
find_package(Qt5Network)
find_package(Qt5Widgets)

# Include Directories.
include_directories(.)
include_directories(include)
include_directories(${CMAKE_BINARY_DIR})

SET(source)
list(APPEND source src/appimagedeltarevisioner.cc
       				  src/appimagedeltarevisioner_p.cc	
	    			  src/zsyncblockrangedownloader_p.cc 
				  src/zsyncremotecontrolfileparser_p.cc
				  src/appimageupdateinformation_p.cc  
				  src/zsyncblockrangereply_p.cc 
				  src/zsyncwriter_p.cc
				  src/appimageupdaterdialog.cc
				  src/appimageupdaterbridge_enums.cc
				  src/helpers_p.cc
				  include/appimagedeltarevisioner.hpp
				  include/appimageupdaterbridge.hpp
				  include/appimageupdaterdialog.hpp
				  include/appimagedeltarevisioner_p.hpp
				  include/zsyncblockrangereply_p.hpp
				  include/zsyncremotecontrolfileparser_p.hpp
				  include/appimageupdateinformation_p.hpp
				  include/zsyncblockrangedownloader_p.hpp
				  include/zsyncinternalstructures_p.hpp 
				  include/zsyncwriter_p.hpp
				  include/appimageupdaterbridge_enums.hpp
				  include/helpers_p.hpp)
if(LOGGING_DISABLED)
	message("-- [*] IMPORTANT: Logging will be disabled for this build.")
endif()

if(NO_GUI)
	message("-- [*] IMPORTANT: No gui classes will be included in this build.")
else()
	list(APPEND source src/appimageupdaterdialog.cc include/appimageupdaterdialog.hpp)
endif()

add_library(AppImageUpdaterBridge ${source})
target_compile_definitions(AppImageUpdaterBridge PUBLIC LOGGING_DISABLED=LG)
target_link_libraries(AppImageUpdaterBridge PUBLIC Qt5::Core Qt5::Network Qt5::Widgets)
target_include_directories(AppImageUpdaterBridge PUBLIC . include)
