/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <DX8\VertexBuffer.h>
#include <DX8\DX8Viewer.h>

#include <debug\debugclient.h>

#include <vector>

#include <d3d8.h>
#include <d3dx8.h>

#include <Misc/Misc.h>



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CVertexBuffer::~CVertexBuffer()
{

  m_pViewer->RemoveVertexBuffer( this );
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    SafeRelease( *it );

    ++it;
  }
  m_vectVertexBuffers.clear();

}



/*-Invalidate-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CVertexBuffer::Invalidate()
{

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    SafeRelease( *it );

    ++it;
  }
  m_vectVertexBuffers.clear();

}



/*-CreateBuffers--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CVertexBuffer::CreateBuffers( LPDIRECT3DDEVICE8 pd3dDevice )
{

  if ( pd3dDevice == NULL )
  {
    return false;
  }
  D3DCAPS8    d3dCaps;
  
  if ( FAILED( pd3dDevice->GetDeviceCaps( &d3dCaps ) ) )
  {
    return false;
  }

  m_dwPrimitivePartCount = d3dCaps.MaxPrimitiveCount;

  DWORD   dwPrimitives = m_dwPrimitiveCount;

  while ( dwPrimitives )
  {
    DWORD     dwPrimitivesThisBuffer = dwPrimitives;

    if ( dwPrimitivesThisBuffer > m_dwPrimitivePartCount )
    {
      dwPrimitivesThisBuffer = m_dwPrimitivePartCount;
    }

    IDirect3DVertexBuffer8*   pVertexBuffer = NULL;

    HRESULT hr = pd3dDevice->CreateVertexBuffer( dwPrimitivesThisBuffer * m_dwPrimitiveSize * 3, 
                                                 m_dwUsage, 
                                                 m_dwFVF, 
                                                 m_d3dPool, 
                                                 &pVertexBuffer );

    if ( hr != D3D_OK )
    {
      dh::Log( "CD3DViewer::CreateVertexBuffer: CreateVertexBuffer failed" );
      Invalidate();
      return false;
    }

    m_vectVertexBuffers.push_back( pVertexBuffer );

    dwPrimitives -= dwPrimitivesThisBuffer;
  }

  return true;

}



/*-Restore--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CVertexBuffer::Restore( LPDIRECT3DDEVICE8 pd3dDevice )
{

  if ( !pd3dDevice )
  {
    return FALSE;
  }

  IDirect3DVertexBuffer8  *pVertexBuffer = NULL;

  if ( !m_vectVertexBuffers.empty() )
  {
    tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
    while ( it != m_vectVertexBuffers.end() )
    {
      SafeRelease( *it );

      ++it;
    }
    m_vectVertexBuffers.clear();
  }

  if ( !CreateBuffers( pd3dDevice ) )
  {
    return FALSE;
  }

  UpdateData();

  return TRUE;

}





/*-UpdateData-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CVertexBuffer::UpdateData()
{

  BYTE*   pData = (BYTE*)m_pData;

  DWORD   dwPrimitivesLeft = m_dwPrimitiveCount;

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    IDirect3DVertexBuffer8*   pVB = *it;

    DWORD   dwPrimitivesToCopy = dwPrimitivesLeft;
    if ( dwPrimitivesToCopy > m_dwPrimitivePartCount )
    {
      dwPrimitivesToCopy = m_dwPrimitivePartCount;
    }

    BYTE*   pPos;

    pVB->Lock( 0, 0, &pPos, 0 );

    memcpy( pPos, pData, dwPrimitivesToCopy * m_dwPrimitiveSize * 3 );

    pVB->Unlock();

    pData += dwPrimitivesToCopy * m_dwPrimitiveSize * 3;

    dwPrimitivesLeft -= dwPrimitivesToCopy;

    ++it;
  }

}



/*-CloneVertexBuffer----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CVertexBuffer* CVertexBuffer::Clone()
{

  CVertexBuffer*  pVertexBuffer = NULL;

  void*   pData;

  pVertexBuffer = m_pViewer->CreateVertexBuffer( m_dwPrimitiveCount,
                                                 m_dwPrimitiveSize,
                                                 m_dwCompleteBufferSize,
                                                 m_d3dPrimType,
                                                 m_dwUsage,
                                                 m_dwFVF,
                                                 m_d3dPool );

  DWORD   dwPrimitivesToCopy = m_dwPrimitiveCount;

  CVertexBuffer::tVectVertexBuffers::iterator    itSource( m_vectVertexBuffers.begin() );
  CVertexBuffer::tVectVertexBuffers::iterator    itTarget( pVertexBuffer->m_vectVertexBuffers.begin() );
  while ( ( itSource != m_vectVertexBuffers.end() )
  &&      ( itTarget != pVertexBuffer->m_vectVertexBuffers.end() ) )
  {
    IDirect3DVertexBuffer8*   pVBSource = *itSource;
    IDirect3DVertexBuffer8*   pVBTarget = *itTarget;

    DWORD   dwCopyNow = dwPrimitivesToCopy;

    if ( dwCopyNow > m_dwPrimitivePartCount )
    {
      dwCopyNow = m_dwPrimitivePartCount;
    }
    
    BYTE*   pSource = NULL;

    pVBTarget->Lock( 0, 0, (BYTE**)&pData, 0 );//D3DLOCK_DISCARD );
    pVBSource->Lock( 0, 0, (BYTE**)&pSource, 0 );//D3DLOCK_READONLY );

    memcpy( pData, pSource, dwCopyNow * m_dwPrimitiveSize );

    pVBTarget->Unlock();
    pVBSource->Unlock();

    ++itSource;
    ++itTarget;
  }

  return pVertexBuffer;

}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CVertexBuffer::Display( DWORD dwIndex, DWORD dwCount )
{

  if ( ( !m_pViewer->m_pd3dDevice )
  ||   ( !m_pViewer->m_bReady ) )
  {
    return false;
  }

  m_pViewer->SetVertexShader( m_dwFVF );

  DWORD     dwPrimitives = m_dwPrimitiveCount,
            dwPrimCopied = 0;

  if ( dwCount == 0 )
  {
    dwCount = m_dwPrimitiveCount;
  }

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    IDirect3DVertexBuffer8*   iVB = *it;

    DWORD   dwPrimitivesToCopy = dwPrimitives;
    if ( dwPrimitivesToCopy > m_dwPrimitivePartCount )
    {
      dwPrimitivesToCopy = m_dwPrimitivePartCount;
    }

    if ( ( dwIndex >= 3 * dwPrimCopied )
    &&   ( dwIndex < 3 * ( dwPrimCopied + dwPrimitivesToCopy ) ) )
    {
      DWORD   dwCopyNow = dwCount;
      if ( dwCopyNow > dwPrimitivesToCopy )
      {
        dwCopyNow = dwPrimitivesToCopy;
      }
      m_pViewer->m_pd3dDevice->SetStreamSource( 0, iVB, m_dwPrimitiveSize );

      m_pViewer->m_pd3dDevice->DrawPrimitive( m_d3dPrimType,
                                              dwIndex - dwPrimCopied,
                                              dwCopyNow );

      dwCount -= dwCopyNow;
      dwPrimCopied += dwCopyNow;
    }

    if ( dwCount == 0 )
    {
      break;
    }

    ++it;
  }

  return true;

}



/*-SetData--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CVertexBuffer::SetData( void* pData )
{

  memcpy( m_pData, pData, m_dwCompleteBufferSize );

  UpdateData();
  return true;

}



