/**
 * RPiPlay - An open-source AirPlay mirroring server for Raspberry Pi
 * Copyright (C) 2019 Florian Draschbacher
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <stddef.h>
#include <cstring>
#include <signal.h>
#include <unistd.h>
//#include <string>
#include <vector>
#include <fstream>

#include "log.h"
#include "lib/raop.h"
#include "lib/stream.h"
#include "lib/logger.h"
#include "lib/dnssd.h"
#include "renderers/video_renderer.h"
#include "renderers/audio_renderer.h"

#define VERSION "1.2"

#define DEFAULT_NAME "RPiPlay"
#define DEFAULT_BACKGROUND_MODE BACKGROUND_MODE_ON
#define DEFAULT_AUDIO_DEVICE AUDIO_DEVICE_HDMI
#define DEFAULT_LOW_LATENCY false
#define DEFAULT_DEBUG_LOG false
#define DEFAULT_HW_ADDRESS { (char) 0x48, (char) 0x5d, (char) 0x60, (char) 0x7c, (char) 0xee, (char) 0x22 }

int start_server(std::vector<char> hw_addr, std::string name, background_mode_t background_mode,
                 audio_device_t audio_device, bool low_latency, bool debug_log);

int stop_server();

static bool running = false;
static dnssd_t *dnssd = NULL;
static raop_t *raop = NULL;
static video_renderer_t *video_renderer = NULL;
static audio_renderer_t *audio_renderer = NULL;

static void signal_handler(int sig)
{
  switch (sig) {
    case SIGINT:
    case SIGTERM:
      running = false;
      break;
  }
}

static void init_signals()
{
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
}

static int parse_hw_addr(std::string str, std::vector<char> &hw_addr)
{
  for (int i = 0; i < str.length(); i += 3) {
    hw_addr.push_back((char) stol(str.substr(i), NULL, 16));
  }
  return 0;
}

std::string find_mac()
{
  std::ifstream iface_stream("/sys/class/net/eth0/address");
  if (!iface_stream) {
    iface_stream.open("/sys/class/net/wlan0/address");
  }
  if (!iface_stream) return "";

  std::string mac_address;
  iface_stream >> mac_address;
  iface_stream.close();
  return mac_address;
}

void print_info(char *name)
{
  printf("RPiPlay %s: An open-source AirPlay mirroring server for Raspberry Pi\n", VERSION);
  printf("Usage: %s [-b (on|auto|off)] [-n name] [-a (hdmi|analog|off)]\n", name);
  printf("Options:\n");
  printf("-n name               Specify the network name of the AirPlay server\n");
  printf("-b (on|auto|off)      Show black background always, only during active connection, or never\n");
  printf("-a (hdmi|analog|off)  Set audio output device\n");
  printf("-l                    Enable low-latency mode (disables render clock)\n");
  printf("-d                    Enable debug logging\n");
  printf("-v/-h                 Displays this help and version information\n");
}

#include <windows.h>
#include <iostream>

HWND GetConsoleHwnd()
{
#define MY_BUFSIZE 1024 // Buffer size for console window titles.
  HWND hwndFound;         // This is what is returned to the caller.
  char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
  // WindowTitle.
  char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
  // WindowTitle.

  // Fetch current window title.

  GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

  // Format a "unique" NewWindowTitle.

  wsprintf(pszNewWindowTitle, "%d/%d",
           GetTickCount(),
           GetCurrentProcessId());

//  wsprintf(pszNewWindowTitle, "rpiplay");

  printf("pszOldWindowTitle: %s\n", pszOldWindowTitle);
  printf("pszNewWindowTitle: %s\n", pszNewWindowTitle);
  // Change current window title.

  SetConsoleTitle(pszNewWindowTitle);

  // Ensure window title has been updated.

  Sleep(40);

  // Look for NewWindowTitle.

  hwndFound = FindWindow(NULL, pszNewWindowTitle);

  // Restore original window title.

//  SetConsoleTitle(pszOldWindowTitle);

  return (hwndFound);
}

static unsigned char *sharedData;
static int *sharedDataLen;
static int *frameType;
static uint64_t *sharedPTS;

int main(int argc, char *argv[])
{
//  GetConsoleHwnd();

  sharedData = (unsigned char *) malloc(200000);
  sharedDataLen = new int(-1);
  frameType = new int(-1);
  sharedPTS = new uint64_t();

  std::cout << "sharedDataLen ptr: " << static_cast<void*>(sharedDataLen) << std::endl;
  std::cout << "sharedData ptr: " << static_cast<void*>(sharedData) << std::endl;
  std::cout << "frameType ptr: " << static_cast<void*>(frameType) << std::endl;

  std::ofstream ptrFile;
  ptrFile.open ("pointers.txt");
  ptrFile << static_cast<void*>(sharedDataLen) << "\n";
  ptrFile << static_cast<void*>(sharedData) << "\n";
  ptrFile << static_cast<void *>(frameType) << "\n";
  ptrFile << static_cast<void *>(sharedPTS) << "\n";
  ptrFile.close();

  init_signals();

  background_mode_t background = DEFAULT_BACKGROUND_MODE;
  std::string server_name = DEFAULT_NAME;
  std::vector<char> server_hw_addr = DEFAULT_HW_ADDRESS;
  audio_device_t audio_device = DEFAULT_AUDIO_DEVICE;
  bool low_latency = DEFAULT_LOW_LATENCY;
  bool debug_log = DEFAULT_DEBUG_LOG;

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (arg == "-n") {
      if (i == argc - 1) continue;
      server_name = std::string(argv[++i]);
    } else if (arg == "-b") {
      // For backwards-compatibility, make just -b disable the background
      if (i == argc - 1 || argv[i + 1][0] == '-') {
        background = BACKGROUND_MODE_OFF;
        continue;
      }

      std::string background_mode(argv[++i]);
      background = background_mode == "off" ? BACKGROUND_MODE_OFF :
                   background_mode == "auto" ? BACKGROUND_MODE_AUTO :
                   BACKGROUND_MODE_ON;
    } else if (arg == "-a") {
      if (i == argc - 1) continue;
      std::string audio_device_name(argv[++i]);
      audio_device = audio_device_name == "hdmi" ? AUDIO_DEVICE_HDMI :
                     audio_device_name == "analog" ? AUDIO_DEVICE_ANALOG :
                     AUDIO_DEVICE_NONE;
    } else if (arg == "-l") {
      low_latency = !low_latency;
    } else if (arg == "-d") {
      debug_log = !debug_log;
    } else if (arg == "-h" || arg == "-v") {
      print_info(argv[0]);
      exit(0);
    }
  }

  std::string mac_address = find_mac();
  if (!mac_address.empty()) {
    server_hw_addr.clear();
    parse_hw_addr(mac_address, server_hw_addr);
  }

  if (start_server(server_hw_addr, server_name, background, audio_device, low_latency, debug_log) != 0) {
    return 1;
  }

  running = true;

  while (running) {
    sleep(1);
  }

  LOGI("Stopping...");
  stop_server();
}

// Server callbacks
extern "C" void conn_init(void *cls)
{
  video_renderer_update_background(video_renderer, 1);
}

extern "C" void conn_destroy(void *cls)
{
  *sharedDataLen = -1;
  *frameType = -1;
  video_renderer_update_background(video_renderer, -1);
}

extern "C" void audio_process(void *cls, raop_ntp_t *ntp, aac_decode_struct *data)
{
  if (audio_renderer != NULL) {
    audio_renderer_render_buffer(audio_renderer, ntp, data->data, data->data_len, data->pts);
  }
}

extern "C" void video_process(void *cls, raop_ntp_t *ntp, h264_decode_struct *data)
{
  *sharedDataLen = data->data_len;
  *frameType = data->frame_type;
  *sharedPTS = data->pts;
  memcpy(sharedData, data->data, data->data_len);


  video_renderer_render_buffer(video_renderer, ntp, data->data, data->data_len, data->pts, data->frame_type);
}

extern "C" void audio_flush(void *cls)
{
  audio_renderer_flush(audio_renderer);
}

extern "C" void video_flush(void *cls)
{
  video_renderer_flush(video_renderer);
}

extern "C" void audio_set_volume(void *cls, float volume)
{
  if (audio_renderer != NULL) {
    audio_renderer_set_volume(audio_renderer, volume);
  }
}

extern "C" void log_callback(void *cls, int level, const char *msg)
{
  switch (level) {
    case LOGGER_DEBUG: {
      LOGD("%s", msg);
      break;
    }
    case LOGGER_WARNING: {
      LOGW("%s", msg);
      break;
    }
    case LOGGER_INFO: {
      LOGI("%s", msg);
      break;
    }
    case LOGGER_ERR: {
      LOGE("%s", msg);
      break;
    }
    default:
      break;
  }

}

int start_server(std::vector<char> hw_addr, std::string name, background_mode_t background_mode,
                 audio_device_t audio_device, bool low_latency, bool debug_log)
{
  raop_callbacks_t raop_cbs;
  memset(&raop_cbs, 0, sizeof(raop_cbs));
  raop_cbs.conn_init = conn_init;
  raop_cbs.conn_destroy = conn_destroy;
  raop_cbs.audio_process = audio_process;
  raop_cbs.video_process = video_process;
  raop_cbs.audio_flush = audio_flush;
  raop_cbs.video_flush = video_flush;
  raop_cbs.audio_set_volume = audio_set_volume;

  raop = raop_init(10, &raop_cbs);
  if (raop == NULL) {
    LOGE("Error initializing raop!");
    return -1;
  }

  raop_set_log_callback(raop, log_callback, NULL);
  raop_set_log_level(raop, debug_log ? RAOP_LOG_DEBUG : LOGGER_INFO);

  logger_t *render_logger = logger_init();
  logger_set_callback(render_logger, log_callback, NULL);
  logger_set_level(render_logger, debug_log ? LOGGER_DEBUG : LOGGER_INFO);

  if (low_latency) logger_log(render_logger, LOGGER_INFO, "Using low-latency mode");

  if ((video_renderer = video_renderer_init(render_logger, background_mode, low_latency)) == NULL) {
    LOGE("Could not init video renderer");
    return -1;
  }

  if (audio_device == AUDIO_DEVICE_NONE) {
    LOGI("Audio disabled");
  } else if ((audio_renderer = audio_renderer_init(render_logger, video_renderer, audio_device, low_latency)) ==
             NULL) {
    LOGE("Could not init audio renderer");
    return -1;
  }

  if (video_renderer) video_renderer_start(video_renderer);
  if (audio_renderer) audio_renderer_start(audio_renderer);

  unsigned short port = 0;
  raop_start(raop, &port);
  raop_set_port(raop, port);

  int error;
  dnssd = dnssd_init(name.c_str(), strlen(name.c_str()), hw_addr.data(), hw_addr.size(), &error);
  if (error) {
    LOGE("Could not initialize dnssd library!");
    return -2;
  }

  raop_set_dnssd(raop, dnssd);

  dnssd_register_raop(dnssd, port);
  dnssd_register_airplay(dnssd, port + 1);

  return 0;
}

int stop_server()
{
  raop_destroy(raop);
  dnssd_unregister_raop(dnssd);
  dnssd_unregister_airplay(dnssd);
  // If we don't destroy these two in the correct order, we get a deadlock from the ilclient library
  audio_renderer_destroy(audio_renderer);
  video_renderer_destroy(video_renderer);
  return 0;
}
