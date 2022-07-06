#include "Basic.hlsli"



Texture2D<float4> tex: register(t0);
SamplerState smp : register(s0);

float4 main(VSOutput input) : SV_TARGET
{
	float4 color = { 1,0,0,1 };
	return float4(tex.Sample(smp,input.uv) * color);
}
