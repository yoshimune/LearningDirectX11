#ifndef UNICODE
#define UNICODE
#endif 

#include <Windows.h>
#include <new>

struct StateInfo
{
	int state = 100;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	//Create the window.

	// #include <new> ‚ª•K—v
	StateInfo *pState = new (std::nothrow) StateInfo;

	HWND hwnd = CreateWindowEx(
		0,								// Optional window styles.
		CLASS_NAME,						// Window class
		L"Learn to Program Windows",	// Window text
		WS_OVERLAPPEDWINDOW,			// Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,		// Parent Window
		NULL,		// Menu
		hInstance,	// Instance handle
		pState		// Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	// Run the message loop.

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

inline StateInfo* GetAppState(HWND hwnd)
{
	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	StateInfo *pState = reinterpret_cast<StateInfo*>(ptr);
	return pState;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	StateInfo *pState;
	if (uMsg == WM_CREATE)
	{
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pState = reinterpret_cast<StateInfo*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pState);
	}
	else
	{
		pState = GetAppState(hwnd);
	}

	switch (uMsg)
	{
	case WM_CLOSE:
		if (MessageBox(hwnd, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(hwnd);
		}

		// Else :User canceled. Do nothing;
		return 0;
			
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		
	case WM_PAINT:
	
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hwnd, &ps);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}