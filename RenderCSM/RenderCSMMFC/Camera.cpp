#include "stdafx.h"
#include "Camera.h"

Camera::Camera( const D3DXVECTOR3 tracePos, const float distance, const float minDis, const float maxDis )
{
	m_tracePos = tracePos;
	m_distance = distance;
	m_fMinDis = minDis;
	m_fMaxDis = maxDis;
	m_HArc = 0;
	m_VArc = 0;

	CalculatePossibleCamPosition();
}

Camera::~Camera(void)
{
}

void Camera::Reset()
{
	m_cameraPos = m_possibleCameraPos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_tracePos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
}

D3DXVECTOR3 Camera::CalculatePossibleCamPosition()
{
	m_possibleCameraPos.x = m_tracePos.x - m_distance * cos( m_VArc ) * sin( m_HArc );
	m_possibleCameraPos.z = m_tracePos.z - m_distance * cos( m_VArc ) * cos( m_HArc );
	m_possibleCameraPos.y = m_tracePos.y - m_distance * sin( m_VArc );

	return m_possibleCameraPos;
}

D3DXMATRIX Camera::UpdateViewMatrix()
{	
	m_cameraPos = m_possibleCameraPos;

	//摄像机的向上方向固定为Y轴
	D3DXMatrixLookAtLH( &m_matView, 
					    &m_cameraPos,
					    &m_tracePos,
					    &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );

	return m_matView;
} 

void Camera::IncreaseDistance( const float distanceInc )
{
	m_distance += distanceInc;

	if ( m_distance >= m_fMaxDis )
	{
		m_distance = m_fMaxDis;
	}
	if ( m_distance <= m_fMinDis )
	{
		m_distance = m_fMinDis;
	}
}

void Camera::IncreaseHArc( const float arcInc )
{
	m_HArc += arcInc;
}

void Camera::IncreaseVArc( const float arcInc )
{
	m_VArc -= arcInc;
	
	if( m_VArc >= static_cast< float >( D3DX_PI / 180 * 89 ) )
	{
		m_VArc = static_cast< float >( D3DX_PI / 180 * 89);
	}
	else if( m_VArc <= static_cast< float >( -D3DX_PI / 180 * 89 ) )
	{
		m_VArc = static_cast< float >( -D3DX_PI / 180 * 89 );
	}
}

D3DXVECTOR3 Camera::GetDirection()
{
	D3DXVECTOR3 cameraDir;
	cameraDir =  m_cameraPos - m_tracePos;
	D3DXVec3Normalize( &cameraDir, &cameraDir );
	return cameraDir;
}