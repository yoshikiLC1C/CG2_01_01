cbuffer ConstBufferDataMaterial : register(b0)
{
	float4 color; //�F�iRGBA�j
};

float4 main() : SV_TARGET
{
	return color;
}