cmake_minimum_required(VERSION 3.4.1)

#if(UNIX AND NOT APPLE)
#  include_directories( /opt/vc/include/
#  	/opt/vc/include/interface/vcos/pthreads
#  	/opt/vc/include/interface/vmcs_host/linux
#  	/opt/vc/src/hello_pi/libs/ilclient )
#
#  add_subdirectory( fdk-aac )
#  add_subdirectory( h264-bitstream )
#
#  include_directories( fdk-aac/libAACdec/include
#      fdk-aac/libAACenc/include
#      fdk-aac/libFDK/include
#      fdk-aac/libMpegTPDec/include
#      fdk-aac/libMpegTPEnc/include
#      fdk-aac/libPCMutils/include
#      fdk-aac/libSBRdec/include
#      fdk-aac/libSBRenc/include
#      fdk-aac/libSYS/include )
#
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT   -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX   -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi")
#
#  aux_source_directory(/opt/vc/src/hello_pi/libs/ilclient/ ilclient_src)
#  set(DIR_SRCS ${ilclient_src})
#  add_library( ilclient
#          STATIC
#          ${DIR_SRCS})
#
#  find_library( BRCM_GLES_V2 brcmGLESv2 HINTS /opt/vc/lib/ )
#  find_library( BRCM_EGL brcmEGL HINTS /opt/vc/lib/ )
#  find_library( OPENMAXIL openmaxil HINTS /opt/vc/lib/ )
#  find_library( BCM_HOST bcm_host HINTS /opt/vc/lib/ )
#  find_library( VCOS vcos HINTS /opt/vc/lib/ )
#  find_library( VCHIQ_ARM vchiq_arm HINTS /opt/vc/lib/ )
#  target_link_libraries( ilclient ${BRCM_GLES_V2} ${BRCM_EGL} ${OPENMAXIL}
#                         ${BCM_HOST} ${VCOS} ${VCHIQ_ARM} pthread rt m )
#
#  add_library( renderers
#          STATIC
#          audio_renderer_rpi.c video_renderer_rpi.c)
#  target_link_libraries ( renderers ilclient airplay fdk-aac h264-bitstream )
#else()


#include_directories("C:/ffmpeg-4.2.1-win32-dev/include")
#link_directories("C:/ffmpeg-4.2.1-win32-dev/lib")
#
#add_executable(decode_video decode_video.c)
#target_link_libraries(decode_video avcodec avutil)
#
#add_executable(encode_video encode_video.c)
#target_link_libraries(encode_video avcodec avutil)
#
#add_executable(stream stream.cpp)
#target_link_libraries(stream avcodec avutil avformat swscale)
#
#include_directories("C:/SDL2-2.0.10/i686-w64-mingw32/include")
#link_directories("C:/SDL2-2.0.10/i686-w64-mingw32/lib")
#add_executable(player player.cpp)
#target_link_libraries(player avcodec avutil avformat swscale mingw32 SDL2main SDL2)
#
#add_executable(player2 player2.cpp)
#target_link_libraries(player2 avcodec avutil avformat swscale mingw32 SDL2main SDL2)

add_subdirectory(h264-bitstream)
add_library(renderers
        STATIC
        audio_renderer_dummy.c video_renderer_dummy.c #[[decode_video.c]])
target_link_libraries(renderers h264-bitstream)

#endif()