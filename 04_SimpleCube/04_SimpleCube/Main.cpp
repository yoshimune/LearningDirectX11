#include <Windows.h>
#include "Game.h"

namespace
{
	std::unique_ptr<Game> g_game;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// エントリポイント
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// 引数未使用時のWarning対策
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr)){
		return 1;
	}

	// Gameクラスのインスタンスを作成
	g_game = std::make_unique<Game>();

	// ウィンドウクラス名を用意
	const wchar_t CLASS_NAME[] = L"01 SimpleWindow";

	// Windowクラスの作成
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASS_NAME;
	wcex.hIconSm = NULL;

	// ウィンドウクラスを登録
	if (!RegisterClassEx(&wcex))
	{
		// 登録に失敗した場合
		DWORD dwError = GetLastError();
		if (dwError != ERROR_CLASS_ALREADY_EXISTS)
		{
			return HRESULT_FROM_WIN32(dwError);
		}
	}

	// ウィンドウインスタンスの作成 =========================================

	// ウインドウクラスのクライアント領域(=DirectXの描画領域)を指定
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// HWND はウィンドウのハンドラ
	HWND hwnd = CreateWindowEx(
		0,								// Optional window styles.
		CLASS_NAME,						// Window class
		L"03 Simple Triangle",			// Window text
		WS_OVERLAPPEDWINDOW,			// Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, (rc.right - rc.left), (rc.bottom - rc.top),

		NULL,		// Parent Window
		NULL,		// Menu
		hInstance,	// Instance handle
		NULL		// Additional application data
	);

	// 作成失敗
	if (hwnd == NULL)
	{
		DWORD dwError = GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}
	// ======================================================================

	// ウィンドウの表示
	ShowWindow(hwnd, nCmdShow);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_game.get()));

	// Gameインスタンスの初期化
	g_game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

	// メッセージループ =====================================================
	bool bGotMsg;
	MSG msg;
	msg.message = WM_NULL;

	// メッセージをキューから取り出す
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	// WM_QUITメッセージが来るまでループする
	// メッセージが無い ≠ WM_QUITではない
	// ウィンドウプロシージャで PostQuitMessage(0); が実行された後にWM_QUITが送られてくる
	while (WM_QUIT != msg.message)
	{
		// メッセージをキューから取り出す
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);

		if (bGotMsg)
		{
			// キーボード入力を扱う
			TranslateMessage(&msg);

			// OSにウィンドウプロシージャを呼び出すよう通知する
			DispatchMessage(&msg);
		}
		else
		{
			// メッセージがない場合
			g_game->Tick();
		}
	}
	// ======================================================================

	return 0;
}

// ウィンドウプロシージャ
// DispatchMessage が呼び出されたあと、OSから呼ばれる
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	static bool s_in_sizemove = false;
	static bool s_in_suspend = false;
	static bool s_minimized = false;
	static bool s_fullscreen = false;
	// TODO: Set s_fullscreen to true if defaulting to fullscreen.

	auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_PAINT:
		if (s_in_sizemove && game)
		{
			game->Tick();
		}
		else
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
	
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			if (!s_minimized)
			{
				s_minimized = true;
				if (!s_in_suspend && game) 
				{ 
					//game->OnSuspending();
				}
				s_in_suspend = true;
			}
		}
		else if (s_minimized)
		{
			s_minimized = false;
			if (s_in_suspend && game)
			{
				//game->OnResuming();
			}
			s_in_suspend = false;
		}
		else if (!s_in_sizemove && game)
		{
			//game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_ENTERSIZEMOVE:
		s_in_sizemove = true;
		break;
		
	case WM_EXITSIZEMOVE:
		s_in_sizemove = false;
		if (game)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			// game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;

	case WM_POWERBROADCAST:
		switch (wParam)
		{
		case PBT_APMQUERYSUSPEND:
			if (!s_in_suspend && game)
			{
				//game->OnSuspending();
			}
			s_in_suspend = true;
			return TRUE;

		case PBT_APMRESUMESUSPEND:
			if (!s_minimized)
			{
				if (s_in_suspend && game)
				{
					//game->OnResuming();
				}
				s_in_suspend = false;
			}
			return TRUE;
		}
		break;

	// アプリケーション終了時（ウィンドウが消えた後）
	case WM_DESTROY:
		// メッセージ送信を終了してもらう
		PostQuitMessage(0);
		return 0;
	}
	// デフォルトの対応をしてもらう
	return DefWindowProc(hWnd, message, wParam, lParam);
}
