// BlowUp.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "BlowUp.h"
#include <stdlib.h> // for abs definition

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BLOWUP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BLOWUP));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

void InvertBlock(HWND hwndScr, HWND hwnd, POINT ptBeg, POINT ptEnd)
{
	HDC hdc;
	hdc = GetDCEx(hwndScr, NULL, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
	ClientToScreen(hwnd, &ptBeg);
	ClientToScreen(hwnd, &ptEnd);
	PatBlt(hdc, ptBeg.x, ptBeg.y, ptEnd.x - ptBeg.x, ptEnd.y - ptBeg.y, DSTINVERT);
	ReleaseDC(hwndScr, hdc);
}

HBITMAP CopyBitmap(HBITMAP hBitmapSrc)
{
	BITMAP bitmap;
	HBITMAP hBitmapDst;
	HDC hdcSrc, hdcDst;
	GetObject(hBitmapSrc, sizeof(BITMAP), &bitmap);
	hBitmapDst = CreateBitmapIndirect(&bitmap);
	hdcSrc = CreateCompatibleDC(NULL);
	hdcDst = CreateCompatibleDC(NULL);
	SelectObject(hdcSrc, hBitmapSrc);
	SelectObject(hdcDst, hBitmapDst);
	BitBlt(hdcDst, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcSrc, 0, 0, SRCCOPY);
	DeleteDC(hdcSrc);
	DeleteDC(hdcDst);
	return hBitmapDst;
}

//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BLOWUP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BLOWUP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bCapturing, bBlocking;
	static HBITMAP hBitmap;
	static HWND hwndScr;
	static POINT ptBeg, ptEnd;
	BITMAP bm;
	HBITMAP hBitmapClip;
	HDC hdc, hdcMem;
	int iEnable;
	PAINTSTRUCT ps;
	RECT rect;
	switch (message)
	{
	case WM_LBUTTONDOWN:
		if (!bCapturing)
		{
			if (LockWindowUpdate(hwndScr = GetDesktopWindow()))
			{
				bCapturing = TRUE;
				SetCapture(hwnd);
				SetCursor(LoadCursor(NULL, IDC_CROSS));
			}
			else
				MessageBeep(0);
		}
		return 0;
	case WM_RBUTTONDOWN:
		if (bCapturing)
		{
			bBlocking = TRUE;
			ptBeg.x = LOWORD(lParam);
			ptBeg.y = HIWORD(lParam);
			ptEnd = ptBeg;
			InvertBlock(hwndScr, hwnd, ptBeg, ptEnd);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (bBlocking)
		{
			InvertBlock(hwndScr, hwnd, ptBeg, ptEnd);
			ptEnd.x = LOWORD(lParam);
			ptEnd.y = HIWORD(lParam);
			InvertBlock(hwndScr, hwnd, ptBeg, ptEnd);
		}
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if (bBlocking)
		{
			InvertBlock(hwndScr, hwnd, ptBeg, ptEnd);
			ptEnd.x = LOWORD(lParam);
			ptEnd.y = HIWORD(lParam);
			if (hBitmap)
			{
				DeleteObject(hBitmap);
				hBitmap = NULL;
			}
			hdc = GetDC(hwnd);
			hdcMem = CreateCompatibleDC(hdc);
			hBitmap = CreateCompatibleBitmap(hdc, abs(ptEnd.x - ptBeg.x), abs(ptEnd.y - ptBeg.y));
			SelectObject(hdcMem, hBitmap);
			StretchBlt(hdcMem, 0, 0, abs(ptEnd.x - ptBeg.x), abs(ptEnd.y - ptBeg.y), hdc, ptBeg.x, ptBeg.y, ptEnd.x - ptBeg.x, ptEnd.y - ptBeg.y, SRCCOPY);
			DeleteDC(hdcMem);
			ReleaseDC(hwnd, hdc);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		if (bBlocking || bCapturing)
		{
			bBlocking = bCapturing = FALSE;
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			ReleaseCapture();
			LockWindowUpdate(NULL);
		}
		return 0;
	case WM_INITMENUPOPUP:
		iEnable = IsClipboardFormatAvailable(CF_BITMAP) ? MF_ENABLED : MF_GRAYED;
		EnableMenuItem((HMENU)wParam, IDM_EDIT_PASTE, iEnable);
		iEnable = hBitmap ? MF_ENABLED : MF_GRAYED;
		EnableMenuItem((HMENU)wParam, IDM_EDIT_CUT, iEnable);
		EnableMenuItem((HMENU)wParam, IDM_EDIT_COPY, iEnable);
		EnableMenuItem((HMENU)wParam, IDM_EDIT_DELETE, iEnable);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_EDIT_CUT:
		case IDM_EDIT_COPY:
			if (hBitmap)
			{
				hBitmapClip = CopyBitmap(hBitmap);
				OpenClipboard(hwnd);
				EmptyClipboard();
				SetClipboardData(CF_BITMAP, hBitmapClip);
			}
			if (LOWORD(wParam) == IDM_EDIT_COPY)
				return 0;
			//fall through for IDM_EDIT_CUT
		case IDM_EDIT_DELETE:
			if (hBitmap)
			{
				DeleteObject(hBitmap);
				hBitmap = NULL;
			}
			InvalidateRect(hwnd, NULL,
				TRUE);
			return 0;
		case IDM_EDIT_PASTE:
			if (hBitmap)
			{
				DeleteObject(hBitmap);
				hBitmap = NULL;
			}
			OpenClipboard(hwnd);
			hBitmapClip = (HBITMAP)GetClipboardData(CF_BITMAP);
			if (hBitmapClip)
				hBitmap = CopyBitmap(hBitmapClip);
			CloseClipboard();
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if (hBitmap)
		{
			GetClientRect(hwnd, &rect);
			hdcMem = CreateCompatibleDC(hdc);
			SelectObject(hdcMem, hBitmap);
			GetObject(hBitmap, sizeof(BITMAP), (PSTR)&bm);
			SetStretchBltMode(hdc, COLORONCOLOR);
			StretchBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
			DeleteDC(hdcMem);
		}
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		if (hBitmap)
			DeleteObject(hBitmap);
		PostQuitMessage(0);
		return 0;
 }
 return DefWindowProc(hwnd, message, wParam, lParam);
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
