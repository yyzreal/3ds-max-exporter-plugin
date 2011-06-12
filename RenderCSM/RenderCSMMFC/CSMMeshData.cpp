#include "stdafx.h"
#include "CSMMeshData.h"

CSMMeshData::CSMMeshData()
{
	m_pHeader = NULL;
	m_pDrawNormal = NULL;
}

CSMMeshData::~CSMMeshData()
{
	SAFE_DELETE( m_pHeader );
	for ( SubMeshList::iterator i = m_vpSubMeshes.begin(); i != m_vpSubMeshes.end(); i ++ )
	{
		SAFE_DELETE( *i );
	}
	SAFE_DELETE( m_pDrawNormal );
}

/*! \param fileName CSM文件名
 *! \return 读取失败返回FALSE
 */
BOOL CSMMeshData::LoadCSM( const string &fileName )
{
	if ( BeginReading( m_inFileCSM, fileName ) )
	{
		//! 读Header
		if ( NULL == m_pHeader )
		{
			m_pHeader = new CSMHeader();
		}
		m_inFileCSM.read( reinterpret_cast< char* >( m_pHeader ), sizeof( CSMHeader ) );

		//! 读取TAG
		int offset = m_pHeader->nHeaderSize;
		for ( int i = 0; i < m_pHeader->numTags; i ++ )
		{
			CSMTagData tag;

			m_inFileCSM.seekg( offset );
			m_inFileCSM.read( reinterpret_cast< char* >( tag.name ), MAX_STRING_LENGTH );
			offset += MAX_STRING_LENGTH;

			m_inFileCSM.seekg( offset );
			m_inFileCSM.read( reinterpret_cast< char* >( &tag.numAnimFrames ), sizeof( int ) );
			offset += sizeof( int );

			for ( int j = 0; j < tag.numAnimFrames; j ++ )
			{
				D3DXMATRIX mtx;;
				m_inFileCSM.seekg( offset );
				m_inFileCSM.read( reinterpret_cast< char* >( &mtx ), sizeof( MATRIX ) );
				offset += sizeof( MATRIX );
				tag.vFrameData.push_back( mtx );
			}

			m_vpTags.push_back( tag );
		}

		//! 读骨骼名称，初始化骨骼列表
		vector< string > boneNames;
		for ( int i = 0; i < m_pHeader->numBones; i ++ )
		{
			CSMBoneData boneData;
			m_inFileCSM.seekg( m_pHeader->nOffBones + i * sizeof( CSMBoneData ) );
			m_inFileCSM.read( reinterpret_cast< char* >( &boneData ), sizeof( CSMBoneData ) );
			
			m_boneList.push_back( boneData );
		}

		//! 读Sub Mesh
		int lastOffset = m_pHeader->nOffSubMesh;
		for ( int i = 0; i < m_pHeader->numSubMesh; i ++ )
		{
			CSMSubMesh *pSubMesh = new CSMSubMesh();

			//! 读Sub Mesh的Header
			m_inFileCSM.seekg( lastOffset, ifstream::beg );
			m_inFileCSM.read( reinterpret_cast<char*>( &( pSubMesh->subMeshHeader ) ), sizeof( CSMSubMeshHeader ) );

			//! 读Sub Mesh的纹理
			m_inFileCSM.seekg( lastOffset + pSubMesh->subMeshHeader.nHeaderSize, ifstream::beg );
			m_inFileCSM.read( pSubMesh->textureFile, MAX_STRING_LENGTH );

			//! 读顶点
			m_inFileCSM.seekg( lastOffset + pSubMesh->subMeshHeader.nOffVertices, ifstream::beg );
			CSMVertexData *vertices = new CSMVertexData[ pSubMesh->subMeshHeader.numVertices ];
			m_inFileCSM.read( reinterpret_cast< char* >( vertices ), sizeof( CSMVertexData ) * pSubMesh->subMeshHeader.numVertices );
			for ( int j = 0; j < pSubMesh->subMeshHeader.numVertices; j ++ )
			{
				//vertices[j].position[0] /= 100;
				//vertices[j].position[1] /= 100;
				//vertices[j].position[1] /= 100;
				pSubMesh->vVertexData.push_back( vertices[j] );
			}

			//! 读Skin Data
			int numSkinData;
			if ( pSubMesh->subMeshHeader.numAnimFrames > 1 )
			{
				numSkinData = pSubMesh->subMeshHeader.numVertices;

				m_inFileCSM.seekg( lastOffset + pSubMesh->subMeshHeader.nOffSkin, ifstream::beg );
				CSMSkinData *skinData = new CSMSkinData[ numSkinData ];
				m_inFileCSM.read( reinterpret_cast< char* >( skinData ), sizeof( CSMSkinData ) * numSkinData );
				for ( int j = 0; j < numSkinData; j ++ )
				{
					pSubMesh->vSkinData.push_back( skinData[j] );
				}
			}

			//! 读三角形面
			m_inFileCSM.seekg( lastOffset + pSubMesh->subMeshHeader.nOffFaces, ifstream::beg );
			CSMTriangleData *triangles = new CSMTriangleData[ pSubMesh->subMeshHeader.numFaces ];
			m_inFileCSM.read( reinterpret_cast< char* >( triangles ), sizeof( CSMTriangleData ) * pSubMesh->subMeshHeader.numFaces );
			for ( int j = 0; j < pSubMesh->subMeshHeader.numFaces; j ++ )
			{
				pSubMesh->vTriangleData.push_back( triangles[j] );
			}

			m_vpSubMeshes.push_back( pSubMesh );

			//! 更新新的起始位置
			lastOffset = lastOffset + pSubMesh->subMeshHeader.nOffEnd;
		}

		EndReading( m_inFileCSM );

		// 初始化Normal绘制器
		UINT normalLen = 2;
		m_pDrawNormal = new DrawNormal( normalLen );
		for ( SubMeshList::iterator i = m_vpSubMeshes.begin(); i != m_vpSubMeshes.end(); i ++ )
		{
			CSMSubMesh *pSubMesh = *i;
			for ( vector< CSMVertexData >::iterator j = pSubMesh->vVertexData.begin(); j != pSubMesh->vVertexData.end(); j ++ )
			{
				CSMVertexData &vtx = *j;
				m_pDrawNormal->AddNormal( D3DXVECTOR3( vtx.position[0], vtx.position[1], vtx.position[2] ),
										  D3DXVECTOR3( vtx.normal[0], vtx.normal[1], vtx.normal[2] ) );
			}
		}

		return TRUE;
	}

	return FALSE;
}