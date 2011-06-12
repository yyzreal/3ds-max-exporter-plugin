#include "stdafx.h"
#include "DrawNormal.h"

DrawNormal::DrawNormal( UINT length )
{
	m_length = length;
	m_pVertexBuffer = NULL;
	m_bInit = FALSE;
	m_pTechnique = NULL;
	m_pInputLayout = NULL;
}

DrawNormal::~DrawNormal()
{
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pInputLayout );
}

void DrawNormal::AddNormal( const D3DXVECTOR3 &pos, const D3DXVECTOR3 &normal )
{
	D3DXVECTOR3 norVec;
	D3DXVec3Normalize( &norVec, &normal );
	norVec = D3DXVECTOR3( norVec.x * m_length, norVec.y * m_length, norVec.z * m_length );

	D3DXVECTOR3 end = pos + norVec;

	Line l( pos, end );
	m_vNormalLines.push_back( l );
}

void DrawNormal::Initialize( ID3D10Device *pD3DDevice, ID3D10Effect *pEffect )
{
	assert( pD3DDevice != NULL && pEffect != NULL );

	InitVertexBuffer( pD3DDevice );

	InitShader( pD3DDevice, pEffect );

	m_bInit = TRUE;
}

void DrawNormal::InitVertexBuffer( ID3D10Device *pD3DDevice )
{
	// 一条线对应两个顶点
	UINT numVertices = static_cast< UINT >( m_vNormalLines.size() * 2 );

	// 缓冲区描述信息
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( D3DXVECTOR3 ) * numVertices;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3DXVECTOR3 *pV = new D3DXVECTOR3[ numVertices ];
	int n = 0;
	for ( vector< Line >::iterator i = m_vNormalLines.begin(); i != m_vNormalLines.end(); i ++ )
	{
		Line &l = *i;
		pV[n++] = l.start;
		pV[n++] = l.end;
	}
	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem = pV;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	// 创建顶点缓冲区
	if ( FAILED( pD3DDevice->CreateBuffer( &bd, &initData, &m_pVertexBuffer ) ) )
	{
		throw runtime_error( "Create Vertex Buffer Error" );
	}

	delete[] pV;
}

void DrawNormal::InitShader( ID3D10Device *pD3DDevice, ID3D10Effect *pEffect )
{
	assert( pD3DDevice != NULL && pEffect != NULL );

	m_pTechnique = pEffect->GetTechniqueByName( "Normal" );

	assert( m_pTechnique != NULL );

	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
	};

	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	//create input layout
	D3D10_PASS_DESC PassDesc;
	m_pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if ( FAILED( pD3DDevice->CreateInputLayout( layout, 
		numElements, 
		PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, 
		&m_pInputLayout ) ) ) 
	{
		throw runtime_error( "Could not create Input Layout!" );
	}
}

void DrawNormal::Draw( ID3D10Device *pD3DDevice )
{
	if ( m_bInit == FALSE )
	{
		assert( NULL &&
			"没有初始化DrawNormal" );
	}

	pD3DDevice->IASetInputLayout( m_pInputLayout );
	//get technique desc
	D3D10_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc( &techDesc );

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		// 设置顶点缓冲区
		UINT stride = sizeof( D3DXVECTOR3 );
		UINT offset = 0;
		pD3DDevice->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
		pD3DDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_LINELIST );

		//apply technique
		m_pTechnique->GetPassByIndex( p )->Apply( 0 );
			// 绘制
		pD3DDevice->Draw( static_cast< UINT >( m_vNormalLines.size() * 2 ), 0 );
	}
}