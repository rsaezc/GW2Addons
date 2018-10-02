#pragma warning(disable : 3571)
#pragma warning(disable : 4717)
#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#include "perlin.hlsl"
#include "rgb2hsl.fxh"

struct VS_SCREEN
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

texture texMountImage;

sampler2D texMountImageSampler =
sampler_state
{
    texture = <texMountImage>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

texture texMountLogo;

sampler2D texMountLogoSampler =
sampler_state
{
	texture = <texMountLogo>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

texture texBackground;

sampler2D texBackgroundSampler =
sampler_state
{
    texture = <texBackground>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

texture texCursor;

sampler2D texCursorSampler =
sampler_state
{
	texture = <texCursor>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

float4 g_vSpriteDimensions;
float g_fFadeInProgress;
float g_fTimer;
float g_fHoverProgress;
bool g_bMountHovered;
uint g_iMountHovered;
float4 g_vColor;

VS_SCREEN Default_VS(in float2 UV : TEXCOORD0)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 dims = (UV * 2 - 1) * g_vSpriteDimensions.zw;

    Out.UV = UV;
    Out.Position = float4(dims + g_vSpriteDimensions.xy * 2 - 1, 0.5f, 1.f);
	Out.Position.y *= -1;

    return Out;
}

float4 Background_PS(VS_SCREEN In) : COLOR0
{
	float4 color = tex2D(texBackgroundSampler, In.UV);
	return color * g_fFadeInProgress;
}

technique Background
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 Default_VS();
		PixelShader = compile ps_3_0 Background_PS();
	}
}

float4 MountImage_PS(VS_SCREEN In) : COLOR0
{
    float4 color = tex2D(texMountImageSampler, In.UV);

	if (!g_bMountHovered)
	{
		color.rgb = dot(color.rgb, float3(0.3, 0.59, 0.11));
	}
	else
	{
		switch (g_iMountHovered)
		{
		case 0:
			if (distance(In.UV, float2(0.5, 1)) > g_fHoverProgress)
			{
				color.rgb = dot(color.rgb, float3(0.3, 0.59, 0.11));
			}
			break;
		case 1:
			if (distance(In.UV, float2(0, 1)) > g_fHoverProgress)
			{
				color.rgb = dot(color.rgb, float3(0.3, 0.59, 0.11));
			}
			break;
		case 2:
			if (distance(In.UV, float2(0, 0)) > g_fHoverProgress)
			{
				color.rgb = dot(color.rgb, float3(0.3, 0.59, 0.11));
			}
			break;
		case 3:
			if (distance(In.UV, float2(0.5, 0)) > g_fHoverProgress)
			{
				color.rgb = dot(color.rgb, float3(0.3, 0.59, 0.11));
			}
			break;
		case 4:
			if (distance(In.UV, float2(1, 0)) > g_fHoverProgress)
			{
				color.rgb = dot(color.rgb, float3(0.3, 0.59, 0.11));
			}
			break;
		case 5:
			if (distance(In.UV, float2(1, 1)) > g_fHoverProgress)
			{
				color.rgb = dot(color.rgb, float3(0.3, 0.59, 0.11));
			}
			break;
		default:
			break;
		}
	}

	return color * g_fFadeInProgress;
}

technique MountImage
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 Default_VS();
		PixelShader = compile ps_3_0 MountImage_PS();
	}
}

float4 MountLogo_PS(VS_SCREEN In) : COLOR0
{
	float mask = 1.f - tex2D(texMountLogoSampler, In.UV).r;
	float shadow = 1.f - tex2D(texMountLogoSampler, In.UV + 0.01f).r;
	
	float luma = dot(g_vColor.rgb, float3(0.2126, 0.7152, 0.0722));

	float3 faded_color = lerp(g_vColor.rgb, luma, 0.33f);
	float3 color = lerp(faded_color, g_vColor.rgb, g_fHoverProgress);
	float glow_mask = 0;
	glow_mask += 1.f - tex2D(texMountLogoSampler, In.UV + float2(0.01f, 0.01f)).r;
	glow_mask += 1.f - tex2D(texMountLogoSampler, In.UV + float2(-0.01f, 0.01f)).r;
	glow_mask += 1.f - tex2D(texMountLogoSampler, In.UV + float2(0.01f, -0.01f)).r;
	glow_mask += 1.f - tex2D(texMountLogoSampler, In.UV + float2(-0.01f, -0.01f)).r;
	float3 glow = g_vColor.rgb * (glow_mask / 4) * g_fHoverProgress * 0.5f * (0.5f + 0.5f * snoise(In.UV * 3.18f + 0.15f * float2(cos(g_fTimer * 3), sin(g_fTimer * 2))));
	return float4(color * mask + glow, g_vColor.a * max(mask, shadow)) * g_fFadeInProgress;
}

technique MountLogo
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 Default_VS();
		PixelShader = compile ps_3_0 MountLogo_PS();
	}
}

float4 Cursor_PS(VS_SCREEN In) : COLOR0
{
	float4 color = tex2D(texCursorSampler, In.UV);
	if (g_bMountHovered)
	{
		float3 hsl_color = RGBtoHSL(color.rgb);
		hsl_color.x += 0.17f; /* Convert yellow tones to green adding 60º (60/360=0.17) */
		color.rgb = HSLtoRGB(hsl_color);
	}

	return color * g_fFadeInProgress;
}

technique Cursor
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 Default_VS();
		PixelShader = compile ps_3_0 Cursor_PS();
	}
}