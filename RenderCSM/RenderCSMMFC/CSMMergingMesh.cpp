#include "stdafx.h"
#include "CSMMergingMesh.h"

CSMMergingMesh::CSMMergingMesh()
{
}

CSMMergingMesh::~CSMMergingMesh()
{
	for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		SAFE_DELETE( i->second.pMesh );
	}
}

/*! \param pd3dDevice D3D10设备
	\param pEffect Effect效果文件
	\return 初始化成功或失败
*/
BOOL CSMMergingMesh::Initialize( ID3D10Device *pd3dDevice, const string &pacName, ID3D10Effect *pEffect )
{
	//! 读入模型列表
	ifstream inFile( pacName.c_str() );
	if ( !inFile.fail() )
	{
		for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
		{
			SAFE_DELETE( i->second.pMesh );
		}

		size_t token = pacName.find_last_of( '\\' );
		string path = pacName.substr( 0, token );

		int n;
		inFile >> n;
		for ( int i = 0; i < n; i ++ )
		{
			string meshType, meshName;
			inFile >> meshType >> meshName;

			meshName = path + "\\" + meshName;

			CSMSkinMesh *pMesh = new CSMSkinMesh();
			if ( FALSE == pMesh->Initialize( pd3dDevice, meshName, pEffect ) )
			{
				return FALSE;
			}

			MeshNode node;
			node.pMesh = pMesh;
			m_vpMeshList.insert( SkinMeshList::value_type( meshType, node ) );
		}
	}
	else
	{
		return FALSE;
	}

	//! 建立模型间的绑定链接
	for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		MeshNode *pMeshNode = &( i->second );

		int tagNum = pMeshNode->pMesh->GetNumTagType();
		for ( int j = 0; j < tagNum; j ++ )
		{
			string tagName = pMeshNode->pMesh->GetTagTypeName( j );

			if ( tagName == "Tag_Torso" )	// 将upper绑定到lower
			{
				assert( i->first == "lower" );
				SkinMeshList::iterator k = m_vpMeshList.find( "upper" );
				if ( k != m_vpMeshList.end() )
				{
					k->second.pPreNode = pMeshNode;
					k->second.tagID = j;
				}
			}
			else if ( tagName == "Tag_Head" )	// 将head绑定到upper
			{
				assert( i->first == "upper" );
				SkinMeshList::iterator k = m_vpMeshList.find( "head" );
				if ( k != m_vpMeshList.end() )
				{
					k->second.pPreNode = pMeshNode;
					k->second.tagID = j;
				}
			}
			else if ( tagName == "Tag_Property" )	// 将道具绑定到身体的某一部位
			{
				SkinMeshList::iterator k = m_vpMeshList.find( "property" );
				if ( k != m_vpMeshList.end() )
				{
					k->second.pPreNode = pMeshNode;
					k->second.tagID = j;
				}
			}
		}
	}

	// 更新矩阵
	UpdateMatrix( 0 );

	//! 计算整体的包围盒
	CalcEntireBox();

	return TRUE;
}

/*! \param pd3dDevice D3D10设备
	\param time 程序运行时间
	\param worldMatVar 设置世界矩阵的接口
	\param texVar 设置纹理的接口
*/
void CSMMergingMesh::Render( ID3D10Device *pd3dDevice, const float time, ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar )
{
	assert( pd3dDevice != NULL && worldMatVar != NULL && texVar != NULL );

	for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		MeshNode *pMN = &( i->second );
		pMN->pMesh->Render( pd3dDevice, time, worldMatVar, texVar );
	}
}

/*! \param pd3dDevice D3D10设备
	\param worldMatVar 设置世界矩阵的接口
	\param texVar 设置纹理的接口
*/
void CSMMergingMesh::RenderSingleFrame( ID3D10Device *pd3dDevice, ID3D10EffectMatrixVariable *worldMatVar, ID3D10EffectShaderResourceVariable *texVar )
{
	assert( pd3dDevice != NULL && worldMatVar != NULL && texVar != NULL );

	for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		MeshNode *pMN = &( i->second );
		pMN->pMesh->RenderSingleFrame( pd3dDevice, worldMatVar, texVar );
	}
}

void CSMMergingMesh::UpdateFrame()
{
	//! 根据模型间的绑定信息更新世界矩阵
	UpdateMatrix( m_curAnimState.curr_frame );

	//! 更新模型顶点
	for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		MeshNode *pMN = &( i->second );
		pMN->pMesh->UpdateFrame();
	}
}

/*! \param frame 当前帧 */
void CSMMergingMesh::UpdateMatrix( int frame )
{
	assert( frame >= 0 && frame < m_numSceneFrames && "frame越界" );

	for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		MeshNode *pMeshNode = &( i->second );
		D3DXMATRIX worldMatrix = m_worldMatrix;

		MeshNode *pMN = pMeshNode;
		while ( pMN->pPreNode != NULL )
		{
			CSMSkinMesh *pPreMesh = pMN->pPreNode->pMesh;
			D3DXMATRIX mountMat = pPreMesh->GetTagFrame( pMN->tagID, frame );

			D3DXMatrixMultiply( &worldMatrix, &worldMatrix, &mountMat );

			pMN = pMN->pPreNode;
		}

		pMeshNode->pMesh->SetWorldMatrix( worldMatrix );
	}
}

void CSMMergingMesh::CalcEntireBox()
{
	D3DXVECTOR3 min, max;
	memset( &min, 0, sizeof( D3DXVECTOR3 ) );
	memset( &max, 0, sizeof( D3DXVECTOR3 ) );
	for ( SkinMeshList::iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		MeshNode *pMeshNode = &( i->second );
		CSMSkinMesh *pCSM = pMeshNode->pMesh;

		D3DXVECTOR3 bMin = pCSM->GetEntireBox().boxMin;
		D3DXVECTOR3 bMax = pCSM->GetEntireBox().boxMax;
		D3DXVec3TransformCoord( &bMin, &bMin, &( pCSM->GetWorldMatrix() ) );
		D3DXVec3TransformCoord( &bMax, &bMax, &( pCSM->GetWorldMatrix() ) );
		if ( min.x > bMin.x )
		{
			min.x = bMin.x;
		}
		if ( min.y > bMin.y )
		{
			min.y = bMin.y;
		}
		if ( min.z > bMin.z )
		{
			min.z = bMin.z;
		}
		if ( max.x  < bMax.x )
		{
			max.x = bMax.x;
		}
		if ( max.y < bMax.y )
		{
			max.y = bMax.y;
		}
		if ( max.z < bMax.z )
		{
			max.z = bMax.z;
		}
	}
	m_entireBox.boxMin = min;
	m_entireBox.boxMax = max;

	CalcEntireBoxInfo();
}

int CSMMergingMesh::GetTotalNumVtx() const
{
	int n = 0;
	for ( SkinMeshList::const_iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		const MeshNode *pMeshNode = &( i->second );
		const CSMSkinMesh *pCSM = pMeshNode->pMesh;

		n += pCSM->GetTotalNumVtx();
	}
	return n;
}

int CSMMergingMesh::GetTotalNumUV() const
{
	int n = 0;
	for ( SkinMeshList::const_iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		const MeshNode *pMeshNode = &( i->second );
		const CSMSkinMesh *pCSM = pMeshNode->pMesh;

		n += pCSM->GetTotalNumUV();
	}
	return n;
}

int CSMMergingMesh::GetTotalNumTri() const
{
	int n = 0;
	for ( SkinMeshList::const_iterator i = m_vpMeshList.begin(); i != m_vpMeshList.end(); i ++ )
	{
		const MeshNode *pMeshNode = &( i->second );
		const CSMSkinMesh *pCSM = pMeshNode->pMesh;

		n += pCSM->GetTotalNumTri();
	}
	return n;
}