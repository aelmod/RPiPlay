cmake_minimum_required(VERSION 3.4.1)

project(rpiplay)

set (CMAKE_CXX_STANDARD 17)
#CMAKE_POLICY(SET CMP0020 NEW)
#set(_WIN32_IE 0x0500)
#set(WINVER 0x0500)

SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

add_subdirectory(lib/curve25519)
add_subdirectory(lib/ed25519)
add_subdirectory(lib/playfair)
add_subdirectory(lib/plist)
add_subdirectory(lib)
add_subdirectory(renderers)

add_executable( rpiplay
        res/Resource.h res/Resource.rc res/Application.manifest res/Application.ico
        MainWindow.h MainWindow.cpp Globals.h WinMain.cpp AboutDialog.h AboutDialog.cpp
        rpiplay.cpp rpiplay.h
        )
#set_target_properties(rpiplay PROPERTIES LINK_FLAGS "-mwindows")
target_link_libraries ( rpiplay renderers airplay comctl32 #[[-s -lcomctl32 -Wl,--subsystem,windows]])
