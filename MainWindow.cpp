#include <cstdio>
#include "AboutDialog.h"
#include "Globals.h"
#include "MainWindow.h"
#include "res/Resource.h"
#include "rpiplay.h"
#include "lib/stream.h"

/* Main window class and title */
static LPCTSTR MainWndClass = TEXT("Win32 Example Application");

//int width = 828;
//int height = 1792;
int width = 200;
int height = 200;

HBITMAP m_hBitmap = nullptr;
BITMAP m_bitmap = {0};
UINT *m_bitmapPixels = nullptr;

void draw(HDC hdc, unsigned char *data, int data_len)
{
  if (data_len == 0) return;

//  printf("data_len %d", data_len);

  const HDC dc = CreateCompatibleDC(hdc);

  if (!dc) {
    printf("CreateCompatibleDC %lu\n", GetLastError());
    return;
  }

//  printf("data_len %d\n", data_len);
//  copyRfbPixelsToBitmap(width, height, 8, data);
  memcpy(m_bitmapPixels, data, data_len);

  if (!SelectObject(dc, m_hBitmap)) {
    printf("SelectObject %lu\n", GetLastError());
  }

  if (!BitBlt(hdc, 0, 0, width, height, dc, 0, 0, SRCCOPY)) {
    //	// If the desktop we're rendering to is inactive (like when the screen
    //	// is locked or the UAC is active), then GDI calls will randomly fail.
    //	// This is completely undocumented so we have no idea how best to deal
    //	// with it. For now, we've only seen this error and for this function
    //	// so only ignore this combination.
    if (GetLastError() != ERROR_INVALID_HANDLE) {
      printf("BitBlt %lu\n", GetLastError());
    }
  }

  DeleteDC(dc);
}

void alloc(const int width, const int height, const int bitsPerPixel)
{
  if (!width || !height || !bitsPerPixel)
    return;

  BITMAPINFO bmpInfo = {0};
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

h264_decode_struct *data;
/* Window procedure for our main window */
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    case 0x0400 + 0x0001: {
      if (!data)
        data = new h264_decode_struct;
//      data = reinterpret_cast<h264_decode_struct*>(wParam);
      auto tmp = reinterpret_cast<h264_decode_struct *>(wParam);

      if (tmp && tmp->data_len > 0) {
//        printf("tmp->data_len * 2 %d", tmp->data_len * 2);

        data->data = (unsigned char *) malloc(tmp->data_len * 2);
        data->data_len = tmp->data_len;

        memcpy(data->data, tmp->data, tmp->data_len * 2);
      }

      InvalidateRect(hWnd, 0, FALSE);
      break;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      const HDC hdc = BeginPaint(hWnd, &ps);

      if (!m_hBitmap) {
        alloc(828, 1792, 32);
      }

      if (data) draw(hdc, data->data, data->data_len);

      EndPaint(hWnd, &ps);

      if (data) free(data->data);
//      printf("paint\n");
//      surface->update(hWnd, updMessage);
      break;
    }
    case WM_COMMAND: {
      WORD id = LOWORD(wParam);

      switch (id) {
        case ID_HELP_ABOUT: {
          ShowAboutDialog(hWnd);
          return 0;
        }

        case ID_FILE_EXIT: {
          DestroyWindow(hWnd);
          return 0;
        }
      }
      break;
    }

    case WM_GETMINMAXINFO: {
      /* Prevent our window from being sized too small */
      MINMAXINFO *minMax = (MINMAXINFO *) lParam;
      minMax->ptMinTrackSize.x = 220;
      minMax->ptMinTrackSize.y = 110;

      return 0;
    }

      /* Item from system menu has been invoked */
    case WM_SYSCOMMAND: {
      WORD id = LOWORD(wParam);

      switch (id) {
        /* Show "about" dialog on about system menu item */
        case ID_HELP_ABOUT: {
          ShowAboutDialog(hWnd);
          return 0;
        }
      }
      break;
    }

    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

/* Register a class for our main window */
BOOL RegisterMainWindowClass()
{
  WNDCLASSEX wc;

  /* Class for our main window */
  wc.cbSize = sizeof(wc);
  wc.style = 0;
  wc.lpfnWndProc = &MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = g_hInstance;
  wc.hIcon = (HICON) LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE |
                                                                                            LR_DEFAULTCOLOR |
                                                                                            LR_SHARED);
  wc.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
  wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
  wc.lpszClassName = MainWndClass;
  wc.hIconSm = (HICON) LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

  return (RegisterClassEx(&wc)) ? TRUE : FALSE;
}

/* Create an instance of our main window */
HWND CreateMainWindow()
{
  /* Create instance of main window */
  HWND hWnd = CreateWindowEx(0, MainWndClass, MainWndClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 828,
                             1792,
                             NULL, NULL, g_hInstance, NULL);

  SetMenu(hWnd, NULL);
  ShowWindow(hWnd, 1);
  UpdateWindow(hWnd);

  if (hWnd) {
    /* Add "about" to the system menu */
    HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
    InsertMenu(hSysMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenu(hSysMenu, 6, MF_BYPOSITION, ID_HELP_ABOUT, TEXT("About"));

    run(NULL, nullptr, hWnd);
  }

  return hWnd;
}

