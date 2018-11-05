#include <Windows.h>
#include "Game.h"

namespace
{
	std::unique_ptr<Game> g_game;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// �G���g���|�C���g
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// �������g�p����Warning�΍�
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr)){
		return 1;
	}

	// Game�N���X�̃C���X�^���X���쐬
	g_game = std::make_unique<Game>();

	// �E�B���h�E�N���X����p��
	const wchar_t CLASS_NAME[] = L"01 SimpleWindow";

	// Window�N���X�̍쐬
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

	// �E�B���h�E�N���X��o�^
	if (!RegisterClassEx(&wcex))
	{
		// �o�^�Ɏ��s�����ꍇ
		DWORD dwError = GetLastError();
		if (dwError != ERROR_CLASS_ALREADY_EXISTS)
		{
			return HRESULT_FROM_WIN32(dwError);
		}
	}

	// �E�B���h�E�C���X�^���X�̍쐬 =========================================

	// �E�C���h�E�N���X�̃N���C�A���g�̈�(=DirectX�̕`��̈�)���w��
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// HWND �̓E�B���h�E�̃n���h��
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

	// �쐬���s
	if (hwnd == NULL)
	{
		DWORD dwError = GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}
	// ======================================================================

	// �E�B���h�E�̕\��
	ShowWindow(hwnd, nCmdShow);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_game.get()));

	// Game�C���X�^���X�̏�����
	g_game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

	// ���b�Z�[�W���[�v =====================================================
	bool bGotMsg;
	MSG msg;
	msg.message = WM_NULL;

	// ���b�Z�[�W���L���[������o��
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	// WM_QUIT���b�Z�[�W������܂Ń��[�v����
	// ���b�Z�[�W������ �� WM_QUIT�ł͂Ȃ�
	// �E�B���h�E�v���V�[�W���� PostQuitMessage(0); �����s���ꂽ���WM_QUIT�������Ă���
	while (WM_QUIT != msg.message)
	{
		// ���b�Z�[�W���L���[������o��
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);

		if (bGotMsg)
		{
			// �L�[�{�[�h���͂�����
			TranslateMessage(&msg);

			// OS�ɃE�B���h�E�v���V�[�W�����Ăяo���悤�ʒm����
			DispatchMessage(&msg);
		}
		else
		{
			// ���b�Z�[�W���Ȃ��ꍇ
			g_game->Tick();
		}
	}
	// ======================================================================

	return 0;
}

// �E�B���h�E�v���V�[�W��
// DispatchMessage ���Ăяo���ꂽ���ƁAOS����Ă΂��
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

	// �A�v���P�[�V�����I�����i�E�B���h�E����������j
	case WM_DESTROY:
		// ���b�Z�[�W���M���I�����Ă��炤
		PostQuitMessage(0);
		return 0;
	}
	// �f�t�H���g�̑Ή������Ă��炤
	return DefWindowProc(hWnd, message, wParam, lParam);
}
