cbuffer ConstBufferDataMaterial : register(b0)
{
	float4 color; //�F�iRGBA�j
};

struct VSOutput
{
	// �V�X�e���p���_���W
	float4 svpos : SV_POSITION;
	// uv�l
	float2 uv : TEXCOORD;
};