#ifndef __CSMMESH_DATA_H__
#define __CSMMESH_DATA_H__

#include <vector>
#include <fstream>
#include <string>

#include "CSM.h"
#include "DrawNormal.h"

using std::vector;
using std::ifstream;
using std::string;
using std::runtime_error;


/*! \brief Crowd Simulation Model网格 */
class CSMMeshData
{
public:
	typedef vector< CSMTagData > TagList;			//!< TAG列表
	typedef vector< CSMSubMesh* > SubMeshList;		//!< SubMesh列表
	typedef vector< CSMBoneData > BoneList;				//!< 骨骼名称列表

	/*! \brief 构造函数 */
	CSMMeshData();
	/*! \brief 析构函数 */
	~CSMMeshData();

	/*! \brief 导入CSM文件,CSM=CrowdSimulationModel */
	BOOL LoadCSM( const string &fileName );

	DrawNormal *GetDrawNormal() const { return m_pDrawNormal; }

	///////////////////////////////////Get&Set Functions///////////////////////////////////////
	/*! \brief 获得TAG的数量 */
	int GetNumTags() const { return static_cast< int >( m_vpTags.size() ); }
	/*! \brief 获得CSM包含的帧数 */
	int GetNumFrames() const { return m_pHeader->numAnimFrames; }
	/*! \brief CSM是否包含动画 */
	BOOL HasAnimation() const { return m_pHeader->numAnimFrames > 1; }
	/*! \brief 获得骨骼数 */
	int GetNumBones() const { return m_pHeader->numBones; }
	CSMBoneData GetBone( int iBone ) const { return m_boneList[iBone]; }
	/*! \brief 获得SubMesh数 */
	int GetNumSubMeshes() const { return m_pHeader->numSubMesh; }
	/*! \brief 获得第iMesh个SubMesh的名称 */
	string GetMeshName( int iMesh ) const { return m_vpSubMeshes[iMesh]->subMeshHeader.name; }
	/*! \brief 获得第iMesh个SubMesh的顶点数 */
	int GetMeshNumVtx( int iMesh ) const { return m_vpSubMeshes[iMesh]->subMeshHeader.numVertices; }
	int GetMeshNumSkin( int iMesh ) const
	{ 
		if ( m_vpSubMeshes[iMesh]->subMeshHeader.numAnimFrames > 1 ) 
		{
			return GetMeshNumVtx( iMesh );
		}
		return 0;
	}
	/*! \brief 获得第iMesh个SubMesh的面数 */
	int GetMeshNumTri( int iMesh ) const { return m_vpSubMeshes[iMesh]->subMeshHeader.numFaces; }
	/*! \brief 获得第iMesh个SubMesh的动画帧数 */
	int GetMeshNumAnim( int iMesh ) const { return m_vpSubMeshes[iMesh]->subMeshHeader.numAnimFrames; }
	/*! \brief 获得第iMesh个SubMesh的纹理 */
	string GetMeshTexture( int iMesh ) const { return m_vpSubMeshes[iMesh]->textureFile; }
	/*! \brief 获得第iMesh个SubMesh的第iVtx个顶点 */
	CSMVertexData GetMeshVtx( int iMesh, int iVtx ) const { return m_vpSubMeshes[iMesh]->vVertexData[iVtx]; }
	CSMSkinData GetMeshSkin( int iMesh, int iVtx ) const { return m_vpSubMeshes[iMesh]->vSkinData[iVtx]; }
	/*! \brief 获得第iMesh个SubMesh的第iTri个面 */
	CSMTriangleData GetMeshTri( int iMesh, int iTri ) const { return m_vpSubMeshes[iMesh]->vTriangleData[iTri]; }
	/*! \brief 获得第i个TAG */
	CSMTagData GetTag( int i ) const { return m_vpTags[i]; }
	//////////////////////////////////////////////////////////////////////////////////////////

private:
	ifstream	m_inFileCSM;					//!< .CSM文件

	CSMHeader	*m_pHeader;						//!< 指向Header的指针
	TagList		m_vpTags;						//!< 存储所有的Tags
	SubMeshList m_vpSubMeshes;					//!< 存储所有的SubMesh

	BoneList	m_boneList;						//!< 骨骼的名称列表

	DrawNormal	*m_pDrawNormal;					//!< 用于绘制Normal
};

#endif