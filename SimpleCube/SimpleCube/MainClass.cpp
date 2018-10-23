#include "MainClass.h"
#include "shellapi.h"

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	HRESULT hr = S_OK;

	// Enable run-time memory check for debug builds.

#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Begin initialization.

	// Instantiate the window manager class.
	std::shared_ptr<MainClass> winMain = std::make_shared<MainClass>();

	// Create a window
	hr = winMain->CreateDesktopWindow();
	if (SUCCEEDED(hr))
	{
		// Instantiate the device manager class.
		std::shared_ptr<DeviceResources> deviceResources = std::make_shared<DeviceResources>();
		// Create device resources.
		deviceResources->CreateDeviceResources();

		// Instantiate the renderer.
		std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(deviceResources);
		renderer->CreateDeviceDependentResource();

		// We have a window, so initialize window size-dependent resources.
		deviceResources->CreateWindowResources(winMain->GetWindowHandle());
		renderer->CreateWindowSizeDependentResource();

		// Go full-screen
		deviceResources->GoFullScreen();

		// Whoops! We resized the "window" when we went full-screen.
		// Better tell the renderer.
		renderer->CreateWindowSizeDependentResource();

		//Run the Program.
		hr = winMain->Run(deviceResources, renderer);
	}

	// Cleanup is handled in destructors.
	return hr;
}

MainClass::MainClass()
{
	m_windowClassName = L"Direct3DWindowClass";
	m_hInstance = NULL;
}


MainClass::~MainClass()
{
}

// Create a window for our Direct3D viewport
HRESULT MainClass::CreateDesktopWindow()
{
	// Window resources are dealt with here.

	if (m_hInstance == NULL)
	{
		m_hInstance = (HINSTANCE)GetModuleHandle(NULL);
	}

	HICON hIcon = NULL;
	WCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);

	// If the icon is NULL, then use the first one found in the exe
	if (hIcon == NULL)
	{
		hIcon = ExtractIcon(m_hInstance, szExePath, 0);
	}

	// Register the windows class
	WNDCLASS wndClass;
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = MainClass::WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = m_hInstance;
	wndClass.hIcon = hIcon;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = m_windowClassName.c_str();

	if (!RegisterClass(&wndClass))
	{
		DWORD dwError = GetLastError();
		if (dwError != ERROR_CLASS_ALREADY_EXISTS)
		{
			return HRESULT_FROM_WIN32(dwError);
		}
	}

	m_rc;
	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;

	// No menu in thie example.
	m_hMenu = NULL;

	// This example uses a non-resizable 640 by 480 viewport for simplicity.
	int nDefaultWidth = 640;
	int nDefaultHeight = 480;
	SetRect(&m_rc, 0, 0, nDefaultWidth, nDefaultHeight);
	AdjustWindowRect(
		&m_rc,
		WS_OVERLAPPEDWINDOW,
		(m_hMenu != NULL) ? true: false
	);

	// Create the Window for our viewport.
	m_hWnd = CreateWindow(
		m_windowClassName.c_str(),
		L"Cube11",
		WS_OVERLAPPEDWINDOW,
		x, y,
		(m_rc.right - m_rc.left), (m_rc.bottom - m_rc.top),
		0,
		m_hMenu,
		m_hInstance,
		0
	);

	if (m_hWnd == NULL)
	{
		DWORD dwError = GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}

	return S_OK;
}

HRESULT MainClass::Run(
	std::shared_ptr<DeviceResources> deviceResources,
	std::shared_ptr<Renderer> renderer
)
{
	HRESULT hr = S_OK;

	if (!IsWindowVisible(m_hWnd))
	{
		ShowWindow(m_hWnd, SW_SHOW);
	}

	// The render loop is controlled here.
	bool bGotMsg;
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		// Process window events.
		// Use PeekMessage() so we can idle time to render the scene.
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);

		if (bGotMsg)
		{
			// Translate and dispatch the message
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Update the scene.
			renderer->Update();

			// Renderer frames during idle time (when no message are waiting).
			renderer->Render();

			// Present the frame to the screen.
			deviceResources->Present();
		}
	}

	return hr;
}

// Process windows messages.
// This looks for window close events,
// letting us exit out of the sample.
LRESULT CALLBACK MainClass::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		HMENU hMenu = GetMenu(hWnd);
		if (hMenu != NULL)
		{
			DestroyMenu(hMenu);
		}
		DestroyWindow(hWnd);
		UnregisterClass(
			m_windowClassName.c_str(),
			m_hInstance
		);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}