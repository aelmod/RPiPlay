cmake_minimum_required(VERSION 3.4.1)
include_directories(curve25519 ed25519 playfair plist/plist)

aux_source_directory(. play_src)
set(DIR_SRCS ${play_src})

add_library(airplay
        STATIC
        ${DIR_SRCS}
        )

target_link_libraries(airplay
        ws2_32
        pthread
        curve25519
        ed25519
        playfair
        plist)

if (UNIX AND NOT APPLE)
  find_package(OpenSSL REQUIRED)
  target_link_libraries(airplay OpenSSL::Crypto)
  target_link_libraries(airplay dns_sd)
elseif (APPLE)
  include_directories(/usr/local/opt/openssl@1.1/include/)
  target_link_libraries(airplay /usr/local/opt/openssl@1.1/lib/libcrypto.a)
elseif (WIN32)
  include_directories("D:/Development/Lib/openssl-1.1.1d-win32-mingw/include")
  target_link_libraries(airplay "D:/Development/Lib/openssl-1.1.1d-win32-mingw/lib/libcrypto.dll.a")

  target_link_libraries(airplay "C:/Program Files/Bonjour SDK/Lib/Win32/dnssd.lib")

  include_directories("D:/Development/Lib/ffmpeg-4.2.1-win32-dev/include")
  target_link_directories(airplay PUBLIC "D:/Development/Lib/ffmpeg-4.2.1-win32-dev/lib")

  target_link_libraries(airplay avcodec avutil avformat swscale)
endif ()
