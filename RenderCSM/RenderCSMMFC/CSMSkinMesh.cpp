#include "stdafx.h"
#include "CSMSkinMesh.h"

static const float EPISILON = 0.0001f;

BOOL IsFloatEqual( const float lf, const float rf )
{
	if ( fabs( lf - rf ) < EPISILON )
	{
		return TRUE;
	}
	return FALSE;
}

/*! \brief D3DXMATRIX提供的“==”函数，误差太精细，故这里重写一个函数 */
BOOL IsMatrixEqual( const D3DXMATRIX lmat, const D3DXMATRIX rmat )
{
	if ( IsFloatEqual( lmat._11, rmat._11 ) && 
		 IsFloatEqual( lmat._12, rmat._12 ) &&
		 IsFloatEqual( lmat._13, rmat._13 ) &&
		 IsFloatEqual( lmat._14, rmat._14 ) &&
		 IsFloatEqual( lmat._21, rmat._21 ) && 
		 IsFloatEqual( lmat._22, rmat._22 ) &&
		 IsFloatEqual( lmat._23, rmat._23 ) &&
		 IsFloatEqual( lmat._24, rmat._24 ) &&
		 IsFloatEqual( lmat._31, rmat._31 ) &&
		 IsFloatEqual( lmat._32, rmat._32 ) &&
		 IsFloatEqual( lmat._33, rmat._33 ) &&
		 IsFloatEqual( lmat._34, rmat._34 ) &&
		 IsFloatEqual( lmat._41, rmat._41 ) &&
		 IsFloatEqual( lmat._42, rmat._42 ) &&
		 IsFloatEqual( lmat._43, rmat._43 ) &&
		 IsFloatEqual( lmat._44, rmat._44 ) )
	{
		return TRUE;
	}
	return FALSE;
}

const string CSMSkinMesh::TECHNIQUE_NAME = "full";

CSMSkinMesh::CSMSkinMesh()
:	m_pBasicTechnique( NULL ),
	m_pVertexLayout( NULL ),
	m_pMeshData( NULL ),
	m_pAnimationResourceView( NULL ),
	m_pAnimationTexture( NULL )
{
	D3DXMatrixIdentity( &m_worldMatrix );
}

CSMSkinMesh::~CSMSkinMesh()
{
	SAFE_DELETE( m_pMeshData );
	for ( vector< ID3D10Buffer* >::iterator i = m_vpVertexBuffers.begin(); i != m_vpVertexBuffers.end(); i ++ )
	{
		SAFE_RELEASE( *i );
	}
	for ( vector< ID3D10ShaderResourceView* >::iterator i = m_vpTextureSRV.begin(); i != m_vpTextureSRV.end(); i ++ )
	{
		SAFE_RELEASE( *i );
	}

	SAFE_RELEASE( m_pAnimationTexture );
	SAFE_RELEASE( m_pAnimationResourceView );
}

/*! \param pd3dDevice D3D10设备
	\param CSMFile CSM文件名称
	\param pEffect Effect效果文件
	\return 初始化成功或失败
 */
BOOL CSMSkinMesh::Initialize( ID3D10Device *pd3dDevice, const string &CSMFile, ID3D10Effect *pEffect )
{
	assert( pd3dDevice != NULL && pEffect != NULL );

	m_pBasicTechnique = pEffect->GetTechniqueByName( CSMSkinMesh::TECHNIQUE_NAME.c_str() );
	if ( m_pBasicTechnique == NULL )
	{
		return FALSE;
	}

	if ( NULL == m_pMeshData )
	{
		m_pMeshData = new CSMMeshData();
	}

	size_t token = CSMFile.find_last_of( "\\" );
	if ( token != string::npos )
	{
		m_filePath = CSMFile.substr( 0, token );
	}
	else
	{
		m_filePath = "";
	}
	if ( FAILED( m_pMeshData->LoadCSM( CSMFile ) ) )
	{
		return FALSE;
	}
	m_pMeshData->GetDrawNormal()->Initialize( pd3dDevice, pEffect );

	if ( FAILED( InitVertexBuffer( pd3dDevice ) ) )
	{
		return FALSE;
	}

	if ( FALSE == InitIndexBuffer( pd3dDevice ) )
	{
		return FALSE;
	}

	if ( FAILED( InitInputLayout( pd3dDevice, m_pBasicTechnique ) ) )
	{
		return FALSE;
	}

	if ( FAILED( LoadTextures( pd3dDevice ) ) )
	{
		return FALSE;
	}

	if ( TRUE == m_pMeshData->HasAnimation() )
	{
		size_t token = CSMFile.find_last_of( ".CSM" );
		string animFile = CSMFile.substr( 0, token - 3 ) + ".AM";

		token = CSMFile.find_last_of( ".CSM" );
		string cfgFile = CSMFile.substr( 0, token - 3 ) + ".CFG";

		if ( FALSE == InitAnim( animFile, cfgFile ) )
		{
			return FALSE;
		}

		CreateAnimationTexture( pd3dDevice, &m_pAnimationTexture, &m_pAnimationResourceView );

		SetAnimation( 0 );
	}

	CalcEntireBox();

	OutputInfoToTXT();

	return TRUE;
}

/*! \param pd3dDevice D3D10设备
	\return 初始化成功或失败
 */
BOOL CSMSkinMesh::InitVertexBuffer( ID3D10Device *pd3dDevice )
{
	assert( pd3dDevice != NULL && m_pMeshData != NULL );

	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		UINT numVertices = m_pMeshData->GetMeshNumVtx( i );

		// 缓冲区描述信息
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof( DXCSMVertex ) * numVertices;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;

		// 创建顶点缓冲区
		m_vpVertexBuffers.push_back( NULL );
		if ( FAILED( pd3dDevice->CreateBuffer( &bd, NULL, &m_vpVertexBuffers[i] ) ) )
		{
			throw runtime_error( "Create Vertex Buffer Error" );
			return FALSE;
		}

		DXCSMVertex *v = NULL;
		// 初始化顶点缓冲区数据
		m_vpVertexBuffers[i]->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast< void** >( &v ) );

		int n = 0;
		for ( int j = 0; j < m_pMeshData->GetMeshNumVtx( i ); j ++ )
		{
			CSMVertexData vtx = m_pMeshData->GetMeshVtx( i, j );
			v[n++] = DXCSMVertex( D3DXVECTOR3( vtx.position[0], vtx.position[1], vtx.position[2] ), D3DXVECTOR2( vtx.u, vtx.v ) );
		}

		m_vpVertexBuffers[i]->Unmap();

	}

	return TRUE;
}

BOOL CSMSkinMesh::InitIndexBuffer( ID3D10Device *pd3dDevice )
{
	assert( pd3dDevice && m_pMeshData );

	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		UINT numIndices = m_pMeshData->GetMeshNumTri( i ) * 3;

		D3D10_BUFFER_DESC bufDesc;
		ZeroMemory( &bufDesc, sizeof( bufDesc ) );
		bufDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		bufDesc.ByteWidth = numIndices * sizeof( unsigned int );
		bufDesc.Usage = D3D10_USAGE_DEFAULT;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.MiscFlags = 0;

		unsigned int *indices = new unsigned int[ numIndices ];
		unsigned int *ptrInd = indices;
		for ( int j = 0; j < m_pMeshData->GetMeshNumTri( i ); j ++ )
		{
			CSMTriangleData tri = m_pMeshData->GetMeshTri( i, j ); 
			for ( int k = 0; k < 3; k ++ )
			{
				*ptrInd = tri.vertexIndices[k];
				ptrInd ++;
			}
		}
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = indices;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		// 创建索引缓冲区
		m_vpIndexBuffers.push_back( NULL );
		if ( FAILED( pd3dDevice->CreateBuffer( &bufDesc, &InitData, &m_vpIndexBuffers[i] ) ) )
		{
			throw runtime_error( "Create Index Buffer Error" );
			return FALSE;
		}
	}

	return TRUE;
}

/*! \param pd3dDevice D3D10设备
	\return 初始化成功或失败
 */
BOOL CSMSkinMesh::LoadTextures( ID3D10Device *pd3dDevice )
{
	assert( pd3dDevice != NULL && m_pMeshData != NULL );

	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		string texture = m_pMeshData->GetMeshTexture( i );
		m_vpTextureSRV.push_back( NULL );

		if ( texture == "" )
		{
			// 没有纹理
			texture = "default.bmp";
		}

		string texFile;
		// 加上路径
		if ( m_filePath != "" )
		{
			texFile = m_filePath + "\\" + texture;
		}
		else
		{
			texFile = texture;
		}
		if ( FAILED( D3DX10CreateShaderResourceViewFromFile( pd3dDevice, texFile.c_str(), NULL, NULL, &m_vpTextureSRV[i], NULL ) ) )
		{
			// 创建纹理失败（D3D10不支持tga纹理）
			D3DX10CreateShaderResourceViewFromFile( pd3dDevice, "default.bmp", NULL, NULL, &m_vpTextureSRV[i], NULL );
		}
	}

	return TRUE;
}

/*! \param pd3dDevice D3D10设备
	\param pTechnique Technique指针
	\return 初始化成功或失败
 */
BOOL CSMSkinMesh::InitInputLayout(ID3D10Device *pd3dDevice, ID3D10EffectTechnique *pTechnique )
{
	assert( pd3dDevice != NULL && pTechnique != NULL );

	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 }, 
	};

	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	//create input layout
	D3D10_PASS_DESC PassDesc;
	pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if ( FAILED( pd3dDevice->CreateInputLayout( layout, 
		numElements, 
		PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, 
		&m_pVertexLayout ) ) ) 
	{
		throw runtime_error( "Could not create Input Layout!" );
		return FALSE;
	}

	return TRUE;
}

/*! \param pd3dDevice D3D10设备
	\param time 程序运行时间
	\param worldMatVar 设置世界矩阵的接口
	\param texVar 设置纹理的接口
 */
void CSMSkinMesh::Render( ID3D10Device *pd3dDevice, const float time, 
						  ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar )
{
	assert( pd3dDevice != NULL && worldMatVar != NULL && texVar != NULL && m_pMeshData != NULL );

	if ( m_pMeshData->HasAnimation() )
	{
		// 更新动画
		Animate( time );
	}

	RenderSingleFrame( pd3dDevice, worldMatVar, texVar );
}

/*! \param pd3dDevice D3D10设备
	\param worldMatVar 设置世界矩阵的接口
	\param texVar 设置纹理的接口
*/
void CSMSkinMesh::RenderSingleFrame( ID3D10Device *pd3dDevice, ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar )
{
	worldMatVar->SetMatrix( m_worldMatrix );

	pd3dDevice->IASetInputLayout( m_pVertexLayout );
	//get technique desc
	D3D10_TECHNIQUE_DESC techDesc;
	m_pBasicTechnique->GetDesc( &techDesc );

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
		{
			// 设置顶点缓冲区
			UINT stride = sizeof( DXCSMVertex );
			UINT offset = 0;
			pd3dDevice->IASetVertexBuffers( 0, 1, &m_vpVertexBuffers[i], &stride, &offset );
			pd3dDevice->IASetIndexBuffer( m_vpIndexBuffers[i], DXGI_FORMAT_R32_UINT, 0 );
			pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

			// 设置纹理
			if ( m_vpTextureSRV[i] != NULL )
			{
				texVar->SetResource( m_vpTextureSRV[i] );
			}

			//apply technique
			m_pBasicTechnique->GetPassByIndex( p )->Apply( 0 );
			// 绘制
			pd3dDevice->DrawIndexed( 3 * m_pMeshData->GetMeshNumTri( i ), 0, 0 );
		}
	}
}

void CSMSkinMesh::RenderNormal( ID3D10Device *pd3dDevice )
{
	// 渲染法线
	m_pMeshData->GetDrawNormal()->Draw( pd3dDevice );
}

void CSMSkinMesh::UpdateFrame()
{
	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		if ( m_pMeshData->GetMeshNumAnim( i ) == 0 )
		{
			// subMesh不含动画
			continue;
		}
		int numVertices = m_pMeshData->GetMeshNumVtx( i );
		D3DXVECTOR3 *skinedVertices = new D3DXVECTOR3[ numVertices ];

		// 为顶点蒙皮
		for ( int j = 0; j < numVertices; j ++ )
		{
			CSMVertexData vtx = m_pMeshData->GetMeshVtx( i, j );
			CSMSkinData skinData = m_pMeshData->GetMeshSkin( i, j );
			D3DXVECTOR3 oriPos = D3DXVECTOR3( vtx.position[0], vtx.position[1], vtx.position[2] );

			D3DXMATRIX finalMat;
			ZeroMemory( finalMat, sizeof( D3DXMATRIX ) );

			for ( int k = 0; k < 4; k ++ )
			{
				if ( skinData.bones[k] != -1 )
				{
					int frameIndex = skinData.bones[k] * m_numSceneFrames + m_curAnimState.curr_frame;
					D3DXMATRIX skinMat = m_frameList[frameIndex];
					finalMat = finalMat + skinMat * skinData.boneWeights[k];
				}
				else
				{
					if ( k == 0 )
					{
						D3DXMatrixIdentity( &finalMat );
					}
					break;
				}
			}
			finalMat._44 = 1;

			D3DXVec3TransformCoord( &skinedVertices[j], &oriPos, &finalMat );
		}

		DXCSMVertex *v = NULL;
		// 更新顶点缓冲区
		m_vpVertexBuffers[i]->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast< void** >( &v ) );

		int n = 0;
		for ( int j = 0; j < numVertices; j ++ )
		{
			CSMVertexData oriVtx = m_pMeshData->GetMeshVtx( i, j ); // 原顶点数据
			D3DXVECTOR3 skinnedVtx = skinedVertices[j];  // 蒙皮后的顶点位置

			v[n++] = DXCSMVertex( skinnedVtx, D3DXVECTOR2( oriVtx.u, oriVtx.v ) );
		}

		m_vpVertexBuffers[i]->Unmap();

		delete[] skinedVertices;
	}
}

void CSMSkinMesh::CalcEntireBox()
{
	D3DXVECTOR3 min, max;
	ZeroMemory( min, sizeof( D3DXVECTOR3 ) );
	ZeroMemory( max, sizeof( D3DXVECTOR3 ) );
	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		for ( int j = 0; j < m_pMeshData->GetMeshNumVtx( i ); j ++ )
		{
			CSMVertexData vtx = m_pMeshData->GetMeshVtx( i, j );

			if ( vtx.position[0] < min.x )
			{
				min.x = vtx.position[0];
			}
			if ( vtx.position[1] < min.y )
			{
				min.y = vtx.position[1];
			}
			if ( vtx.position[2] < min.z )
			{
				min.z = vtx.position[2];
			}
			if ( vtx.position[0] > max.x )
			{
				max.x = vtx.position[0];
			}
			if ( vtx.position[1] > max.y )
			{
				max.y = vtx.position[1];
			}
			if ( vtx.position[2] > max.z )
			{
				max.z = vtx.position[2];
			}
		}
	}

	m_entireBox.boxMin = min;
	m_entireBox.boxMax = max;

	CalcEntireBoxInfo();
}


int CSMSkinMesh::GetTotalNumVtx() const
{
	int n = 0;
	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		n += m_pMeshData->GetMeshNumVtx( i );
	}
	return n;
}

int CSMSkinMesh::GetTotalNumUV() const
{
	int n = 0;
	return n;
}

int CSMSkinMesh::GetTotalNumTri() const
{
	int n = 0;
	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		n += m_pMeshData->GetMeshNumTri( i );
	}
	return n;
}

int CSMSkinMesh::CreateAnimationTexture( ID3D10Device *pd3dDevice, ID3D10Texture2D **ppAnimationTexture, ID3D10ShaderResourceView **ppAnimationResourceView )
{
	if( FALSE == m_pMeshData->HasAnimation() )
	{
		return FALSE;
	}

	UINT texelsPerBone = 4;
	UINT maxBones = m_numTotalFrames;
	UINT pixelCount = maxBones * texelsPerBone;
	UINT texWidth = 0;
	UINT texHeight = 0;

	// This basically fits the animation into a roughly square texture where the 
	//      width is a multiple of rowsPerBone(our size requirement for matrix storage)
	//      AND both dimensions are power of 2 since it seems to fail without this...
	texWidth = ( int )sqrt( ( float ) pixelCount ) + 1;    // gives us a starting point
	texHeight = 1;
	while(texHeight < texWidth) 
		texHeight = texHeight << 1;
	texWidth = texHeight;

	HRESULT hr = S_OK;
	D3D10_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof( D3D10_TEXTURE2D_DESC ) );

	// Create our texture
	desc.MipLevels = 1;
	desc.Usage = D3D10_USAGE_IMMUTABLE;
	desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.ArraySize = 1;
	desc.Width = texWidth;
	desc.Height = texHeight;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	// Make a offline buffer that is the full size of the texture
	UINT bufferSize = texHeight * texWidth * sizeof( D3DXVECTOR4 );
	D3DXVECTOR4 *pData = new D3DXVECTOR4[ desc.Width * desc.Height ];
	memset( ( void* )pData, 0, bufferSize );    // clear it

	D3DXVECTOR4 * pCurrentDataPtr = pData;
	for ( BoneFrames::iterator i = m_frameList.begin(); i != m_frameList.end(); i ++ )
	{
		D3DXMATRIX frame = *i;
		D3DXVECTOR4 *pFrameMatrixVectors = ( D3DXVECTOR4 * )( &frame );
		memcpy( ( void * )pCurrentDataPtr ++, ( void * )( pFrameMatrixVectors[0] ), sizeof( D3DXVECTOR4 ) );
		memcpy( ( void * )pCurrentDataPtr ++, ( void * )( pFrameMatrixVectors[1] ), sizeof( D3DXVECTOR4 ) );
		memcpy( ( void * )pCurrentDataPtr ++, ( void * )( pFrameMatrixVectors[2] ), sizeof( D3DXVECTOR4 ) );
		memcpy( ( void * )pCurrentDataPtr ++, ( void * )( pFrameMatrixVectors[3] ), sizeof( D3DXVECTOR4 ) );
	}

	D3D10_SUBRESOURCE_DATA srd;
	srd.pSysMem = ( void* )pData;
	srd.SysMemPitch = texWidth * ( sizeof( D3DXVECTOR4 ) );
	srd.SysMemSlicePitch = 1;
	if ( FAILED( pd3dDevice->CreateTexture2D( &desc, &srd, ppAnimationTexture) ) )
	{
		delete[] pData;
		return FALSE;
	}

	delete[] pData;

	// Make a resource view for it
	D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = desc.MipLevels;
	if ( FAILED( pd3dDevice->CreateShaderResourceView( *ppAnimationTexture, &SRVDesc, ppAnimationResourceView ) ) )
	{
		return FALSE;
	}

	return TRUE;
}

void CSMSkinMesh::OutputInfoToTXT()
{
	string fileName = m_filePath + "/Model_Output.txt";
	ofstream outFile( fileName.c_str(), ofstream::out );

	outFile << "标记的数量: " << m_pMeshData->GetNumTags() << endl;
	outFile << "动画帧数量: " << m_pMeshData->GetNumFrames() << endl;
	outFile << "骨骼的数量: " << m_pMeshData->GetNumBones() << endl;
	outFile << "子网格的数量: " << m_pMeshData->GetNumSubMeshes() << endl;
	outFile << endl;

	outFile << "Bones" << endl;
	for ( int i = 0; i < m_pMeshData->GetNumBones(); i ++ )
	{
		outFile << "\t" << i+1 << ": " << m_pMeshData->GetBone( i ).name << endl;
	}

	outFile << endl;

	outFile << "Submeshes" << endl;
	for ( int i = 0; i < m_pMeshData->GetNumSubMeshes(); i ++ )
	{
		outFile << "\t" << i+1 << ": " << m_pMeshData->GetMeshName( i ) << endl;
		outFile << "\t\t顶点数: " << m_pMeshData->GetMeshNumVtx( i ) << endl;
		outFile << "\t\tSkin数据数: " << m_pMeshData->GetMeshNumSkin( i ) << endl;
		outFile << "\t\t面数: " << m_pMeshData->GetMeshNumTri( i ) << endl;

		outFile << "\t\t这些是蒙皮数据: " << endl;
		for ( int j = 0; j < m_pMeshData->GetMeshNumSkin( i ); j ++ )
		{
			CSMSkinData pSkinData = m_pMeshData->GetMeshSkin( i, j );
			outFile << "\t\t" << j << ": ";
			for ( int k = 0; k < 4; k++ )
			{
				if ( pSkinData.bones[k] != -1 )
				{
					outFile << "(" << m_pMeshData->GetBone( pSkinData.bones[k] ).name << ", " << pSkinData.boneWeights[k] << ") ";
				}
				else
				{
					break;
				}
			}
			outFile << endl;
		}
	}

	outFile.close();

	fileName = m_filePath + "/Animation_Output.txt";
	outFile.open( fileName.c_str(), ofstream::out );

	for ( int i = 0; i < m_pMeshData->GetNumBones(); i ++ )
	{
		outFile << i+1 << ": " << m_pMeshData->GetBone( i ).name << endl;

		for ( int j = 0; j < m_numSceneFrames; j ++ )
		{
			int offset = i * m_numSceneFrames + j;
			D3DXMATRIX m = m_frameList[j];
			outFile << "\tFrame" << j+1 << ": " << endl;
			outFile << "\t\t" << m._11 << "  " << m._12 << "  " << m._13 << "  " << m._14 << endl;
			outFile << "\t\t" << m._21 << "  " << m._22 << "  " << m._23 << "  " << m._24 << endl;
			outFile << "\t\t" << m._31 << "  " << m._32 << "  " << m._33 << "  " << m._34 << endl;
			outFile << "\t\t" << m._41 << "  " << m._42 << "  " << m._43 << "  " << m._44 << endl;
		}
	}
}