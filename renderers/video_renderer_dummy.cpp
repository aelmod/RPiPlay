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

#include "video_renderer.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "h264_data.h"
#include "h264-bitstream/h264_stream.h"
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/move/unique_ptr.hpp>
#include <iostream>

using namespace boost::interprocess;

struct video_renderer_s {
  logger_t *logger;
};

static const char *const FIRST_FRAME_SHARED_MEMORY = "FirstFrameSharedMemory";

message_queue *frames_queue = nullptr;
managed_shared_memory *segment = nullptr;

using FirstFrame = std::pair<int *, int>;


video_renderer_t *video_renderer_init(logger_t *logger, background_mode_t background_mode, bool low_latency)
{
  message_queue::remove("frames_queue");

  frames_queue = new message_queue(
      open_or_create,
      "frames_queue",
      100,
      MAX_SIZE
  );

  shared_memory_object::remove(FIRST_FRAME_SHARED_MEMORY);
  segment = new managed_shared_memory(create_only, FIRST_FRAME_SHARED_MEMORY, 65536);

  auto t1 = new int();
//  size_t nbytes = sizeof(FirstFrame);
  int *ptr = (int *) segment->allocate(sizeof(int *));

  *ptr = 10;


  shared_memory_object::remove("FOO");
  boost::interprocess::shared_memory_object shm
      (boost::interprocess::create_only, "FOO"              //name
          , boost::interprocess::read_write
      );
  shm.truncate(sizeof(int *) + sizeof(int));

  //Map the whole shared memory in this process
  boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);


//  auto *firstFrame = new FirstFrame(new int(1488), 1337);

  int *ttt = new int(4554645);
  int *ttt2 = new int(1111111);

  auto *addr = region.get_address();
  memcpy(addr, ttt, sizeof(int *));
  memcpy((int *) addr + sizeof(int), ttt2, sizeof(int *));

//  addr = ttt;
//  *addr = *firstFrame;

//  boost::interprocess::mapped_region region2(shm, boost::interprocess::read_write);
//  t1 = (int *) region2.get_address();
//
//  std::cout << *t1 << std::endl;
//  int kek = 10;
//  auto pair = FirstFrame();
//  pair.first = &kek;
//  pair.second = 100;

//  memcpy(ptr, &pair, 4);
//  int *kek = new int(10);
//  structure s;
//  s.integer1 = 10;
//  s.integer2 = 20;
//
//  s.ptr = &s.integer1;
//  s.ptr = &s.integer2;

//  segment->construct<int>
//      ("FirstFrame instance")
//      (777);

  video_renderer_t *renderer;

  renderer = (video_renderer_t *) calloc(1, sizeof(video_renderer_t));
  if (!renderer) {
    return NULL;
  }
  renderer->logger = logger;
  return renderer;
}

void video_renderer_start(video_renderer_t *renderer)
{
}

void video_renderer_render_buffer(video_renderer_t *renderer, raop_ntp_t *ntp, unsigned char *data, int data_len,
                                  uint64_t pts, int type)
{
//  if (type == 0) {
//    segment->construct<FirstFrame>
//        ("FirstFrame instance")
//        (os.str(), data_len);
//    return;
//  }

  std::ostringstream os;
  os << data;

  h264_data frame(os.str(), data_len, pts, type);

  std::stringstream oss;

  boost::archive::text_oarchive oa(oss);
  oa << frame;

  std::string serialized_string(oss.str());

  frames_queue->send(serialized_string.data(), serialized_string.size(), 0);
}

void video_renderer_flush(video_renderer_t *renderer)
{
}

void video_renderer_destroy(video_renderer_t *renderer)
{
  if (renderer) {
    free(renderer);
  }
}

void video_renderer_update_background(video_renderer_t *renderer, int type)
{

}