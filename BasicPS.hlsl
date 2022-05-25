cbuffer ConstBufferDataMaterial : register(b0)
{
	float4 color; //êFÅiRGBAÅj
};

float4 main() : SV_TARGET
{
	return color;
}