cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(entgraph
		DESCRIPTION "A node editor for Source engine map entity I/O."
		VERSION 0.1.0
		HOMEPAGE_URL "https://github.com/craftablescience/entgraph")
set(PROJECT_NAME_PRETTY "EntGraph" CACHE STRING "" FORCE)
set(PROJECT_ORGANIZATION_NAME "craftablescience" CACHE STRING "" FORCE)
set(PROJECT_HOMEPAGE_URL_API "https://api.github.com/repos/craftablescience/entgraph" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)
include(GNUInstallDirs)

# Options
option(ENTGRAPH_BUILD_INSTALLER "Build installer for ${PROJECT_NAME_PRETTY} application" ON)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)

# Qt
if(WIN32 AND NOT DEFINED QT_BASEDIR)
	message(FATAL_ERROR "Please define your QT install dir with -DQT_BASEDIR=\"C:/your/qt6/here\"")
endif()

if(DEFINED QT_BASEDIR)
	string(REPLACE "\\" "/" QT_BASEDIR "${QT_BASEDIR}")

	# Add it to the prefix path so find_package can find it
	list(APPEND CMAKE_PREFIX_PATH "${QT_BASEDIR}")
	set(QT_INCLUDE "${QT_BASEDIR}/include")
	message(STATUS "Using ${QT_INCLUDE} as the Qt include directory")
endif()

if(WIN32)
	if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(QT_LIB_SUFFIX "d" CACHE STRING "" FORCE)
	else()
		set(QT_LIB_SUFFIX "" CACHE STRING "" FORCE)
	endif()
endif()

# CMake has an odd policy that links a special link lib for Qt on newer versions of CMake
cmake_policy(SET CMP0020 NEW)

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

# Set up automatic Qt stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# QtNodes
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/nodeeditor")
set_target_properties(
        QtNodes
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# SpeedyKeyV
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/speedykeyv")

# vmfpp
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/vmfpp")

# Config file
configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/src/cfg/Config.h.in"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/cfg/Config.h")
configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/installer/deb/${PROJECT_NAME}.desktop.in"
		"${CMAKE_CURRENT_SOURCE_DIR}/installer/deb/${PROJECT_NAME}.desktop")
configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/installer/deb/${PROJECT_NAME}.sh.in"
		"${CMAKE_CURRENT_SOURCE_DIR}/installer/deb/${PROJECT_NAME}.sh")

# Add sources and create executable
set(${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/res/res.qrc"

        "${CMAKE_CURRENT_SOURCE_DIR}/src/cfg/Config.h"

        "${CMAKE_CURRENT_SOURCE_DIR}/src/graph/EntityGraph.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/graph/EntityGraph.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/graph/EntityGraphModel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/graph/EntityGraphModel.h"

        "${CMAKE_CURRENT_SOURCE_DIR}/src/wrapper/VMFWrapper.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/wrapper/VMFWrapper.h"

		"${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Options.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Options.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Window.h")
if(WIN32)
	list(APPEND ${PROJECT_NAME}_SOURCES
			"${CMAKE_CURRENT_SOURCE_DIR}/res/icon.rc")
endif()

add_executable(
		${PROJECT_NAME} WIN32
		${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Gui Qt::Widgets Qt::OpenGL QtNodes keyvalues vmfpp)
target_include_directories(
		${PROJECT_NAME} PRIVATE
		"${QT_INCLUDE}"
		"${QT_INCLUDE}/QtCore"
		"${QT_INCLUDE}/QtGui"
		"${QT_INCLUDE}/QtWidgets"
		"${QT_INCLUDE}/QtOpenGL"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/speedykeyv")
if(WIN32 AND MSVC)
	target_link_options(
			${PROJECT_NAME} PRIVATE
			"/ENTRY:mainCRTStartup")
endif()

# Set up install rules
install(IMPORTED_RUNTIME_ARTIFACTS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL QtNodes
		RUNTIME DESTINATION .
		LIBRARY DESTINATION .)
install(FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/CREDITS.md"
		"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
		"${CMAKE_CURRENT_SOURCE_DIR}/installer/.nonportable"
		DESTINATION .)
if(WIN32)
	install(FILES
			"${QT_BASEDIR}/plugins/imageformats/qjpeg${QT_LIB_SUFFIX}.dll"
			"${QT_BASEDIR}/plugins/imageformats/qtga${QT_LIB_SUFFIX}.dll"
			DESTINATION imageformats)
	install(FILES "${QT_BASEDIR}/plugins/platforms/qwindows${QT_LIB_SUFFIX}.dll"
			DESTINATION platforms)
	install(FILES "${QT_BASEDIR}/plugins/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll"
			DESTINATION styles)
elseif(UNIX)
	install(DIRECTORY
			"${QT_BASEDIR}/lib/"
			"${QT_BASEDIR}/plugins/"
			DESTINATION .
			FILES_MATCHING PATTERN "*.so*")

	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/deb/${PROJECT_NAME}.desktop"
			DESTINATION "/usr/share/applications/")
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png"
			DESTINATION "/usr/share/pixmaps/")

	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/installer/deb/${PROJECT_NAME}.sh"
			DESTINATION .
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

# Copy these next to the executable
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CREDITS.md" "${CMAKE_BINARY_DIR}/CREDITS.md" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"    "${CMAKE_BINARY_DIR}/LICENSE"    COPYONLY)
# Don't copy the .nonportable file, we're debugging in standalone mode

# Copy these so the user doesn't have to
if(WIN32)
	configure_file("${QT_BASEDIR}/bin/Qt6Core${QT_LIB_SUFFIX}.dll"    "${CMAKE_BINARY_DIR}/Qt6Core${QT_LIB_SUFFIX}.dll"    COPYONLY)
	configure_file("${QT_BASEDIR}/bin/Qt6Gui${QT_LIB_SUFFIX}.dll"     "${CMAKE_BINARY_DIR}/Qt6Gui${QT_LIB_SUFFIX}.dll"     COPYONLY)
	configure_file("${QT_BASEDIR}/bin/Qt6Widgets${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/Qt6Widgets${QT_LIB_SUFFIX}.dll" COPYONLY)
	configure_file("${QT_BASEDIR}/bin/Qt6OpenGL${QT_LIB_SUFFIX}.dll"  "${CMAKE_BINARY_DIR}/Qt6OpenGL${QT_LIB_SUFFIX}.dll"  COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/imageformats/qjpeg${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/imageformats/qjpeg${QT_LIB_SUFFIX}.dll" COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/imageformats/qtga${QT_LIB_SUFFIX}.dll"  "${CMAKE_BINARY_DIR}/imageformats/qtga${QT_LIB_SUFFIX}.dll"  COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/platforms/qwindows${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/platforms/qwindows${QT_LIB_SUFFIX}.dll" COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll" COPYONLY)
elseif(UNIX)
	configure_file("${QT_BASEDIR}/lib/libQt6Core.so.6"    "${CMAKE_BINARY_DIR}/libQt6Core.so.6"    COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6Gui.so.6"     "${CMAKE_BINARY_DIR}/libQt6Gui.so.6"     COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6Widgets.so.6" "${CMAKE_BINARY_DIR}/libQt6Widgets.so.6" COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6OpenGL.so.6"  "${CMAKE_BINARY_DIR}/libQt6OpenGL.so.6"  COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6DBus.so.6"    "${CMAKE_BINARY_DIR}/libQt6DBus.so.6"    COPYONLY)

	configure_file("${QT_BASEDIR}/lib/libQt6WaylandClient.so.6" "${CMAKE_BINARY_DIR}/libQt6WaylandClient.so.6" COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libQt6XcbQpa.so.6"        "${CMAKE_BINARY_DIR}/libQt6XcbQpa.so.6"        COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libicui18n.so.56"         "${CMAKE_BINARY_DIR}/libicui18n.so.56"         COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libicuuc.so.56"           "${CMAKE_BINARY_DIR}/libicuuc.so.56"           COPYONLY)
	configure_file("${QT_BASEDIR}/lib/libicudata.so.56"         "${CMAKE_BINARY_DIR}/libicudata.so.56"         COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/imageformats/libqjpeg.so" "${CMAKE_BINARY_DIR}/imageformats/libqjpeg.so" COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/imageformats/libqtga.so"  "${CMAKE_BINARY_DIR}/imageformats/libqtga.so"  COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/platforms/libqwayland-generic.so" "${CMAKE_BINARY_DIR}/platforms/libqwayland-generic.so" COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/platforms/libqminimal.so"         "${CMAKE_BINARY_DIR}/platforms/libqminimal.so"         COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/platforms/libqxcb.so"             "${CMAKE_BINARY_DIR}/platforms/libqxcb.so"             COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/platformthemes/libqgtk3.so" "${CMAKE_BINARY_DIR}/platformthemes/libqgtk3.so" COPYONLY)

	configure_file("${QT_BASEDIR}/plugins/wayland-shell-integration/libxdg-shell.so" "${CMAKE_BINARY_DIR}/wayland-shell-integration/libxdg-shell.so" COPYONLY)
	configure_file("${QT_BASEDIR}/plugins/wayland-shell-integration/libqt-shell.so"  "${CMAKE_BINARY_DIR}/wayland-shell-integration/libqt-shell.so"  COPYONLY)
endif()

if(ENTGRAPH_BUILD_INSTALLER)
	set(CPACK_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
	set(CPACK_PACKAGE_VENDOR ${PROJECT_ORGANIZATION_NAME})
	set(CPACK_PACKAGE_CONTACT "lauralewisdev@gmail.com")
	set(CPACK_PACKAGE_DESCRIPTION ${CMAKE_PROJECT_DESCRIPTION})
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})
	set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
	set(CPACK_MONOLITHIC_INSTALL ON)
	set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME_PRETTY})
	if(WIN32)
		set(CPACK_GENERATOR "NSIS")
		set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
		set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
		set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME_PRETTY})
		set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
		set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.ico")
		set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_NAME}.exe")
		set(CPACK_NSIS_URL_INFO_ABOUT "${CMAKE_PROJECT_HOMEPAGE_URL}")
		set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
	else()
		set(CPACK_GENERATOR "DEB")
		set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
		set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/${PROJECT_NAME}")
	endif()
	include(CPack)
endif()
