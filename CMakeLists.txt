CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(QAppImageUpdate VERSION 2.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

option(LG "LOGGING_DISABLED" OFF)
option(NG "NO_GUI" OFF)
option(BAP "BUILD_AS_PLUGIN" OFF)

# Let cmake know that this is a release build.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(MIN_QT_VERSION "5.6.0")

find_package(Qt5Core ${MIN_QT_VERSION})
find_package(Qt5Network ${MIN_QT_VERSION})

if(NOT NO_GUI)
    find_package(Qt5Widgets ${MIN_QT_VERSION})
endif()

# cmake macros used
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Include Directories.
include_directories(.)
include_directories(include)
include_directories(${CMAKE_BINARY_DIR})

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
	add_subdirectory(examples/CheckForUpdate)
	add_subdirectory(examples/GetEmbeddedInfo)
	add_subdirectory(examples/ProxyUpdate)
	add_subdirectory(examples/Update)
	add_subdirectory(examples/UpdateWithTorrent)
	add_subdirectory(examples/UpdateWithGUI)
	add_subdirectory(examples/UpdateWithGUIAndTorrent)
endif()

SET(source)
list(APPEND source
    src/qappimageupdate.cc
    src/qappimageupdate_p.cc
    src/rangereply.cc
    src/rangereply_p.cc
    src/rangedownloader.cc
    src/rangedownloader_p.cc
    src/zsyncremotecontrolfileparser_p.cc
    src/appimageupdateinformation_p.cc
    src/zsyncwriter_p.cc
    src/helpers_p.cc
    include/qappimageupdate.hpp
    include/qappimageupdate_p.hpp
    include/rangereply.hpp
    include/rangereply_p.hpp
    include/zsyncremotecontrolfileparser_p.hpp
    include/appimageupdateinformation_p.hpp
    include/rangedownloader.hpp
    include/rangedownloader_p.hpp
    include/zsyncinternalstructures_p.hpp
    include/zsyncwriter_p.hpp
    include/qappimageupdatecodes.hpp
    include/qappimageupdateenums.hpp
    include/helpers_p.hpp)

SET(toinstall)
list(APPEND toinstall
    QAppImageUpdate
    include/qappimageupdate.hpp
    include/qappimageupdateenums.hpp
    include/qappimageupdatecodes.hpp
)	

if(LOGGING_DISABLED)
    message("-- [*] IMPORTANT: Logging will be disabled for this build.")
    add_definitions(-DLOGGING_DISABLED)
endif()

if(NO_GUI)
    message("-- [*] IMPORTANT: No gui classes will be included in this build.")
    add_definitions(-DNO_GUI)
else()
    list(APPEND source src/softwareupdatedialog_p.cc
		       include/softwareupdatedialog_p.hpp)
endif()


if(DECENTRALIZED_UPDATE_ENABLED)
    message("-- [*] IMPORTANT: Decentralized update feature is ENABLED.")
    add_definitions(-DDECENTRALIZED_UPDATE_ENABLED)
    list(APPEND source src/torrentdownloader.cc
	    	       src/torrentdownloader_p.cc
		       src/seeder.cc
		       src/seeder_p.cc
		       include/torrentdownloader.hpp
		       include/torrentdownloader_p.hpp
		       include/seeder.hpp
		       include/seeder_p.hpp)
endif()

if(NOT DECENTRALIZED_UPDATE_ENABLED)
    message("-- [*] IMPORTANT: Decentralized update feature is DISABLED in this build.")
endif()

if(BUILD_AS_PLUGIN)
   message("-- [*] IMPORTANT: Building as a Qt Plugin.")
   add_definitions(-DBUILD_AS_PLUGIN)
   list(APPEND source include/qappimageupdateinterface.hpp
	              include/qappimageupdateinterfaceimpl.hpp
		      src/qappimageupdateinterfaceimpl.cc)
   add_library(QAppImageUpdate SHARED ${source})
else()
   if(BUILD_AS_SHARED_LIB)
   	add_library(QAppImageUpdate SHARED ${source})
   endif()

   if(NOT BUILD_AS_SHARED_LIB)
	add_library(QAppImageUpdate ${source})
   endif()
endif()

target_link_libraries(QAppImageUpdate PUBLIC Qt5::Core Qt5::Network)
target_include_directories(QAppImageUpdate INTERFACE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include>")
target_include_directories(QAppImageUpdate INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QAppImageUpdate>" )

if(DECENTRALIZED_UPDATE_ENABLED)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/share/cmake/Modules")
	find_package(LibtorrentRasterbar)	
	target_link_libraries(QAppImageUpdate PUBLIC LibtorrentRasterbar::torrent-rasterbar)
endif()

if(NOT NO_GUI)
	target_link_libraries(QAppImageUpdate PUBLIC Qt5::Widgets)
endif()


# Add pkg-config and install instructions
configure_file(
	"${PROJECT_SOURCE_DIR}/other/pkgconfig/QAppImageUpdate.pc.in"
	"${PROJECT_BINARY_DIR}/QAppImageUpdate.pc"
  @ONLY
)

install(FILES
	${PROJECT_BINARY_DIR}/QAppImageUpdate.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(TARGETS
	QAppImageUpdate
	EXPORT QAppImageUpdateTargets
  DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES
  ${toinstall}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QAppImageUpdate")

# Add CMake config
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/QAppImageUpdate")
if(NOT NO_GUI)
    set(PACKAGE_FIND_DEPENDENCY_QTWIDGETS "find_dependency(Qt5Widgets ${MIN_QT_VERSION})")
endif()
configure_package_config_file(
	"${PROJECT_SOURCE_DIR}/other/cmake/QAppImageUpdateConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/QAppImageUpdateConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/QAppImageUpdateConfig.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}")

install(EXPORT QAppImageUpdateTargets
	FILE QAppImageUpdateTargets.cmake
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}")
