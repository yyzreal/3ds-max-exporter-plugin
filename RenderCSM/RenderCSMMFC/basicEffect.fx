//--------------------------------------------------------------------------------------
// basicEffect.fx
//
//--------------------------------------------------------------------------------------

Texture2D tex2D;
SamplerState linearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

RasterizerState Cull
{		
	CullMode = Front;
	FrontCounterClockwise = true;
	DepthClipEnable = true;	
};

DepthStencilState EnableDepth
{
	DepthEnable = true;
	DepthWriteMask = All;
	DepthFunc = Less;
};

matrix World;
matrix View;
matrix Projection;

struct VS_INPUT
{
	float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};


struct NormalVS_Input
{
	float4 Pos : POSITION;
};

struct NormalPS_Input
{
	float4 Pos : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
	PS_INPUT output;
	
	output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );    
    output.Pos = mul( output.Pos, Projection );
	output.Color = float4( 1.0f, 0.0f, 0.0f, 1.0f );
	output.Tex = input.Tex;
	
    return output;  
}

NormalPS_Input NormalVS( NormalVS_Input input )
{
	NormalPS_Input output;
	
	output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );    
    output.Pos = mul( output.Pos, Projection );
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 textured( PS_INPUT input ) : SV_Target
{
    return tex2D.Sample( linearSampler, input.Tex ); 
}

float4 noTexture( PS_INPUT input ) : SV_Target
{
    return input.Color; 
}

float4 NormalPS( NormalPS_Input input ) : SV_Target
{
	return float4( 1.0f, 1.0f, 1.0f ,1.0f );
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 full
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, textured() ) );
        SetDepthStencilState( EnableDepth, 1 );
    }
}

technique10 texturingDisabled
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, noTexture() ) );
        SetDepthStencilState( EnableDepth, 1 );
    }
}

technique10 Normal
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, NormalVS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, NormalPS() ) );
		SetDepthStencilState( EnableDepth, 1 );
	}
}