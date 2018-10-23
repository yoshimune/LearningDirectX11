#include "directx11.h"

#include <algorithm>
#include <memory>
#include <fstream>

#include <DirectXMath.h>

#include "DirectXTex/WICTextureLoader/WICTextureLoader.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace {
	struct BinFile
	{
		BinFile(const wchar_t* fpath)
		{
			std::ifstream binFIle(fpath, std::ios::in | std::ios::binary);

			if (binFIle.is_open())
			{
				int fsize = static_cast<int>(binFIle.seekg(0, std::ios::end).tellg());
				binFIle.seekg(0, std::ios::beg);
				std::unique_ptr<char> code(new char[fsize]);
				binFIle.read(code.get(), fsize);
				nSize = fsize;
				Bin = std::move(code);
			}
		}

		const void* get() const { return Bin.get(); }
		int size() const { return nSize; }

	private:
		int nSize = 0;
		std::unique_ptr<char> Bin;
	};

	// ���_�f�[�^�\����
	struct VertexData
	{
		FLOAT x, y, z;
		FLOAT tx, ty;
	};

	// �V�F�[�_�[�萔�o�b�t�@
	struct ConstBuffer
	{
		XMMATRIX mtxProj;
		XMMATRIX mtxView;
		XMMATRIX mtxWorld;
		XMVECTOR Diffuse;
	};
}

AppDX11::AppDX11()
{
}


AppDX11::~AppDX11()
{
}

HRESULT AppDX11::Init(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT cdev_flags = 0;

#ifdef _DEBUG
		cdev_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	// �X���b�v�`�F�[���ݒ�
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;	// 1/60 = 60fps
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	// DirectX11 �f�o�C�X�ƃX���b�v�`�F�[���쐬
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		cdev_flags, &feature_level, 1, D3D11_SDK_VERSION, &sd,
		&pSwapChain, &pDevice, NULL, &pImContext);

	if (FAILED(hr)) { return hr; }

	hWnd = hwnd;

	// �X���b�v�`�F�[���ɗp�ӂ��ꂽ�o�b�t�@�i2D�e�N�X�`���j���擾
	ComPtr<ID3D11Texture2D> back_buff;
	hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&back_buff));
	if (FAILED(hr)) { return hr; }

	// �����_�[�^�[�Q�b�g�r���[�̍쐬
	hr = pDevice->CreateRenderTargetView(back_buff.Get(), NULL, &pRTView);
	if (FAILED(hr)) { return hr; }

	// �����_�[�^�[�Q�b�g�r���[�o�^
	ID3D11RenderTargetView* rtv[1] = { pRTView.Get() };
	pImContext->OMSetRenderTargets(1, rtv, NULL);
	// ComPtr���g�p���Ă���ꍇ�A&pRTView��n���Ȃ��悤�ɂ���
	// ComPtr�̓��e�����������Ȃ邽��

	// viewport
	Viewport.Width = static_cast<FLOAT>(width);
	Viewport.Height = static_cast<FLOAT>(height);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	pImContext->RSSetViewports(1, &Viewport);

	// �V�F�[�_�[�ǂݍ���
	BinFile vscode(L"vshader.cso");
	BinFile pscode(L"pshader.cso");

	// ���_�V�F�[�_�쐬
	// FIXME �V�F�[�_�[���f�o�b�O��񂠂�ŃR���p�C�������
	// �����ŃG���[����
	hr = pDevice->CreateVertexShader(vscode.get(), vscode.size(), NULL, &pVertexShader);
	if (FAILED(hr)) { return hr; }

	// �s�N�Z���V�F�[�_�쐬
	hr = pDevice->CreatePixelShader(pscode.get(), pscode.size(), NULL, &pPixelShader);
	if (FAILED(hr)) { return hr; }

	// ���̓��C�A�E�g��`
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	UINT elem_num = ARRAYSIZE(layout);

	// ���̓��C�A�E�g�쐬
	hr = pDevice->CreateInputLayout(layout, elem_num, vscode.get(), vscode.size(), &pVertexLayout);

	// ���_�o�b�t�@�쐬
	VertexData vertices[] = {
		{ 0.0f, 0.5f, 0.0f, 0.5f, 0.0f },
		{ 0.5f, -0.5f, 0.0f, 1.0f, 1.0f },
		{ -0.5f, -0.5f, 0.0f, 0.0f, 1.0f },
	};
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VertexData) * 3;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertices;

		hr = pDevice->CreateBuffer(&bd, &InitData, &pVertexBuffer);
		if (FAILED(hr)) { return hr; }
	}


	// �C���f�b�N�X�o�b�t�@
	UINT indices[] = { 0,1,2 };
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * 3;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = indices;

		hr = pDevice->CreateBuffer(&bd, &InitData, &pIndexBuffer);
		if (FAILED(hr)) { return hr; }
	}

	// �e�N�X�`���쐬
	hr = CreateWICTextureFromFile(pDevice.Get(), L"data\\image.png", &pTexture, &pShaderResourceView);
	if (FAILED(hr)) { return hr; }

	// �T���v���[�쐬
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pDevice->CreateSamplerState(&sampDesc, &pSampler);
	if (FAILED(hr)) { return hr; }


	// �萔�o�b�t�@
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = pDevice->CreateBuffer(&bd, nullptr, &pCBuffer);
		if (FAILED(hr)) { return hr; }
	}
	
	// ���X�^���C�U�X�e�[�g
	CD3D11_DEFAULT default_state;
	CD3D11_RASTERIZER_DESC rsdesc(default_state);
	rsdesc.CullMode = D3D11_CULL_NONE;
	hr = pDevice->CreateRasterizerState(&rsdesc, &pRsState);
	if (FAILED(hr)) { return hr; }

	// �f�v�X�X�e���V���X�e�[�g
	CD3D11_DEPTH_STENCIL_DESC dsdesc(default_state);
	hr = pDevice->CreateDepthStencilState(&dsdesc, &pDsState);
	if (FAILED(hr)) { return hr; }

	// �u�����h�X�e�[�g
	CD3D11_BLEND_DESC bddesc(default_state);
	hr = pDevice->CreateBlendState(&bddesc, &pBdState);
	if (FAILED(hr)) { return hr; }

	return hr;
}

void AppDX11::Update()
{
	RotateY += 0.01f;
	if (RotateY > XM_2PI) { RotateY -= XM_2PI; }
}

void AppDX11::Render()
{
	// �w��F�ĉ�ʃN���A
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	pImContext->ClearRenderTargetView(pRTView.Get(), ClearColor);

	// ���_�o�b�t�@
	UINT vb_slot = 0;
	ID3D11Buffer* vb[1] = { pVertexBuffer.Get() };
	UINT stride[1] = { sizeof(VertexData) };
	UINT offset[1] = { 0 };
	pImContext->IASetVertexBuffers(vb_slot, 1, vb, stride, offset);

	// ���̓��C�A�E�g
	pImContext->IASetInputLayout(pVertexLayout.Get());

	// �C���f�b�N�X�o�b�t�@
	pImContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// �v���~�e�B�u�`��
	pImContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�V�F�[�_�[
	pImContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
	pImContext->PSSetShader(pPixelShader.Get(), nullptr, 0);

	// �T���v���[
	UINT smp_slot = 0;
	ID3D11SamplerState* smp[1] = { pSampler.Get() };
	pImContext->PSSetSamplers(smp_slot, 1, smp);

	// �V�F�[�_�[���\�[�X�r���[(texture)
	UINT srv_slot = 0;
	ID3D11ShaderResourceView* srv[1] = { pShaderResourceView.Get() };
	pImContext->PSSetShaderResources(srv_slot, 1, srv);

	// �萔�o�b�t�@
	ConstBuffer cbuff;

	// �v���W�F�N�V�����s��
	FLOAT aspect = Viewport.Width / Viewport.Height;
	FLOAT min_z = 0.01f;
	FLOAT max_z = 1000.0f;
	FLOAT fov = XM_PIDIV4;	// ��p
	cbuff.mtxProj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(fov, aspect, min_z, max_z));

	// �J�����s��
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -1.5f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	cbuff.mtxView = XMMatrixTranspose(XMMatrixLookAtLH(Eye, At, Up));

	cbuff.mtxWorld = XMMatrixTranspose(XMMatrixRotationY(RotateY));
	cbuff.Diffuse = XMVectorSet(1.0f, 1.0f, 0.5f, 1);
	// �V�F�[�_�[�ł͍s���]�u���Ă���n��

	// �萔�o�b�t�@�̓��e���X�V
	pImContext->UpdateSubresource(pCBuffer.Get(), 0, NULL, &cbuff, 0, 0);

	// �萔�o�b�t�@
	UINT cb_slot = 0;
	ID3D11Buffer* cb[1] = { pCBuffer.Get() };
	pImContext->VSSetConstantBuffers(cb_slot, 1, cb);
	pImContext->PSSetConstantBuffers(cb_slot, 1, cb);

	// ���X�^���C�U�X�e�[�g
	pImContext->RSSetState(pRsState.Get());

	// �f�v�X�X�e���V���X�e�[�g
	pImContext->OMSetDepthStencilState(pDsState.Get(), 0);

	// �u�����h�X�e�[�g
	pImContext->OMSetBlendState(pBdState.Get(), NULL, 0xffffffff);

	// �|���S���`��
	pImContext->DrawIndexed(3, 0, 0);

	// ���ʂ��E�C���h�E�ɔ��f
	pSwapChain->Present(0, 0);
}
