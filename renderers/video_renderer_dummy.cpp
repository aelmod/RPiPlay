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
#include "windows.h"

struct video_renderer_s {
    logger_t *logger;
};

video_renderer_t *video_renderer_init(logger_t *logger, background_mode_t background_mode, bool low_latency)
{
  video_renderer_t *renderer;
  renderer = static_cast<video_renderer_t *>(calloc(1, sizeof(video_renderer_t)));
  if (!renderer) {
    return NULL;
  }
  renderer->logger = logger;
  return renderer;
}

HBITMAP m_hBitmap = 0;
BITMAP m_bitmap = {0};
UINT *m_bitmapPixels = 0;

void alloc(const int width, const int height, const int bitsPerPixel)
{
  if (!width || !height || !bitsPerPixel)
    return;

  BITMAPINFO bmpInfo = { 0 };
  bmpInfo.bmiHeader.biBitCount = bitsPerPixel;
  bmpInfo.bmiHeader.biHeight = -int(height);
  bmpInfo.bmiHeader.biWidth = width;
  bmpInfo.bmiHeader.biPlanes = 1;
  bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

  m_hBitmap = CreateDIBSection(
          nullptr,
          static_cast<BITMAPINFO *>(&bmpInfo),
          DIB_RGB_COLORS,
          reinterpret_cast<void **>(&m_bitmapPixels),
          nullptr,
          0);

  if (!m_hBitmap)
    return; //TODO: catch this case

  GetObject(m_hBitmap, sizeof(m_bitmap), &m_bitmap);
}

void dealloc()
{
DeleteObject(m_hBitmap);
}

void video_renderer_start(video_renderer_t *renderer) {
}

//UINT *m_bitmapPixels = 0;
void copyRfbPixelsToBitmap(const UINT uWidth, const UINT uHeight, const UINT uBitsPerPixel, const LPVOID pBits)
{
  const LONG lBmpSize = uWidth * uHeight * (uBitsPerPixel / 8);
  memcpy(m_bitmapPixels, pBits, lBmpSize);
}

void video_renderer_render_buffer(video_renderer_t *renderer, raop_ntp_t *ntp, unsigned char *data, int data_len,
                                  uint64_t pts, int type)
{
  PAINTSTRUCT ps;
//  auto hdc = BeginPaint(pHinstance, &ps);
//
//  const HDC dc = CreateCompatibleDC(NULL);
//
//  if (!dc) {
//    wchar_t buffer[256];
//    wsprintfW(buffer, L"CreateCompatibleDC %d\n", GetLastError());
//    OutputDebugString(buffer);
//    return;
//  }
//
  copyRfbPixelsToBitmap(828, 1792, 32, data);
//
//  if (!SelectObject(dc, m_hBitmap)) {
//    wchar_t buffer[256];
//    wsprintfW(buffer, L"SelectObject %d\n", GetLastError());
//    OutputDebugString(buffer);
//  }
//
//  if (!BitBlt(hdc, 0, 0, 1792, 828, dc, 0, 0, SRCCOPY)) {
//    //	// If the desktop we're rendering to is inactive (like when the screen
//    //	// is locked or the UAC is active), then GDI calls will randomly fail.
//    //	// This is completely undocumented so we have no idea how best to deal
//    //	// with it. For now, we've only seen this error and for this function
//    //	// so only ignore this combination.
//    if (GetLastError() != ERROR_INVALID_HANDLE) {
//      wchar_t buffer[256];
//      wsprintfW(buffer, L"BitBlt %d\n", GetLastError());
//      OutputDebugString(buffer);
//    }
//  }
//
//  DeleteDC(dc);
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
