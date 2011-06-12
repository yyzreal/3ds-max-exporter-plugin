#ifndef __DRAWNORMAL_H__
#define __DRAWNORMAL_H__

#include <vector>
using std::vector;
using std::runtime_error;

struct Line
{
	D3DXVECTOR3 start;
	D3DXVECTOR3 end;

	Line()
	{
		ZeroMemory( &start, sizeof( start ) );
		ZeroMemory( &end, sizeof( end ) );
	}

	Line( const D3DXVECTOR3 &s, const D3DXVECTOR3 &e )
	{
		start = s;
		end = e;
	}
};

class DrawNormal
{
public:
	DrawNormal( UINT length );
	~DrawNormal();

	void AddNormal( const D3DXVECTOR3 &pos, const D3DXVECTOR3 &normal );
	void Initialize( ID3D10Device *pD3DDevice, ID3D10Effect *pEffect );
	void Draw( ID3D10Device *pD3DDevice );

private:
	void InitVertexBuffer( ID3D10Device *pD3DDevice );
	void InitShader( ID3D10Device *pD3DDevice, ID3D10Effect *pEffect );

private:
	vector< Line >		m_vNormalLines;
	UINT				m_length;
	BOOL				m_bInit;

	ID3D10Buffer		*m_pVertexBuffer;
	ID3D10EffectTechnique	*m_pTechnique;
	ID3D10InputLayout	*m_pInputLayout;
};

#endif