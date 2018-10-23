#pragma once

#include "pch.h"

class DeviceResources
{
public:
	DeviceResources();
	~DeviceResources();

	HRESULT CreateDeviceResources();
	HRESULT CreateWindowResources(HWND hWnd);

	HRESULT ConfigureBackBuffer();
	HRESULT ReleaseBackBuffer();
	HRESULT GoFullScreen();
	HRESULT GoWindowed();

	float GetAspectRatio();

	ID3D11Device*           GetDevice() { return m_pd3dDevice.Get(); };
	ID3D11DeviceContext*    GetDeviceContext() { return m_pd3dDeviceContext.Get(); };
	ID3D11RenderTargetView* GetRenderTarget() { return m_pRenderTarget.Get(); }
	ID3D11DepthStencilView* GetDepthStencil() { return m_pDepthStencilView.Get(); }

	void Present();

private:
	// このフラグはAPIによってカラーちゃんねるの並びが異なるサーフェイスのためサポートを追加する
	// Direct2Dとの互換性のために要求される
	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	const D3D_FEATURE_LEVEL m_levels[7] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	Microsoft::WRL::ComPtr<ID3D11Device> m_pd3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pd3dDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_pDXGISwapChain;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pBackBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTarget;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pDepthStencil;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;

	D3D_FEATURE_LEVEL m_featureLevel;
	D3D11_TEXTURE2D_DESC m_bbDesc;
	D3D11_VIEWPORT m_viewport;
};

