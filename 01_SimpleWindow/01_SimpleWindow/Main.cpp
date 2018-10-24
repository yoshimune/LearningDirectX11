#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// エントリポイント
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// 引数未使用時のWarning対策
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

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
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// HWND はウィンドウのハンドラ
	HWND hwnd = CreateWindowEx(
		0,								// Optional window styles.
		CLASS_NAME,						// Window class
		L"01 Simple Window",			// Window text
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
			// 今回は何もしていないが、ゲームなどの場合は、ここでUpdateとRenderを行う
		}
	}
	// ======================================================================

	return 0;
}

// ウィンドウプロシージャ
// DispatchMessage が呼び出されたあと、OSから呼ばれる
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	// アプリケーション終了時（ウィンドウが消えた後）
	case WM_DESTROY:
		// メッセージ送信を終了してもらう
		PostQuitMessage(0);
		return 0;

	// その他のメッセージ
	default:
		// デフォルトの対応をしてもらう
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}