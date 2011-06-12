#include "stdafx.h"
#include "ModelBase.h"

void ModelBase::CalcEntireBoxInfo()
{
	D3DXVECTOR3 min = m_entireBox.boxMin;
	D3DXVECTOR3 max = m_entireBox.boxMax;

	m_entireCenter = ( min + max ) / 2;

	if ( max.x - min.x > m_entireLengthMax )
	{
		m_entireLengthMax = max.x - min.x;
	}
	if ( max.y - min.y > m_entireLengthMax )
	{
		m_entireLengthMax = max.y - min.y;
	}
	if ( max.z - min.z > m_entireLengthMax )
	{
		m_entireLengthMax = max.z - min.z;
	}
}