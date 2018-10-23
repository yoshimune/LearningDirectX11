#include "DeviceResources.h"
//#include "dxgi1_3.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

DeviceResources::DeviceResources()
{
}


DeviceResources::~DeviceResources()
{
}

// Create Device
HRESULT DeviceResources::CreateDeviceResources()
{
	HRESULT hr = S_OK;

#if defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Direct3D 11 API device と対応した Context を作成
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;


	DX::ThrowIfFailed(D3D11CreateDevice(
		nullptr,                            // specify nullptr to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,			// Create a device using the hardware
		0,									// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		deviceFlags,						// Set debug and Direct2D compatibility flags.
		m_levels,							// List of feature levels this app can support.
		_countof(m_levels),					// Size of the list above.
		D3D11_SDK_VERSION,					// Always set this to D3D11_SDK_VERSION for Windows Store apps.
		device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
		&m_featureLevel,                    // returns feature level of device created
		context.ReleaseAndGetAddressOf()    // returns the device immediate context
	));

	DX::ThrowIfFailed(device.As(&m_pd3dDevice));
	DX::ThrowIfFailed(context.As(&m_pd3dDeviceContext));
	
	return hr;
}

// Create SwapChain
HRESULT DeviceResources::CreateWindowResources(HWND hWnd)
{
	HRESULT hr = S_OK;

#if defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
	desc.Windowed = TRUE;
	desc.BufferCount = 2;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SampleDesc.Count = 1;		// multisampling setting
	desc.SampleDesc.Quality = 0;	// vendor-specific flag
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.OutputWindow = hWnd;

	// Create the DXGI device object to use in other factories, such as Direct2D
	ComPtr<IDXGIDevice> dxgiDevice;
	DX::ThrowIfFailed(m_pd3dDevice.As(&dxgiDevice));

	// Create swap chain
	ComPtr<IDXGIAdapter> adapter;
	ComPtr<IDXGIFactory> factory;

	DX::ThrowIfFailed(dxgiDevice->GetAdapter(adapter.GetAddressOf()));

	DX::ThrowIfFailed(adapter->GetParent(IID_PPV_ARGS(factory.GetAddressOf())));

	DX::ThrowIfFailed(factory->CreateSwapChain(
		m_pd3dDevice.Get(),
		&desc,
		&m_pDXGISwapChain));

	// Configure the back buffer, stencil buffer, and viewport.
	DX::ThrowIfFailed(ConfigureBackBuffer());

	return hr;
}

HRESULT DeviceResources::ConfigureBackBuffer()
{
	HRESULT hr = S_OK;

	DX::ThrowIfFailed(m_pDXGISwapChain->GetBuffer(
		0,
		IID_PPV_ARGS(m_pBackBuffer.GetAddressOf())
	));

	m_pBackBuffer->GetDesc(&m_bbDesc);

	// Create a depth-stencil view for use with 3D rendering if needed.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		static_cast<UINT> (m_bbDesc.Width),
		static_cast<UINT> (m_bbDesc.Height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
	);

	m_pd3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		&m_pDepthStencil
	);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

	m_pd3dDevice->CreateDepthStencilView(
		m_pDepthStencil.Get(),
		&depthStencilViewDesc,
		&m_pDepthStencilView
	);

	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.Height = (float)m_bbDesc.Height;
	m_viewport.Width = (float)m_bbDesc.Width;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;

	m_pd3dDeviceContext->RSSetViewports(
		1,
		&m_viewport
	);

	return hr;
}


HRESULT DeviceResources::ReleaseBackBuffer()
{
	HRESULT hr = S_OK;

	// Release the render target view based on the back buffer:
	m_pRenderTarget.Reset();

	// Release the back buffer itself:
	m_pBackBuffer.Reset();

	// The depth stencil will need to be resized, so release it (and view)
	m_pDepthStencilView.Reset();
	m_pDepthStencil.Reset();

	// After releasing references to these resources, we need to call Flush() to
	// ensure that Direct3D also releases any references it might still have to
	// the same resources - such as pipeline bindings.

	// これらのリソースの参照を解放した後、確実にFlush()を呼び、
	// Direct3Dがパイプラインバインディングなどの同じリソースに対して引き続き
	// 参照できるようにする必要があります。
	m_pd3dDeviceContext->Flush();

	return hr;
	
}

HRESULT DeviceResources::GoFullScreen()
{
	HRESULT hr = S_OK;

	DX::ThrowIfFailed(m_pDXGISwapChain->SetFullscreenState(TRUE, NULL));

	// Swap chains created with the DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL flag need to
	// call ResizeBuffers in order to realize a full-screen mode switch.
	// your next call to Present will fail.

	// XGI_SWAP_EFFECT_FLIP_SEQUENTIALフラグから作成したスワップチェーンは、
	// フルスクリーンモードにスイッチすることを実現するため、
	// 必要に応じてResizeButtersを呼ぶことが必要。

	// Before calling ResizeBuffers, you have to release all references to the back
	// buffer device resource.

	// ResizeBufferを呼ぶ前に、全てのバックバッファデバイスリソースに
	// 対する参照を解放しなくてはならない。
	ReleaseBackBuffer();

	// Now we can call ResizeBuffers.
	DX::ThrowIfFailed(m_pDXGISwapChain->ResizeBuffers(
		0,						// Number of Buffers. Set this to 0 to preserve the existing setting.
		0, 0,					// Width and Height of the swap chain. Set to 0 to match the screen resolution.
		DXGI_FORMAT_UNKNOWN,	// This tells DXGI to retain the current back buffer format.
		0
	));

	// Then we can recreate the back buffer, depth buffer, and so on.
	DX::ThrowIfFailed(ConfigureBackBuffer());

	return hr;
}
HRESULT DeviceResources::GoWindowed()
{
	HRESULT hr = S_OK;

	DX::ThrowIfFailed(m_pDXGISwapChain->SetFullscreenState(FALSE, NULL));

	// SwapChains created with the DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL flag need to
	// call ResizeBuffers in order to realize a change to windowed mode.
	// Otherwise, your next call to Present will fail.

	// DXGI_SWAP_EFFECT_FLIP_SEQUENTIALフラグより作成されたスワップチェーンは、
	// ウィンドウモードに変更することを実現するため、ResizeBuffersを呼ぶ必要がある。
	// 一方、次のPresent呼び出しは失敗する。

	// Before calling ResizeBuffers, you have to release all references to the
	// back buffer device resources.

	// ResizeBuffersを呼ぶ前に、全てのバックバッファデバイスリソース参照を
	// 解放する必要がある。

	// Now we can call ResizeBuffers
	DX::ThrowIfFailed(
		m_pDXGISwapChain->ResizeBuffers(
			0,						// Number of buffers. Set this to 0 preserve the existing setting.
			0, 0,					// Width and height of the swap chain. MUST be set to a non-zero value. For example, match the window size.
			DXGI_FORMAT_UNKNOWN,	// This tells DXGI to retain the current back buffer format.
			0
		)
	);

	return hr;
}

float DeviceResources::GetAspectRatio()
{
	return static_cast<float>(m_bbDesc.Width) / static_cast<float>(m_bbDesc.Height);
}

void DeviceResources::Present()
{
	m_pDXGISwapChain->Present(1, 0);
}
