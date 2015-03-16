Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s0);

cbuffer constants : register(b0)
{
    float offset	: packoffset(c0.x);
	float2 bounds	: packoffset(c0.y);	
};

float3 Hue(float H)
{
    float R = abs(H * 6 - 3) - 1;
    float G = 2 - abs(H * 6 - 2);
    float B = 2 - abs(H * 6 - 4);
    return saturate(float3(R,G,B));
}

float4 main(
    float4 clipSpaceOutput  : SV_POSITION,
    float4 sceneSpaceOutput : SCENE_POSITION,
    float4 texelSpaceInput0 : TEXCOORD0
    ) : SV_Target
{
    float2 sampleLocation = (texelSpaceInput0.xy / (bounds * texelSpaceInput0.zw));
        
    float4 color = InputTexture.Sample(InputSampler, texelSpaceInput0);
	bool magenta = color.r == 1.0 && color.g == 0.0 && color.b == 1.0; 
	if(magenta)
	{
		sampleLocation.y = frac(sampleLocation.y + offset);
		color = float4(Hue(sampleLocation.y), 1.0);
	}
    return color;
}