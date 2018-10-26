// Pixel Shader

cbuffer ConstBuff : register(b0)
{
	matrix mtxProj;
	matrix mtxView;
	matrix mtxWorld;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_Target
{
	return float4(1.0,1.0,1.0,1.0);
}