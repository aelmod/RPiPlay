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
#include "h264-bitstream/h264_stream.h"
#include "pointer_writer.h"

struct video_renderer_s {
  logger_t *logger;
};

int *modified_data_size;
uint8_t *modified_data;

video_renderer_t *video_renderer_init(logger_t *logger, background_mode_t background_mode, bool low_latency)
{
  video_renderer_t *renderer;
  renderer = calloc(1, sizeof(video_renderer_t));
  if (!renderer) {
    return NULL;
  }
  renderer->logger = logger;

  modified_data_size = malloc(sizeof(int));
  modified_data = malloc(31 * 2);

  *modified_data_size = -1;

  write_pointer(modified_data);
  write_pointer(modified_data_size);

  return renderer;
}

void video_renderer_start(video_renderer_t *renderer)
{
}

void video_renderer_render_buffer(video_renderer_t *renderer, raop_ntp_t *ntp, unsigned char *data, int data_len,
                                  uint64_t pts, int type)
{
  if (type == 0) {
    h264_stream_t *h = h264_new();

    int sps_start, sps_end;

    int sps_size = find_nal_unit(data, data_len, &sps_start, &sps_end);
    int pps_size = data_len - 8 - sps_size;
    if (sps_size > 0) {
      read_nal_unit(h, &data[sps_start], sps_size);
      h->sps->vui.bitstream_restriction_flag = 1;
      h->sps->vui.max_dec_frame_buffering = 4; // It seems this is the lowest value that works for iOS and macOS

      // Write the modified SPS NAL
      int new_sps_size = write_nal_unit(h, modified_data + 3, data_len * 2) - 1;
      modified_data[0] = 0;
      modified_data[1] = 0;
      modified_data[2] = 0;
      modified_data[3] = 1;

      // Copy the original PPS NAL
      memcpy(modified_data + new_sps_size + 4, data + 4 + sps_size, pps_size + 4);

      data_len = new_sps_size + pps_size + 8;

      *modified_data_size = data_len;
    }
  }
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