#include "Game.h"
#include <DirectXColors.h>
#include <fstream>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

Game::Game()
	:window_(nullptr)
	,output_width_(800)
	,output_height_(600)
	,featureLevels_(D3D_FEATURE_LEVEL_9_1)
{
}


Game::~Game()
{
}

// 初期化処理
void Game::Initialize(HWND window, int width, int height)
{
	window_ = window;
	output_width_ = width;
	output_height_ = height;

	viewport_.Width = static_cast<FLOAT>(width);
	viewport_.Height = static_cast<FLOAT>(height);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;


	// フレームレート管理カウンタの初期化
	// カウンタの初期化
	QueryPerformanceCounter(&total_count_);

	// 周波数取得
	QueryPerformanceFrequency(&frequency_);


	CreateDevice();
	CreateResources();
}

void Game::Tick()
{
	if (UpdateTimer())
	{
		Update();
		Render();
	}
}

bool Game::UpdateTimer()
{
	// 現在のカウント数を取得
	LARGE_INTEGER current_count;
	QueryPerformanceCounter(&current_count);

	// 前回からの経過カウント数を取得
	long long diff_count = current_count.QuadPart - total_count_.QuadPart;

	// カウント数と周波数から経過時間を算出
	float time = float(double(diff_count) / double(frequency_.QuadPart));

	if (time >= 1.0f / FRAME_RATE)
	{
		total_count_ = current_count;
		return true;;
	}

	return false;
}

// ゲームの状態を更新
void Game::Update()
{
}

// 描画する
void Game::Render()
{
	// 画面クリア
	Clear();

	// 頂点バッファ用意
	UINT vb_slot = 0;
	ID3D11Buffer* vb[1] = { d3d_vertex_buffer_.Get() };
	UINT stride[1] = { sizeof(VertexData) };
	UINT offset[1] = { 0 };

	// 頂点バッファをバインド
	d3d_context_->IASetVertexBuffers(vb_slot, 1, vb, stride, offset);

	// インデックスバッファ
	d3d_context_->IASetIndexBuffer(d3d_index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 入力レイアウトオブジェクトをバインド
	d3d_context_->IASetInputLayout(d3d_vertex_layout_.Get());


	// プリミティブ形状
	d3d_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// シェーダー
	d3d_context_->VSSetShader(d3d_vertex_shader_.Get(), nullptr, 0);
	d3d_context_->PSSetShader(d3d_pixel_shader_.Get(), nullptr, 0);

	// 定数バッファ
	ConstantBuffer cbuffer;

	// プロジェクション行列
	FLOAT aspect = viewport_.Width / viewport_.Height;
	FLOAT min_z = 0.01f;
	FLOAT max_z = 100.0f;
	FLOAT fov = XM_PIDIV4; // 画角
	cbuffer.proj_matrix_ = XMMatrixTranspose(XMMatrixPerspectiveFovLH(fov, aspect, min_z, max_z));

	// カメラ行列
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -1.5f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	cbuffer.view_matrix_ = XMMatrixTranspose(XMMatrixLookAtLH(Eye, At, Up));

	cbuffer.model_matrix_ = XMMatrixIdentity();

	// 定数バッファの内容を更新
	d3d_context_->UpdateSubresource(d3d_cbuffer_.Get(), 0, NULL, &cbuffer, 0, 0);

	// 定数バッファ
	UINT cb_slot = 0;
	ID3D11Buffer* cb[1] = { d3d_cbuffer_.Get() };
	d3d_context_->VSSetConstantBuffers(cb_slot, 1, cb);
	d3d_context_->PSSetConstantBuffers(cb_slot, 1, cb);

	// ラスタライザステート
	d3d_context_->RSSetState(d3d_rasterizer_state_.Get());

	// デプス・ステンシルステート
	d3d_context_->OMSetDepthStencilState(d3d_depth_stencil_state_.Get(), 0);

	// ブレンドステート
	d3d_context_->OMSetBlendState(d3d_blend_state_.Get(), NULL, 0xffffffff);

	// ポリゴン描画
	d3d_context_->DrawIndexed(3, 0, 0);

	// 結果をウィンドウに反映
	dxgi_swap_chain_->Present(0, 0);
}

//画面クリア
void Game::Clear()
{
	// 画面クリア
	d3d_context_->ClearRenderTargetView(d3d_render_target_view_.Get(), Colors::Black);
	d3d_context_->ClearDepthStencilView(d3d_depth_stencil_view_.Get(), D3D11_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);
	
	d3d_context_->OMSetRenderTargets(1, d3d_render_target_view_.GetAddressOf(), d3d_depth_stencil_view_.Get());

	// ビューポートをセット
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(output_width_), static_cast<float>(output_height_));
	d3d_context_->RSSetViewports(1, &viewport);
}

// デバイスを作成する
// 基本的にアプリケーションの起動時のみ呼ばれる
void Game::CreateDevice()
{
	// ランタイムレイヤー指定フラグ
	// 指定なし
	UINT creationFlags = 0;

#ifdef _DEBUG
	// デバッグレイヤー
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 対応する Direct3D Feature Level の配列
	static const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// デバイスの作成
	DX::ThrowIfFailed(D3D11CreateDevice(
		nullptr,                            // specify nullptr to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		d3d_device_.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
		&featureLevels_,                    // returns feature level of device created
		d3d_context_.ReleaseAndGetAddressOf()    // returns the device immediate context
	));

	// デバッグ設定
#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3d_debug;
	if (SUCCEEDED(d3d_device_.As(&d3d_debug)))
	{
		ComPtr<ID3D11InfoQueue> d3d_info_queue;
		if (SUCCEEDED(d3d_debug.As(&d3d_info_queue)))
		{
#ifdef _DEBUG
			d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif // _DEBUG
			D3D11_MESSAGE_ID hide [] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3d_info_queue->AddStorageFilterEntries(&filter);
		}
	}
#endif // !NDEBUG

	// パイプラインの設定を行う
	CreateConstantResources();
}

// リソースを作成する
// ウィンドウサイズの変更があった場合に呼ばれる
void Game::CreateResources()
{
	// 以前のウィンドウサイズに合わせたコンテキストをクリアする

	// レンダーターゲットビューの用意
	ID3D11RenderTargetView* nullViews[] = { nullptr };

	// レンダーターゲットビューをコンテキストにバインドする
	d3d_context_->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

	// メンバークリア
	d3d_render_target_view_.Reset();
	d3d_depth_stencil_state_.Reset();

	// リソースオブジェクトの破棄を実行
	d3d_context_->Flush();

	UINT back_buffer_width = static_cast<UINT>(output_width_);
	UINT back_buffer_height = static_cast<UINT>(output_height_);
	DXGI_FORMAT back_buffer_format = DXGI_FORMAT_B8G8R8A8_UNORM;
	DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT back_buffer_count = 2;

	// 既にスワップチェーンが作成されている場合
	if (dxgi_swap_chain_)
	{
		auto hr = dxgi_swap_chain_->ResizeBuffers(back_buffer_count, back_buffer_width, back_buffer_height, back_buffer_format, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// もしデバイスが何らかの理由で削除されている場合は。新しいデバイスとスワップチェーンを作成する
			OnDeviceLost();
			return;
		}
		else
		{
			// その他のエラー
			DX::ThrowIfFailed(hr);
		}
	}
	// スワップチェーンが作成されていない場合
	else
	{
		// D3D Device から DXGI Device を検索します
		ComPtr<IDXGIDevice1> dxgi_device;
		DX::ThrowIfFailed(d3d_device_.As(&dxgi_device));

		// 現在のデバイスで動作中の物理アダプタ(GPU またはカード)を初期化します。
		ComPtr<IDXGIAdapter> dxgi_adapter;
		DX::ThrowIfFailed(dxgi_device->GetAdapter(dxgi_adapter.GetAddressOf()));

		// アダプタから親ファクトリを入手します
		ComPtr<IDXGIFactory2> dxgi_factory;
		DX::ThrowIfFailed(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory.GetAddressOf())));

		// スワップチェーン作成のためのディスクリプタを作成します。
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.Width = back_buffer_width;
		swap_chain_desc.Height = back_buffer_height;
		swap_chain_desc.Format = back_buffer_format;
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.SampleDesc.Quality = 0;
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.BufferCount = back_buffer_count;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fs_swap_chain_desc = {};
		fs_swap_chain_desc.Windowed = TRUE;

		// Win32 windowからスワップチェーンを作成します。
		DX::ThrowIfFailed(dxgi_factory->CreateSwapChainForHwnd(
			d3d_device_.Get(),
			window_,
			&swap_chain_desc,
			&fs_swap_chain_desc,
			nullptr,
			dxgi_swap_chain_.ReleaseAndGetAddressOf()
		));

		// Alt + Enter ショートカットキーに応答しないようにします。
		DX::ThrowIfFailed(dxgi_factory->MakeWindowAssociation(window_, DXGI_MWA_NO_ALT_ENTER));
	}

	// 最終的な3Dレンダーターゲットとなるこのウィンドウのバックバッファーを含めます。
	ComPtr<ID3D11Texture2D> back_buffer;
	DX::ThrowIfFailed(dxgi_swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf())));

	// レンダーターゲットビューインターフェイスをバインドします
	DX::ThrowIfFailed(d3d_device_->CreateRenderTargetView(back_buffer.Get(), nullptr, d3d_render_target_view_.ReleaseAndGetAddressOf()));

	// 2Dサーフェイスをデプス・ステンシルバッファとして割り当てます。
	// デプス・ステンシルビューをバインドするためこのサーフェイス上に作成します。
	CD3D11_TEXTURE2D_DESC depth_stencil_desc(depth_buffer_format, back_buffer_width, back_buffer_height, 1, 1, D3D11_BIND_DEPTH_STENCIL);

	ComPtr<ID3D11Texture2D> depth_stencil;
	DX::ThrowIfFailed(d3d_device_->CreateTexture2D(&depth_stencil_desc, nullptr, depth_stencil.GetAddressOf()));

	CD3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(d3d_device_->CreateDepthStencilView(depth_stencil.Get(), &depth_stencil_view_desc, d3d_depth_stencil_view_.ReleaseAndGetAddressOf()));


}

// ゲーム中固定のリソースを作成する
void Game::CreateConstantResources()
{
	// Shader =====================================================================
	// バイナリファイルからシェーダーを読み込みます
	BinaryData vertex_shader_code(L"vertexShader.cso");
	BinaryData pixel_shader_code(L"pixelShader.cso");

	// 頂点シェーダ作成
	DX::ThrowIfFailed(
		d3d_device_->CreateVertexShader(vertex_shader_code.get(), vertex_shader_code.size(), NULL, d3d_vertex_shader_.GetAddressOf())
	);

	// ピクセルシェーダ作成
	DX::ThrowIfFailed(
		d3d_device_->CreatePixelShader(pixel_shader_code.get(), pixel_shader_code.size(), NULL, d3d_pixel_shader_.GetAddressOf())
	);



	// 入力レイアウト =========================================================
	// 入力レイアウト定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT element_num = _countof(layout);

	// 入力レイアウト作成
	DX::ThrowIfFailed(
		d3d_device_->CreateInputLayout(layout, element_num, vertex_shader_code.get(), vertex_shader_code.size(), d3d_vertex_layout_.GetAddressOf())
	);


	// 頂点バッファ ===========================================================
	{	
		// 頂点バッファ定義
		VertexData vertices[] = {
			{ 0.0f, 0.5f, 0.0f},
			{ 0.5f, -0.5f, 0.0f},
			{ -0.5f, -0.5f, 0.0f},
		};

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VertexData) * 3;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertices;

		// 頂点バッファ作成
		DX::ThrowIfFailed(
			d3d_device_->CreateBuffer(&bd, &InitData, d3d_vertex_buffer_.GetAddressOf())
		);
	}

	// インデックスバッファ ==================================================
	{
		UINT indices[] = { 0,1,2 };
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * 3;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = indices;

		// インデックスバッファ作成
		DX::ThrowIfFailed(
			d3d_device_->CreateBuffer(&bd, &InitData, d3d_index_buffer_.GetAddressOf())
		);
	}

	//定数バッファ ===========================================================
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;

		DX::ThrowIfFailed(
			d3d_device_->CreateBuffer(&bd, nullptr, d3d_cbuffer_.GetAddressOf())
		);
	}

	// ラスタライザステート ===================================================
	CD3D11_DEFAULT default_state;
	CD3D11_RASTERIZER_DESC rs_desc(default_state);
	rs_desc.CullMode = D3D11_CULL_NONE;
	
	DX::ThrowIfFailed(
		d3d_device_->CreateRasterizerState(&rs_desc, d3d_rasterizer_state_.GetAddressOf())
	);

	// デプス・ステンシルステート =============================================
	CD3D11_DEPTH_STENCIL_DESC ds_desc(default_state);
	
	DX::ThrowIfFailed(
		d3d_device_->CreateDepthStencilState(&ds_desc, d3d_depth_stencil_state_.GetAddressOf())
	);

	// ブレンドステート ========================================================
	CD3D11_BLEND_DESC bd_desc(default_state);

	DX::ThrowIfFailed(
		d3d_device_->CreateBlendState(&bd_desc, d3d_blend_state_.GetAddressOf())
	);
}


// デバイスが削除された際にデバイスリソースの再設定を行う
void Game::OnDeviceLost()
{
	// リソースを削除
	d3d_vertex_buffer_.Reset();
	d3d_pixel_shader_.Reset();
	d3d_vertex_layout_.Reset();
	d3d_vertex_buffer_.Reset();
	d3d_index_buffer_.Reset();
	d3d_shader_resource_view_.Reset();
	d3d_cbuffer_.Reset();
	d3d_rasterizer_state_.Reset();
	d3d_depth_stencil_state_.Reset();
	d3d_blend_state_.Reset();
	d3d_render_target_view_.Reset();
	d3d_depth_stencil_view_.Reset();
	dxgi_swap_chain_.Reset();
	d3d_context_.Reset();
	d3d_device_.Reset();

	// 再生成
	CreateDevice();
	CreateResources();
}