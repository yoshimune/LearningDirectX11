#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// �G���g���|�C���g
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// �������g�p����Warning�΍�
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

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
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// HWND �̓E�B���h�E�̃n���h��
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

	// �쐬���s
	if (hwnd == NULL)
	{
		DWORD dwError = GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}
	// ======================================================================

	// �E�B���h�E�̕\��
	ShowWindow(hwnd, nCmdShow);

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
			// ����͉������Ă��Ȃ����A�Q�[���Ȃǂ̏ꍇ�́A������Update��Render���s��
		}
	}
	// ======================================================================

	return 0;
}

// �E�B���h�E�v���V�[�W��
// DispatchMessage ���Ăяo���ꂽ���ƁAOS����Ă΂��
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	// �A�v���P�[�V�����I�����i�E�B���h�E����������j
	case WM_DESTROY:
		// ���b�Z�[�W���M���I�����Ă��炤
		PostQuitMessage(0);
		return 0;

	// ���̑��̃��b�Z�[�W
	default:
		// �f�t�H���g�̑Ή������Ă��炤
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}