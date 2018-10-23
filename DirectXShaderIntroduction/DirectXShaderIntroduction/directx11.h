#pragma once

#include <Windows.h>
#include <wrl/client.h>
#include <d3d11_1.h>

class AppDX11
{
public:
	AppDX11();
	~AppDX11();

	HRESULT Init(HWND hwnd);

	void Update();
	void Render();

private:
	HWND hWnd = NULL;
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pImContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRTView;
	D3D11_VIEWPORT Viewport;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pVertexLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Resource> pTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;

	Microsoft::WRL::ComPtr<ID3D11Buffer> pCBuffer;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRsState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDsState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> pBdState;

	// ƒ|ƒŠƒSƒ“‰ñ“]Šp“x
	FLOAT RotateY = 0.0f;
};

