#include "CrowdSimulationModel.h"

//////////////////////////////ExportManager//////////////////////////////////

ExportManager::ExportManager()
{
	m_numAnimFrames = 0;
	m_bSplitMode = FALSE;
}

ExportManager::~ExportManager()
{
	for ( CSMList::iterator i = m_vpCSM.begin(); i != m_vpCSM.end(); i ++ )
	{
		SAFE_DELETE( *i );
	}
}

/*! \param outFile[] 输出文件流
*	\param fileName 文件名
*	\param binary 是否是二进制格式写出
*	\return 打开文件流失败返回FALSE
*/
BOOL ExportManager::BeginWriting( ofstream &outFile, const string &fileName, BOOL binary )
{
	if ( binary == TRUE )
	{
		outFile.open( fileName.c_str(), ofstream::out | ofstream::trunc | ofstream::binary );
	}
	else
	{
		outFile.open( fileName.c_str(), ofstream::out | ofstream::trunc );
	}

	return outFile.fail() ? FALSE : TRUE;
}

/*! \param outFile[] 输出文件流 */
void ExportManager::EndWriting( ofstream &outFile )
{
	if ( outFile )
	{
		outFile.close();
	}
}
/*! \param pNode 场景中开始寻找的节点
 *	\param nodeName 寻找节点的名称
 */
INode * ExportManager::FindNodeByName( INode *pNode, string nodeName )
{
	if ( nodeName == pNode->GetName() )
	{
		return pNode;
	}
	else
	{
		int numChildren = pNode->NumberOfChildren();
		for ( int i = 0; i < numChildren; i++ )
		{
			INode *childNode = pNode->GetChildNode( i );
			INode *resNode = NULL;
			resNode = FindNodeByName( childNode, nodeName );
			if ( resNode != NULL )
			{
				return resNode;
			}
		}
		return NULL;
	}
}

/*! \param pRootNode 3dmax场景的根节点
 *	\param bExportAnim 是否导出动画
 *	\param bTag 是否导出TAG
 *	\param headNode 头部节点名称
 *	\param upperNode 上半身节点名称
 *	\param lowerNode 下半身节点名称
 *	\param bProp 是否导出道具
 *	\param propNode 道具节点名称
 *	\param logFile 输出的日志文件名
 */
void ExportManager::Gather3DSMAXSceneData( INode *pRootNode, 
										   BOOL bExportAnim, 
										   BOOL bTag, string headNodeName, string upperNodeName, string lowerNodeName, 
										   BOOL bProp, string propNodeName, string mountTo, 
										   const string &logFile /* =  */ )
{
#ifdef LOG
	//! 设置输出log
	m_ofLog.open( logFile.c_str(), ofstream::out | ofstream::trunc );
#endif

	//! 搜集场景的帧信息
	if ( bExportAnim == TRUE )
	{
		Interval ivAnimRange = GetCOREInterface()->GetAnimRange();
		m_numAnimFrames = ( ivAnimRange.End() - ivAnimRange.Start() ) / GetTicksPerFrame() + 1;
	}
	else
	{
		// 没有动画则场景只有1帧
		m_numAnimFrames = 1;
	}

	vector< string > suffix( NUM_PART_IDENTITY );
	//! （根据模型拆分）搜集CSM所需数据
	if ( bTag == TRUE )
	{
		m_bSplitMode = TRUE;

		// 会导出多个CSM文件，*_lower.CSM, *_upper.CSM, *_head.CSM, *_property.CSM
		// suffix是导出文件名需要添加的内容
		suffix[LOWER] = "_lower";
		suffix[UPPER] = "_upper";
		suffix[HEAD] = "_head";
		suffix[PROPERTY] = "_property";
		for ( int id = LOWER; id <= HEAD; id ++ )
		{
			CSMInfo *csm = new CSMInfo( static_cast< PartIdentity >( id ) );
			csm->strAppend = suffix[id];

			m_vpCSM.push_back( csm );
		}

		// 找到场景中相应的节点
		INode *pLowerNode = FindNodeByName( pRootNode, lowerNodeName );
		INode *pUpperNode = FindNodeByName( pRootNode, upperNodeName );
		INode *pHeadNode = FindNodeByName( pRootNode, headNodeName );
		assert( pHeadNode != NULL && pUpperNode != NULL && pLowerNode != NULL );

		vector< INode* > nodeList( 4 );
		nodeList[LOWER] = pLowerNode;
		nodeList[UPPER] = pUpperNode;
		nodeList[HEAD]  = pHeadNode;

		// 将各节点对应到CSM文件
		for ( int id = LOWER; id <= HEAD; id ++ )
		{
			m_vpCSM[id]->pNode = nodeList[id];
		}

		// 处理道具
		INode *pPropNode = NULL;
		if ( bProp == TRUE )
		{
			CSMInfo *csm = new CSMInfo( PROPERTY );
			csm->strAppend = suffix[PROPERTY];
			m_vpCSM.push_back( csm );

			pPropNode = FindNodeByName( pRootNode, propNodeName );
			assert( pPropNode != NULL );
			nodeList.push_back( pPropNode );

			m_vpCSM[PROPERTY]->pNode = nodeList[PROPERTY];
		}

		// 搜集这些特定节点的信息
		GatherSpecifiedNodeData( m_vpCSM.begin(), m_vpCSM.end(), bExportAnim );

		// 将Lower转换到世界空间
		MaxTriObjData *pLowerObj = m_vpCSM[LOWER]->maxObjList[0];
		UTILITY->TransformToWorld( pLowerObj, pLowerObj->worldMat );

		// 搜集TAG信息
		GenTag( pLowerNode, pUpperNode, m_vpCSM[LOWER], "Tag_Torso" );
		GenTag( pUpperNode, pHeadNode, m_vpCSM[UPPER], "Tag_Head" );
		if ( bProp == TRUE )
		{
			if ( mountTo == "Head" )
			{
				GenTag( pHeadNode, pPropNode, m_vpCSM[PROPERTY], "Tag_Property" );
			}
			else if ( mountTo == "Upper" )
			{
				GenTag( pUpperNode, pPropNode, m_vpCSM[PROPERTY], "Tag_Property" );
			}
			else if ( mountTo == "Lower" )
			{
				GenTag( pLowerNode, pPropNode, m_vpCSM[PROPERTY], "Tag_Property" );
			}
			else
			{
				// should never be here
				assert( FALSE && "unknown mount type" );
			}
		}
	}
	else
	{
		m_bSplitMode = FALSE;

		// 只导出一个CSM文件
		suffix[ENTIRETY] = "";
		CSMInfo *csm = new CSMInfo( ENTIRETY );
		csm->strAppend = suffix[ENTIRETY];
		m_vpCSM.push_back( csm );

		// 搜集所有节点信息
		GatherRecursiveNodeData( pRootNode, bExportAnim );

		csm->maxObjList = m_vpMaxObjs;

		// 所有Mesh转换到世界空间
		for ( MaxTriObjList::iterator i = m_vpMaxObjs.begin(); i != m_vpMaxObjs.end(); i ++ )
		{
			MaxTriObjData *pMaxObj = *i;
			UTILITY->TransformToWorld( pMaxObj, pMaxObj->worldMat );
		}
	}

	//! 搜集动画数据
	if ( bExportAnim == TRUE )
	{
		//! 生成骨骼树结构
		GenBoneTree();

		//! 搜集骨骼蒙皮信息
		for ( SubMeshBoneList::iterator i = m_submeshBones.begin(); i != m_submeshBones.end(); i ++ )
		{
			MaxTriObjData *pMaxObjData = i->pMaxObjData;
			ISkinContextData *pSkinContext = i->pSkinContext;
			ISkin *pSkin = i->pSkin;

			GatherSkinData( pSkinContext, pSkin, pMaxObjData );
		}

		//! 搜集骨骼动画信息
		for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i ++ )
		{
			GatherBoneFrames( &( i->second ) );
		}
	}

	//! 将Max数据转为CSM数据
	for ( MaxTriObjList::iterator i = m_vpMaxObjs.begin(); i != m_vpMaxObjs.end(); i ++ )
	{
		MaxTriObjData *pTriData = *i;
		UTILITY->MaxDataToCSMData( pTriData );
	}

#ifdef LOG
	m_ofLog.close();
#endif
}

/*! \param pNode 场景中的节点
 *	\param bExportAnim 是否导出动画
 */
void ExportManager::GatherRecursiveNodeData( INode *pNode, BOOL bExportAnim )
{
	GatherNodeData( pNode, bExportAnim );

	// 递归搜集场景树中的节点
	int numChildren = pNode->NumberOfChildren();
	for ( int i = 0; i < numChildren; i++ )
	{
		INode *childNode = pNode->GetChildNode( i );
		GatherRecursiveNodeData( childNode, bExportAnim );
	}
}

/*! \param pNode 场景中的节点
 *	\param beg Node列表的begin
 *	\param end Node列表的end
 *	\param bExportAnim 是否导出动画
 */
void ExportManager::GatherSpecifiedNodeData( CSMList::iterator beg, CSMList::iterator end, BOOL bExportAnim )
{
	for ( CSMList::iterator i = beg; i != end; i ++ )
	{
		CSMInfo *csm = *i;
		assert( GatherNodeData( csm->pNode, bExportAnim ) );
		csm->maxObjList.push_back( m_vpMaxObjs.back() );
	}
}

/*! \param pNode 场景中的节点
 *	\param bExportAnim 是否导出动画
 *	\return 如果该节点是本插件所需的节点，则返回TRUE，否则返回FALSE
 */
BOOL ExportManager::GatherNodeData( INode *pNode, BOOL bExportAnim )
{
	// 获得BaseObject
	ObjectState os = pNode->EvalWorldState( 0 );
	Object *pObj = os.obj;

	// 在本export插件中，只输出未隐藏的triObject
	if ( pObj != NULL &&
		 FALSE == pNode->IsNodeHidden() &&
		 FALSE == IsBone( pNode ) &&		// 不是骨骼
		 pObj->CanConvertToType( triObjectClassID ) )
	{
		TriObject* pTriObj = static_cast< TriObject* >( pObj->ConvertToType( 0, triObjectClassID ) );
		if ( pTriObj != NULL )
		{

#ifdef LOG
			m_ofLog << "=>搜集Node" << pNode->GetName() << endl;
#endif

			MaxTriObjData *pMaxTriData = new MaxTriObjData();
			memset( pMaxTriData, 0, sizeof( MaxTriObjData ) );

			// 名称
			pMaxTriData->objName = pNode->GetName();

			// Mesh的世界矩阵
			Matrix3 worldMat = pNode->GetNodeTM( 0 );
			pMaxTriData->worldMat = UTILITY->TransformToDXMatrix( worldMat );

			// 搜集Mesh数据
			Mesh &lMesh = pTriObj->GetMesh();
			GatherMeshData( lMesh, pMaxTriData );

			// 搜集材质数据
			Mtl* pMtl = pNode->GetMtl();
			if ( pMtl != NULL )
			{
				GatherMaterialData( pMtl, pMaxTriData );
				
				if ( pMaxTriData->numTextures > 0 )
				{
					for ( vector< MaxTriangleData >::iterator i = pMaxTriData->vTriangleData.begin(); i != pMaxTriData->vTriangleData.end(); i ++ )
					{
						int faceID = i - pMaxTriData->vTriangleData.begin();
						// 获得该三角形面的Material ID
						i->materialID = lMesh.getFaceMtlIndex( faceID ) % pMaxTriData->numTextures;
					}
				}
			}
			else
			{
#ifdef LOG
				m_ofLog << "\t=>没有材质" << endl;
#endif
				// 添加一个空材质
				string texName = "";
				pMaxTriData->vTexNames.push_back( texName );
				pMaxTriData->numTextures ++;
			}

			// 计算顶点法线（必须在搜集材质之后，因为法线分组与材质相关）
			UTILITY->ComputeVertexNormalsOpt( pMaxTriData );

			if ( bExportAnim == TRUE )
			{
				// 搜集骨骼动画
				Modifier *pMf = GetSkinMode( pNode );
				if ( pMf != NULL )
				{
					ISkin* pSkin = static_cast< ISkin* > ( pMf->GetInterface( I_SKIN ) );
					ISkinContextData* pContext = pSkin->GetContextInterface( pNode );

					// 先存储起来，待会再搜集该Node的骨骼蒙皮信息（这么做是要等待所有的骨骼索引被最终确定）
					SubMeshBones pSubMeshBones;
					pSubMeshBones.pMaxObjData = pMaxTriData;
					pSubMeshBones.pSkinContext = pContext;
					pSubMeshBones.pSkin = pSkin;
					m_submeshBones.push_back( pSubMeshBones );

					// 搜集骨骼
					GatherBones( pSkin );

					// 有动画
					pMaxTriData->numAnimFrames = m_numAnimFrames;
				}
				else
				{
#ifdef LOG
					m_ofLog << "\t=>没有骨骼" << endl;
#endif
					// 没有动画
					pMaxTriData->numAnimFrames = 1;
				}
			}
			
			m_vpMaxObjs.push_back( pMaxTriData );

			if ( pTriObj != pObj )
			{
				pTriObj->DeleteMe();
			}
		}
		return TRUE;
	}
	
	return FALSE;
}

/*! \param lMesh 3dmax网格
	\param worldMat 网格的世界坐标
	\param pMaxTriData 存储搜集到的网格数据
 */
void ExportManager::GatherMeshData( Mesh &lMesh, MaxTriObjData *pMaxTriData )
{
	//! 搜集顶点信息
	int nVerts = lMesh.getNumVerts();
	pMaxTriData->vVertexData.clear();
	for ( int i = 0; i < nVerts; i++ )
	{
		MaxVertexData vd;
		Point3 &pts = lMesh.getVert( i );

		/*! 3Dmax9是Z-up的右手坐标系，而DX是Y-up的左手坐标系，所以需要调整坐标（交换y坐标和z坐标） */
		vd.position[0] = pts.x;
		vd.position[1] = pts.z;
		vd.position[2] = pts.y;

		pMaxTriData->vVertexData.push_back( vd );
	}
#ifdef LOG
	m_ofLog << "\t=>搜集顶点" << nVerts << "个" << endl;
#endif

	//! 搜集纹理坐标信息
	int nTVerts = lMesh.getNumTVerts();
	pMaxTriData->vTexCoordData.clear();
	for ( int i = 0; i < nTVerts; i++ )
	{
		MaxTexCoordData tcd;
		UVVert &lTVert = lMesh.getTVert( i );
		/*! 3dmax的u,v坐标系是这样	\n
		  （0,1) ----- (1,1)		\n
		        |     |				\n
				|     |				\n
		   (0,0) ----- (1,0)		\n
		   而dx的u,v坐标系是这样	\n
		  （0,0) ----- (1,0)		\n
		        |     |				\n
		        |     |				\n
		   (0,1) ----- (1,1)		\n
		   所以，需要调整v坐标 */
		tcd.u = lTVert.x;
		tcd.v = 1 - lTVert.y;

		pMaxTriData->vTexCoordData.push_back( tcd );
	}
#ifdef LOG
	m_ofLog << "\t=>搜集纹理坐标" << nTVerts << "个" << endl;
#endif

	//! 建立法线信息
	lMesh.buildNormals();
	//! 搜集三角形面信息
	int nTris = lMesh.getNumFaces();
#ifdef LOG
	m_ofLog << "\t=>搜集三角形面" << nTris << "个" << endl;
#endif
	for ( int i = 0; i < nTris; i++ )
	{
		MaxTriangleData td;
		//! 顶点坐标索引
		if ( nVerts > 0 )
		{
			Face& lFace = lMesh.faces[i];
			DWORD* pVIndices = lFace.getAllVerts();

			/*! 3Dmax9中三角形是逆时针，而DX是顺时针，所以需要调换顺序 */
			td.vertexIndices[0] = pVIndices[0];
			td.vertexIndices[1] = pVIndices[2];
			td.vertexIndices[2] = pVIndices[1];
		}
		else
		{
#ifdef LOG
			m_ofLog << "\t\t=>搜集顶点索引失败" << endl;
#endif
		}

		//! 纹理坐标索引
		if ( nTVerts > 0 )
		{
			TVFace& lTVFace = lMesh.tvFace[i];
			DWORD* pUVIndices = lTVFace.getAllTVerts();		

			/*! 3Dmax9中三角形是逆时针，而DX是顺时针，所以需要调换顺序 */
			td.texCoordIndices[0] = pUVIndices[0];
			td.texCoordIndices[1] = pUVIndices[2];
			td.texCoordIndices[2] = pUVIndices[1];
		}
		else
		{
#ifdef LOG
			m_ofLog << "\t\t=>搜集纹理索引失败" << endl;
#endif
		}

		//! 法线
		Point3& nvtx = Normalize( lMesh.getFaceNormal( i ) );
		/*! 3Dmax9是Z-up的右手坐标系，而DX是Y-up的左手坐标系，所以需要调整坐标（交换y坐标和z坐标） */
		td.normal[0] = nvtx.x;
		td.normal[1] = nvtx.z;
		td.normal[2] = nvtx.y;

		//! Smoothing Group
		td.smoothGroup = lMesh.faces[i].smGroup;

		pMaxTriData->vTriangleData.push_back( td );
	}
}

/*! \param pMtl 3dmax材质
	\param pSubMesh 存储搜集到的网格数据
*/
void ExportManager::GatherMaterialData( Mtl *pMtl, MaxTriObjData *pMaxTriData )
{
	if ( pMtl == NULL )
	{
		return;
	}

	//! 检测是否是一个标准的Material
	if ( pMtl->ClassID() == Class_ID( DMTL_CLASS_ID, 0 ) )
	{
		StdMat *pStdMtl = ( StdMat * )pMtl;
		BOOL bTwoSided = pStdMtl->GetTwoSided();

		Texmap *pTex = pMtl->GetSubTexmap( ID_DI );
		//! 如果材质是一个图像文件，则
		if ( pTex != NULL && pTex->ClassID() == Class_ID( BMTEX_CLASS_ID, 0 ) )
		{
			BitmapTex* pBmpTex = static_cast< BitmapTex* >( pTex );
			string texName = TruncatePath( pBmpTex->GetMapName() );
			//! 存储纹理名称
			pMaxTriData->vTexNames.push_back( texName );
			pMaxTriData->numTextures ++;
#ifdef LOG
			m_ofLog << "\t=>获取纹理材质" << texName << endl;
			StdUVGen *uv = pBmpTex->GetUVGen();
			float utile = uv->GetUScl( 0 );
			float vtile = uv->GetVScl( 0 );
			m_ofLog << "\t\tuTile=" << utile << ", vTile=" << vtile << endl;
#endif
		}
		else //! 材质不是一个图像文件，留空
		{
			string texName = "";
			pMaxTriData->vTexNames.push_back( texName );
			pMaxTriData->numTextures ++;
		}
	}

	for ( int i = 0; i < pMtl->NumSubMtls(); i ++ )
	{
		Mtl *pSubMtl = pMtl->GetSubMtl( i );
		GatherMaterialData( pSubMtl, pMaxTriData );
	}
}

/*! \param pTagedNode 提供TAG信息的节点
 *	\param pNode 节点绑定到pTagedNode的节点。\n
		         比如，pTagedNode是一个Lower，提供一个叫Torso的TAG，pNode是一个Upper，它绑定到Torso
 *	\param pCSM 存储Tag信息的CSM，它的identity必须不是ENTIRETY，且pCSM中存储的Node应该与pTagedNode相一致
 *	\param tagName 该TAG的名称
 */
void ExportManager::GenTag( INode *pTagedNode, INode *pNode, CSMInfo *pCSM, string tagName )
{
	if ( pCSM->identity == ENTIRETY || pCSM->pNode != pTagedNode )
	{
		return;
	}

	CSMTagData tag;
	strcpy( tag.name, tagName.c_str() );
	tag.numAnimFrames = m_numAnimFrames;
	for ( int i = 0; i < tag.numAnimFrames; i ++ )
	{
		TimeValue t = GetTicksPerFrame() * i;

		Matrix3 matA = pTagedNode->GetNodeTM( t );
		Matrix3 matB = pNode->GetNodeTM( t );
		Matrix3 mountMat = matB * Inverse( matA );

		tag.vFrameData.push_back( UTILITY->TransformToDXMatrix( mountMat ) );
	}
	pCSM->vpTags.push_back( tag );

#ifdef LOG
	m_ofLog << "\t=>获取TAG--" << tagName << ", 帧数为" << m_numAnimFrames << endl;
#endif
}

/*! \param pNode 3dmax场景节点
	\return 包含蒙皮信息的Modifier
 */
Modifier* ExportManager::GetSkinMode( INode *pNode )
{
	Object* pObj = pNode->GetObjectRef();
	if( !pObj )
	{
		return NULL;
	}
	while( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		IDerivedObject *pDerivedObj = dynamic_cast< IDerivedObject* >( pObj );
		int modStackIndex = 0;
		while( modStackIndex < pDerivedObj->NumModifiers() )
		{
			Modifier* mod = pDerivedObj->GetModifier( modStackIndex );
			if( mod->ClassID() == SKIN_CLASSID )
			{
				return mod;
			}
			modStackIndex++;
		}
		pObj = pDerivedObj->GetObjRef();
	}
	return NULL;
}

void ExportManager::GenBoneTree()
{
	vector< INode* > freshBoneNodes; // 存储需要新加入到BoneList中的骨骼节点
	//! 寻找到所有的骨骼
	for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i ++ )
	{
		INode *boneNode = i->second.pBoneNode;

		INode *parentNode = boneNode->GetParentNode();
		if ( FALSE == parentNode->IsRootNode() && parentNode != NULL )
		{
			string parentBoneName = parentNode->GetName();
			BoneList::iterator k = m_boneList.find( parentBoneName );
			if ( k == m_boneList.end() ) // 该父骨骼节点不在模型的蒙皮骨骼列表中
			{
				// 加入到待搜索列表中
				freshBoneNodes.push_back( parentNode );
			}
		}
	}
	//! 加入所有新找到的骨骼到骨骼列表
	for ( vector< INode* >::iterator i = freshBoneNodes.begin(); i != freshBoneNodes.end(); i ++ )
	{
		INode *pBoneNode = *i;
		while ( TRUE )
		{
			string boneName = pBoneNode->GetName();
			BoneList::iterator k = m_boneList.find( boneName );
			if ( k == m_boneList.end() )
			{
				Bone newBone;
				newBone.pBoneNode = pBoneNode;
				m_boneList.insert( BoneList::value_type( boneName, newBone ) );

				pBoneNode = pBoneNode->GetParentNode();
				if ( TRUE == pBoneNode->IsRootNode() || pBoneNode == NULL )
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	//! 生成骨骼索引
	int realIndex = 0;
	for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i ++, realIndex ++ )
	{
		i->second.index = realIndex;
	}

	//! 建立骨骼的父子关系
	for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i ++ )
	{
		INode *boneNode = i->second.pBoneNode;
		INode *parentBoneNode = boneNode->GetParentNode();

		if ( FALSE == parentBoneNode->IsRootNode() && parentBoneNode != NULL )
		{
			string parentBoneName = parentBoneNode->GetName();
			BoneList::iterator k = m_boneList.find( parentBoneName );
			assert( k != m_boneList.end() );
			i->second.pParentBone = &( k->second );
		}
	}

	//! 建立与父骨骼的相对矩阵
	for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i ++ )
	{
		i->second.relativeMat = GetRelativeBoneMatrix( &( i->second ) );

#ifdef DEBUG
		Matrix3 relMat = i->second.relativeMat;
		FilterData( relMat );
		m_ofLog << "\t" << i->first << "的相对矩阵：" << endl;
		m_ofLog << "\t\t" << relMat.GetRow( 0 ).x << "  " << relMat.GetRow( 0 ).y << "  " << relMat.GetRow( 0 ).z << endl;
		m_ofLog << "\t\t" << relMat.GetRow( 1 ).x << "  " << relMat.GetRow( 1 ).y << "  " << relMat.GetRow( 1 ).z << endl;
		m_ofLog << "\t\t" << relMat.GetRow( 2 ).x << "  " << relMat.GetRow( 2 ).y << "  " << relMat.GetRow( 2 ).z << endl;
		m_ofLog << "\t\t" << relMat.GetRow( 3 ).x << "  " << relMat.GetRow( 3 ).y << "  " << relMat.GetRow( 3 ).z << endl;
#endif

	}

	//! 建立所有骨骼相对于父骨骼的动画帧
	for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i ++ )
	{
#ifdef LOG
		m_ofLog << "\t骨骼" << i->first << ": " << endl; 
#endif

		for ( int j = 0; j < m_numAnimFrames; j ++ )
		{
			//! 第j帧的时刻time
			int time = j * GetTicksPerFrame();

			Matrix3 relTraMatT = GetLocalBoneTranMatrix( &( i->second ), time );

			i->second.localFrames.push_back( relTraMatT );

#ifdef DEBUG
			FilterData( relTraMatT );
			m_ofLog << "\tFrame" << j << ": " << endl;
			m_ofLog << "\t\t" << relTraMatT.GetRow( 0 ).x << "  " << relTraMatT.GetRow( 0 ).y << "  " << relTraMatT.GetRow( 0 ).z << endl;
			m_ofLog << "\t\t" << relTraMatT.GetRow( 1 ).x << "  " << relTraMatT.GetRow( 1 ).y << "  " << relTraMatT.GetRow( 1 ).z << endl;
			m_ofLog << "\t\t" << relTraMatT.GetRow( 2 ).x << "  " << relTraMatT.GetRow( 2 ).y << "  " << relTraMatT.GetRow( 2 ).z << endl;
			m_ofLog << "\t\t" << relTraMatT.GetRow( 3 ).x << "  " << relTraMatT.GetRow( 3 ).y << "  " << relTraMatT.GetRow( 3 ).z << endl;
#endif
		}
	}
}

/*! 这个函数很诡异，对于Bone都返回FALSE，对于Biped，footstep返回FALSE，其余的Biped则返回TRUE
	\param pNode 3dmax场景节点
	\return 判断结果
 */
BOOL ExportManager::IsBone( INode *pNode )
{
	if( pNode == NULL )
	{
		return FALSE; 
	}

	ObjectState os = pNode->EvalWorldState( 0 ); 
	if ( !os.obj ) 
	{
		return FALSE;
	}

	//! 检测是否是Bone
	if( os.obj->ClassID() == Class_ID( BONE_CLASS_ID, 0 ) )
	{
		return TRUE;
	}

	//! dummy节点也算为骨骼
	if( os.obj->ClassID() == Class_ID( DUMMY_CLASS_ID, 0 ) )
	{
		return TRUE;
	}

	//! 检测是否是Biped
	Control *cont = pNode->GetTMController();   
	if( cont->ClassID() == BIPSLAVE_CONTROL_CLASS_ID ||       //others biped parts    
		cont->ClassID() == BIPBODY_CONTROL_CLASS_ID )         //biped root "Bip01"     
	{
		return TRUE;
	}

	return FALSE;   
}

/*! \param pContext 3dmax蒙皮信息
	\param pSkin 与mesh所相关联的骨骼息
	\param pSubMesh 存储搜集到的网格数据
 */
void ExportManager::GatherSkinData( ISkinContextData* pContext, ISkin *pSkin, MaxTriObjData *pMaxTriData )
{
	//! 将存储在单个Sub Mesh中的Bone索引对应到“搜集完场景中所有的Bone”之后的索引
	map< int, int > oriToRealMap;
	int iBoneCnt = pSkin->GetNumBones();
	for ( int oriIndex = 0; oriIndex < iBoneCnt; oriIndex ++ )
	{
		string boneName = pSkin->GetBone( oriIndex )->GetName();

		//! 寻找到真正的索引并存储到映射表
		BoneList::iterator iter = m_boneList.find( boneName );
		int realIndex = iter->second.index;
		if ( iter != m_boneList.end() )
		{
			oriToRealMap.insert( map<int, int>::value_type( oriIndex, realIndex ) );
		}
		else
		{
			throw std::runtime_error( "Couldn't find bone!" );
		}
	}

	//! 更新索引，并存储蒙皮信息
	int numVtx = pContext->GetNumPoints();
	for ( int iVtx = 0; iVtx < numVtx; iVtx++ )
	{
		int num = pContext->GetNumAssignedBones( iVtx );
		assert( num <= 4 &&
			"本插件最多一个顶点只能绑定4个骨骼，你超过了，现在将失败退出" );
		for ( int iVBone = 0; iVBone < num; iVBone++ )
		{
			int oriBoneIdx = pContext->GetAssignedBone( iVtx, iVBone );
			float weight = pContext->GetBoneWeight( iVtx, iVBone );

			int realBoneIdx = oriToRealMap.find( oriBoneIdx )->second;
			pMaxTriData->vVertexData[iVtx].bones[iVBone] = realBoneIdx;
			pMaxTriData->vVertexData[iVtx].boneWeights[iVBone] = UTILITY->FilterData( weight );
		}
	}
}

/*! \param pSkin 包含与mesh所相关联的骨骼列表 */
void ExportManager::GatherBones( ISkin *pSkin )
{
	int iBoneCnt = pSkin->GetNumBones();
#ifdef LOG
	m_ofLog << "\t=>获取骨骼" << iBoneCnt << "个" << endl;
#endif
	for ( int i = 0; i < iBoneCnt; i ++ )
	{
		Bone bone;
		bone.pBoneNode = pSkin->GetBone( i );
		string boneName = bone.pBoneNode->GetName();
		
		m_boneList.insert( BoneList::value_type( boneName, bone ) );
	}
}

/*! \param pBone 骨骼 */
void ExportManager::GatherBoneFrames( Bone *pBone )
{
	INode *pBoneNode = pBone->pBoneNode;

#ifdef LOG
	m_ofLog << "=>获取骨骼" << pBoneNode->GetName() << "动画" << endl;
#endif

	// 获得节点的transform control
	Control *c = pBoneNode->GetTMController();

	//! 测试是否是一个biped controller(关于代码的解释请参见3dmax SDK Document)
	if ( ( c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID ) ||
		 ( c->ClassID() == BIPBODY_CONTROL_CLASS_ID ) ||
		 ( c->ClassID() == FOOTPRINT_CLASS_ID ) )
	{
		//!	Get the Biped Export Interface from the controller 
		IBipedExport *BipIface = ( IBipedExport * ) c->GetInterface( I_BIPINTERFACE );

		//!	Remove the non uniform scale
		BipIface->RemoveNonUniformScale( TRUE );

		//!	Release the interface when you are done with it
		c->ReleaseInterface( I_BIPINTERFACE, BipIface );
	}

	vector< Bone* > tempList;
	//! 遍历骨骼的父节点，直到根节点，加入到列表
	Bone *pRootBone = pBone;
	tempList.push_back( pRootBone );
	while ( pRootBone->pParentBone != NULL )
	{
		pRootBone = pRootBone->pParentBone;
		tempList.push_back( pRootBone );
	}

	//! baseMat是将Vworld转为Vlocal
	Matrix3 baseMat;
	baseMat.IdentityMatrix();
	//! 从根节点遍历，直到当前节点
	for ( vector< Bone* >::reverse_iterator riter = tempList.rbegin(); riter != tempList.rend(); riter ++ )
	{
		Bone *pB = *riter;
		baseMat =  baseMat * Inverse( pB->relativeMat );
	}

#ifdef DEBUG
	FilterData( baseMat );
	m_ofLog << "\t" << "Base矩阵：" << endl;
	m_ofLog << "\t\t" << baseMat.GetRow( 0 ).x << "  " << baseMat.GetRow( 0 ).y << "  " << baseMat.GetRow( 0 ).z << endl;
	m_ofLog << "\t\t" << baseMat.GetRow( 1 ).x << "  " << baseMat.GetRow( 1 ).y << "  " << baseMat.GetRow( 1 ).z << endl;
	m_ofLog << "\t\t" << baseMat.GetRow( 2 ).x << "  " << baseMat.GetRow( 2 ).y << "  " << baseMat.GetRow( 2 ).z << endl;
	m_ofLog << "\t\t" << baseMat.GetRow( 3 ).x << "  " << baseMat.GetRow( 3 ).y << "  " << baseMat.GetRow( 3 ).z << endl;
#endif

	//! 逐帧搜集
	for ( int i = 0; i < m_numAnimFrames; i ++ )
	{
		Matrix3 frame = baseMat;

		//! 第i帧的时刻time
		int time = i * GetTicksPerFrame();

		//! 从当前节点开始，向父节点遍历，直到根节点停止
		for ( vector< Bone* >::iterator iter = tempList.begin(); iter != tempList.end(); iter ++ )
		{
			Bone *pB = *iter;
			//! 乘以第i帧的子骨骼相对于本地坐标系的变换
			frame = frame * pB->localFrames[i];
			//! 变换至父坐标系
			frame = frame * pB->relativeMat;
		}

		//! 至此获得了时刻time的关键帧矩阵
		pBone->boneFrames.push_back( frame );

#ifdef DEBUG
		FilterData( frame );
		m_ofLog << "\tFrame" << i << ": " << endl;
		m_ofLog << "\t\t" << frame.GetRow( 0 ).x << "  " << frame.GetRow( 0 ).y << "  " << frame.GetRow( 0 ).z << endl;
		m_ofLog << "\t\t" << frame.GetRow( 1 ).x << "  " << frame.GetRow( 1 ).y << "  " << frame.GetRow( 1 ).z << endl;
		m_ofLog << "\t\t" << frame.GetRow( 2 ).x << "  " << frame.GetRow( 2 ).y << "  " << frame.GetRow( 2 ).z << endl;
		m_ofLog << "\t\t" << frame.GetRow( 3 ).x << "  " << frame.GetRow( 3 ).y << "  " << frame.GetRow( 3 ).z << endl;
#endif
	}

#ifdef LOG
	m_ofLog << "\t=>获取动画" << m_numAnimFrames << "帧" << endl;
#endif
}

/*! \param numAnimFrames 动画帧数 */
void ExportManager::GenCSMHeaderInfo( int numAnimFrames )
{
	for ( CSMList::iterator i = m_vpCSM.begin(); i != m_vpCSM.end(); i ++ )
	{
		CSMInfo *csm = *i;

		CSMHeader *pHeader = &( csm->header );
		memset( pHeader, 0, sizeof( CSMHeader ) );

		pHeader->ident = CSM_MAGIC_NUMBER;

		pHeader->numTags = static_cast< int >( csm->vpTags.size() );
		pHeader->numBones = static_cast< int >( m_boneList.size() );
		pHeader->numAnimFrames = numAnimFrames;
		pHeader->numSubMesh = 0;
		for ( MaxTriObjList::iterator j = csm->maxObjList.begin(); j != csm->maxObjList.end(); j ++ )
		{
			MaxTriObjData *pMaxObj = *j;
			int numSubMesh = static_cast< int >( pMaxObj->vSubMeshes.size() );
			pHeader->numSubMesh += numSubMesh;
		}

		pHeader->nHeaderSize = sizeof( CSMHeader );

		pHeader->nOffBones = pHeader->nHeaderSize;
		for ( TagList::iterator j = csm->vpTags.begin(); j != csm->vpTags.end(); j ++ )
		{
			CSMTagData pTag = *j;
			pHeader->nOffBones += sizeof( MAX_STRING_LENGTH );
			pHeader->nOffBones += sizeof( int );
			pHeader->nOffBones += sizeof( D3DXMATRIX ) * pTag.numAnimFrames;
		}

		pHeader->nOffSubMesh = pHeader->nOffBones + sizeof( CSMBoneData ) * pHeader->numBones;

		pHeader->nFileSize = pHeader->nOffSubMesh;
		for ( MaxTriObjList::iterator j = csm->maxObjList.begin(); j != csm->maxObjList.end(); j ++ )
		{
			MaxTriObjData *pMaxObj = *j;
			for ( SubMeshList::iterator k = pMaxObj->vSubMeshes.begin(); k != pMaxObj->vSubMeshes.end(); k ++ )
			{
				CSMSubMesh *pSubMesh = *k;
				pHeader->nFileSize += pSubMesh->subMeshHeader.nOffEnd;
			}
		}
	}
}

/*! \param fileName 输出文件名
	\return 打开文件失败，返回FALSE
 */
BOOL ExportManager::WriteAllCSMFile( const string &fileName )
{
	size_t token = fileName.find_last_of( "CSM" );
	string strFore = fileName.substr( 0, token - 3 );

	for ( CSMList::iterator i = m_vpCSM.begin(); i != m_vpCSM.end(); i ++ )
	{
		CSMInfo *csm = *i;
		string strAppend = csm->strAppend + ".CSM";
		csm->strCSMFile = strFore + strAppend;

		if ( WriteCSMFile( csm ) == FALSE )
		{
			return FALSE;
		}
	}

	if ( TRUE == m_bSplitMode && m_vpCSM.size() > 1 )
	{
		WriteCSM_PACFile( fileName );
	}
	
	return TRUE;
}

/*! \param pCSM 需要输出的CSM信息
 *	\return 打开文件失败，返回FALSE
 */
BOOL ExportManager::WriteCSMFile( CSMInfo *pCSM )
{
	string fileName = pCSM->strCSMFile;
	size_t token = fileName.find_last_of( '\\' );
	string path = fileName.substr( 0, token );

	ofstream &ofFileCSM = pCSM->ofCSMFile;
	CSMHeader *pHeader = &( pCSM->header );

	if ( BeginWriting( ofFileCSM, fileName, TRUE ) )
	{
		//! 统计CSM信息，填充CSM的Header
		GenCSMHeaderInfo( m_numAnimFrames );

		//! 将Header写入文件
		ofFileCSM.write( reinterpret_cast< char* >( pHeader ), sizeof( CSMHeader ) );

		int offset = pHeader->nHeaderSize;
		//! 将Tag Data写入文件
		for ( TagList::iterator i =pCSM->vpTags.begin(); i != pCSM->vpTags.end(); i ++ )
		{
			CSMTagData pTag = *i;
			
			ofFileCSM.seekp( offset, ofstream::beg );
			ofFileCSM.write( reinterpret_cast< char* >( pTag.name ), MAX_STRING_LENGTH );
			offset += MAX_STRING_LENGTH;

			ofFileCSM.seekp( offset, ofstream::beg );
			ofFileCSM.write( reinterpret_cast< char* >( &pTag.numAnimFrames ), sizeof( int ) );
			offset += sizeof( int );

			for ( int j = 0; j < pTag.numAnimFrames; j ++ )
			{
				D3DXMATRIX mtx = pTag.vFrameData[j];
				ofFileCSM.seekp( offset, ofstream::beg );
				ofFileCSM.write( reinterpret_cast< char* >( &mtx ), sizeof( MATRIX ) );
				offset += sizeof( MATRIX );
			}
		}

		//! 将Bones写入文件
		int n = 0;
		for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i++, n++ )
		{
			CSMBoneData CSMBone;
			string boneName = i->first;
			strcpy( CSMBone.name, boneName.c_str() );
			CSMBone.ID = i->second.index;
			if ( i->second.pParentBone != NULL )
			{
				CSMBone.parentID = i->second.pParentBone->index;
			}
			else
			{
				CSMBone.parentID = -1;
			}
			CSMBone.relativeMat = UTILITY->TransformToDXMatrix( i->second.relativeMat );
			ofFileCSM.seekp( pHeader->nOffBones + n * sizeof( CSMBone ), ofstream::beg );
			ofFileCSM.write( reinterpret_cast< const char* >( &CSMBone ), sizeof( CSMBone ) );
		}

		//! 将Sub Mesh写入文件
		int lastOffSet = pHeader->nOffSubMesh;
		for ( MaxTriObjList::iterator i = pCSM->maxObjList.begin(); i != pCSM->maxObjList.end(); i ++ )
		{
			MaxTriObjData *pMaxObj = *i;

			for ( SubMeshList::iterator j = pMaxObj->vSubMeshes.begin(); j != pMaxObj->vSubMeshes.end(); j ++ )
			{
				CSMSubMesh *pSubMesh = *j;

				//! 写入Sub Mesh Header
				ofFileCSM.seekp( lastOffSet, ofstream::beg );
				ofFileCSM.write( reinterpret_cast< char* >( &pSubMesh->subMeshHeader ), pSubMesh->subMeshHeader.nHeaderSize );

				//! 写入Texture File
				string oldTexFile = pSubMesh->textureFile;
				string texName;
				if ( oldTexFile != "" )
				{
					//! 将texture文件拷贝到目录下
					token = oldTexFile.find_last_of( '\\' );
					texName = oldTexFile.substr( token + 1 );
					string newTexFile = path + "\\" + texName;
					CopyFile( pSubMesh->textureFile, newTexFile.c_str(), FALSE );
				}
				char texFile[ MAX_STRING_LENGTH ];
				strcpy( texFile, texName.c_str() );
				ofFileCSM.seekp( lastOffSet + pSubMesh->subMeshHeader.nHeaderSize, ofstream::beg );
				ofFileCSM.write( reinterpret_cast< char* >( texFile ), MAX_STRING_LENGTH );

				//! 写入Vertex Data
				for ( int k = 0; k < pSubMesh->subMeshHeader.numVertices; k ++ )
				{
					CSMVertexData *pVertex = &pSubMesh->vVertexData[k];
#ifdef FILTER
					UTILITY->FilterData( *pVertex );
#endif
					ofFileCSM.seekp( lastOffSet + pSubMesh->subMeshHeader.nOffVertices + k * sizeof( CSMVertexData ), ofstream::beg );
					ofFileCSM.write( reinterpret_cast< char* >( pVertex ), sizeof( CSMVertexData ) );
				}

				//! 写入Skin Data
				int numSkinData;
				if ( pSubMesh->subMeshHeader.numAnimFrames > 1 )
				{
					numSkinData = pSubMesh->subMeshHeader.numVertices;
				}
				else
				{
					numSkinData = 0;
				}
				for ( int k = 0; k < numSkinData; k ++ )
				{
					CSMSkinData *pSkinData = &pSubMesh->vSkinData[k];
#ifdef FILTER
					UTILITY->FilterData( *pSkinData );
#endif
					ofFileCSM.seekp( lastOffSet + pSubMesh->subMeshHeader.nOffSkin + k * sizeof( CSMSkinData ), ofstream::beg );
					ofFileCSM.write( reinterpret_cast< char* >( pSkinData ), sizeof( CSMSkinData ) );
				}

				//! 写入Triangle Data
				for ( int k = 0; k < pSubMesh->subMeshHeader.numFaces; k ++ )
				{
					CSMTriangleData *pTriData = &pSubMesh->vTriangleData[k];

					ofFileCSM.seekp( lastOffSet + pSubMesh->subMeshHeader.nOffFaces + k * sizeof( CSMTriangleData ), ofstream::beg );
					ofFileCSM.write( reinterpret_cast< char* >( pTriData ), sizeof( CSMTriangleData ) );
				}

				lastOffSet = lastOffSet + pSubMesh->subMeshHeader.nOffEnd;
			}
		}

		EndWriting( ofFileCSM );

		return TRUE;
	}

	return FALSE;
}

/*! \param fileName 输出文件名 
	\return 打开文件失败，返回FALSE
 */
BOOL ExportManager::WriteAMFile( const string &fileName )
{
	//! 先写入动画配置文件
	if ( FALSE == WriteCFGFile( fileName ) )
	{
		return FALSE;
	}

	size_t token = fileName.find_last_of( ".CSM" );
	string animFile = fileName.substr( 0, token - 3 ) + ".AM";

	if ( BeginWriting( m_ofFileAM, animFile, TRUE ) )
	{
		//! 逐骨骼写入
		for ( BoneList::iterator i = m_boneList.begin(); i != m_boneList.end(); i ++ )
		{
			Bone bone = i->second;
			int boneSeek = bone.index * m_numAnimFrames * sizeof( MATRIX );
			//! 逐帧写入
			for ( BoneFrames::iterator j = bone.boneFrames.begin(); j != bone.boneFrames.end(); j++ )
			{
				MATRIX m = UTILITY->TransformToDXMatrix( *j );
#ifdef FILTER
				UTILITY->FilterData( m );
#endif
				int frameIndex = j - bone.boneFrames.begin();
				int frameSeek = boneSeek + frameIndex * sizeof( MATRIX );

				m_ofFileAM.seekp( frameSeek, ofstream::beg );
				m_ofFileAM.write( reinterpret_cast< char* >( &m ), sizeof( MATRIX ) );
			}
		}

		EndWriting( m_ofFileAM );

		return TRUE;
	}
	return FALSE;
}

/*! \param fileName 输出文件名
	\return 打开文件失败，返回FALSE
 */
BOOL ExportManager::WriteCFGFile( const string& fileName )
{
	size_t token = fileName.find_last_of( ".CSM" );
	string cfgFile = fileName.substr( 0, token - 3 ) + ".CFG";

	if ( BeginWriting( m_ofFileCFG, cfgFile, FALSE ) )
	{
		//! 输出场景的动画帧数
		m_ofFileCFG << "SceneFrame: " << m_numAnimFrames << endl;

		//! 骨骼数
		m_ofFileCFG << "NumberOfBone: " << m_boneList.size() << endl;

		//! 输出动画个数
		m_ofFileCFG << "NumberOfAnimation: " << m_animList.size() << endl;

		//! 逐动画输出动画信息
		for ( AnimList::iterator i = m_animList.begin(); i != m_animList.end(); i ++ )
		{
			m_ofFileCFG << i->animName << "\t" 
						<< i->firstFrame << "\t" 
						<< i->numFrames << "\t" 
						<< i->numLoops << "\t" 
						<< i->fps << endl;
		}
		
		EndWriting( m_ofFileCFG );

		return TRUE;
	}
	return FALSE;
}

BOOL ExportManager::WriteCSM_PACFile( const string &fileName )
{
	size_t token = fileName.find_last_of( ".CSM" );
	string pacFile = fileName.substr( 0, token - 3 ) + ".CSM_PAC";

	if ( BeginWriting( m_ofFileCSM_PAC, pacFile, FALSE ) )
	{
		//! 输出总共的CSM数量
		int num = static_cast< int >( m_vpCSM.size() );
		m_ofFileCSM_PAC << num << endl;

		vector< string > vID( 4 );
		vID[ LOWER ] = "lower";
		vID[ UPPER ] = "upper";
		vID[ HEAD  ] = "head";
		vID[ PROPERTY ] = "property";
		//! 逐个输出所有的CSM信息
		for ( CSMList::iterator i = m_vpCSM.begin(); i != m_vpCSM.end(); i ++ )
		{
			CSMInfo *csm = *i;
			assert( csm->identity < ENTIRETY && 
				"导出单个CSM不应该导出csm_pac文件" );

			string id = vID[ csm->identity ];
			string csmFile = csm->strCSMFile;
			token = csmFile.find_last_of( "\\" );
			if ( token != string::npos )
			{
				csmFile = csmFile.substr( token + 1 );
			}

			m_ofFileCSM_PAC << id << "\t" << csmFile << endl;
		}

		return TRUE;
	}
	return FALSE;
}


/*! \param pNode 3dmax场景节点
	\param t 时间点
	\return 骨骼矩阵
 */
Matrix3 ExportManager::GetBoneTM( INode *pNode, TimeValue t )   
{   
	Matrix3 tm = pNode->GetNodeTM( t );
	tm.NoScale();   
	return tm;   
} 

/*! \param pNode 3dmax场景中的节点
	\param t 时间
	\return 在时刻t节点相对于其父节点的矩阵
 */
Matrix3 ExportManager::GetRelativeMatrix( INode *pNode, TimeValue t /* = 0  */ )
{
	Matrix3 worldMat = pNode->GetNodeTM( t );
	Matrix3 parentMat = pNode->GetParentTM( t );

	//! 因为NodeWorldTM = NodeLocalTM * ParentWorldTM（注意：3dmax9中的矩阵是右乘）\n
	//! 所以NodeLocalTM = NodeWorldTM * Inverse( ParentWorldTM )
	Matrix3 relativeMat = worldMat * Inverse( parentMat );

	return relativeMat;
}

/*! 根骨骼的节点可能在场景中还有父节点（比如Scene Root）
	\param pBone 骨骼
	\param t 时间
	\return 在时刻t骨骼相对于其父骨骼的矩阵
 */
Matrix3 ExportManager::GetRelativeBoneMatrix( Bone *pBone, TimeValue t /* = 0  */ )
{
	if ( pBone->pParentBone == NULL )
	{
		Matrix3 worldMat = pBone->pBoneNode->GetNodeTM( t );
		return worldMat;
	}
	else
	{
		return GetRelativeMatrix( pBone->pBoneNode, t );
	}
}

/*! \param pBone 3dmax场景中的节点
	\param t 时间
	\return 骨骼在时刻t在本地空间内的变换
 */
Matrix3 ExportManager::GetLocalBoneTranMatrix( Bone *pBone, TimeValue t )
{
	Matrix3 relMatT0 = GetRelativeBoneMatrix( pBone, 0 );
	Matrix3 relMatTN = GetRelativeBoneMatrix( pBone, t );
	//! 因为relMatT = transformMatT * relMat0
	//! 所以transformMatT = relMatT * Inverse( relMat0 ) 
	Matrix3 transformMatT = relMatTN * Inverse( relMatT0 );

	return transformMatT;
}