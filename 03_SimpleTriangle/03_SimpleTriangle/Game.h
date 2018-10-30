#pragma once

#include <Windows.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>

#include<memory>
#include <fstream>
#include <exception>

// 頂点データ構造体
struct VertexData
{
	FLOAT x, y, z;
};

// シェーダー定数バッファ
struct ConstantBuffer
{
	DirectX::XMMATRIX proj_matrix_;
	DirectX::XMMATRIX view_matrix_;
	DirectX::XMMATRIX model_matrix_;
};

// バイナリデータ管理
struct BinaryData
{
private:
	int size_;
	std::unique_ptr<char> binary_;

public:
	BinaryData(const wchar_t* file_path)
	{
		// ファイルからバイナリデータを読み込む
		std::ifstream file(file_path, std::ios::in | std::ios::binary);

		if (file.is_open())
		{
			int file_size = static_cast<int>(file.seekg(0, std::ios::end).tellg());
			file.seekg(0, std::ios::beg);
			std::unique_ptr<char> code(new char[file_size]);
			file.read(code.get(), file_size);
			size_ = file_size;
			binary_ = std::move(code);
		}
	}

	const void* get() const { return binary_.get(); }
	int size() const { return size_; }
};

class Game
{
public:
	Game();
	~Game();

	// 初期化処理
	void Initialize(HWND window, int width, int height);

	// ゲームループ
	void Tick();

private:

	// タイマー更新
	bool UpdateTimer();
	
	// ゲームの状態を更新
	void Update();

	// 描画する
	void Render();

	// 画面クリア
	void Clear();

	// デバイスを作成する
	void CreateDevice();

	// リソースを作成する
	void CreateResources();

	// ゲーム中固定のリソースを作成する
	void CreateConstantResources();

	// デバイスが削除された際にデバイスリソースの再設定を行う
	void OnDeviceLost();

	// デバイスリソース
	HWND window_;
	int output_width_;
	int output_height_;

	D3D_FEATURE_LEVEL featureLevels_;

	Microsoft::WRL::ComPtr<ID3D11Device> d3d_device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d_context_;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swap_chain_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d_render_target_view_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d_depth_stencil_view_;
	D3D11_VIEWPORT viewport_;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> d3d_vertex_shader_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> d3d_pixel_shader_;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> d3d_vertex_layout_;

	Microsoft::WRL::ComPtr<ID3D11Buffer> d3d_vertex_buffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> d3d_index_buffer_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d_shader_resource_view_;

	Microsoft::WRL::ComPtr<ID3D11Buffer> d3d_cbuffer_;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> d3d_rasterizer_state_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> d3d_depth_stencil_state_;
	Microsoft::WRL::ComPtr<ID3D11BlendState> d3d_blend_state_;

	// フレームレート
	const float FRAME_RATE = 60.0f;
	// 総合カウント数
	LARGE_INTEGER total_count_;
	// カウンターの周波数
	LARGE_INTEGER frequency_;
};

// エラーハンドリング
namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception();
		}
	}
}