#include "stdafx.h"
#include "SceneState.h"
#include "DXManager.h"

void SceneState_Nothing::Enter( DXManager * pDX )
{
	pDX->GetCamera()->SetTracePosition( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
	pDX->GetCamera()->CalculatePossibleCamPosition();
}

void SceneState_Nothing::Execute( DXManager * pDX )
{
	// Nothing
}

void SceneState_Nothing::Exit( DXManager * pDX )
{

}

SceneState_Nothing * SceneState_Nothing::Instance()
{
	static SceneState_Nothing instance;
	return &instance;
}

void SceneState_EntireModel::Enter( DXManager * pDX )
{
	assert( pDX->GetMesh() != NULL &&
		"Mesh不能为空" );

	Camera *pCamera = pDX->GetCamera();
	ModelBase *pMesh = pDX->GetMesh();

	pCamera->SetTracePosition( pMesh->GetTracePosition() );
	pCamera->SetMinMaxDistance( pMesh->GetTraceDistanceMin(), pMesh->GetTraceDistanceMax() );
	pCamera->SetDistance( pMesh->GetTraceDistanceMax() );
	pCamera->CalculatePossibleCamPosition();
}

void SceneState_EntireModel::Execute( DXManager * pDX )
{

}

void SceneState_EntireModel::Exit( DXManager * pDX )
{

}

SceneState_EntireModel * SceneState_EntireModel::Instance()
{
	static SceneState_EntireModel instance;
	return &instance;
}

void SceneState_MergingModel::Enter( DXManager * pDX )
{
	assert( pDX->GetMesh() != NULL &&
		"Mesh不能为空" );

	Camera *pCamera = pDX->GetCamera();
	ModelBase *pMesh = pDX->GetMesh();

	pCamera->SetTracePosition( pMesh->GetTracePosition() );
	pCamera->SetMinMaxDistance( pMesh->GetTraceDistanceMin(), pMesh->GetTraceDistanceMax() );
	pCamera->SetDistance( pMesh->GetTraceDistanceMax() );
	pCamera->CalculatePossibleCamPosition();
}

void SceneState_MergingModel::Execute( DXManager * pDX )
{

}

void SceneState_MergingModel::Exit( DXManager * pDX )
{

}

SceneState_MergingModel * SceneState_MergingModel::Instance()
{
	static SceneState_MergingModel instance;
	return &instance;
}