#ifndef __DX8_VERTEXBUFFER_H__
#define __DX8_VERTEXBUFFER_H__
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

#include <windows.h>
#include <d3dx8.h>
#include <vector>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CD3DViewer;

class CVertexBuffer
{

  public:

    typedef std::vector<IDirect3DVertexBuffer8*>    tVectVertexBuffers;

    tVectVertexBuffers            m_vectVertexBuffers;

    D3DPRIMITIVETYPE              m_d3dPrimType;

    DWORD                         m_dwPrimitiveCount,
                                  m_dwPrimitiveSize,
                                  m_dwPrimitivePartCount,
                                  m_dwCompleteBufferSize,

                                  m_dwUsage,
                                  m_dwFVF;

    D3DPOOL                       m_d3dPool;

    void*                         m_pData;

    CD3DViewer*                   m_pViewer;


    CVertexBuffer( CD3DViewer* pViewer )
      : m_d3dPrimType( D3DPT_TRIANGLELIST ),
        m_dwPrimitiveCount( 0 ),
        m_dwPrimitiveSize( 0 ),
        m_dwPrimitivePartCount( 0 ),
        m_dwCompleteBufferSize( 0 ),
        m_dwFVF( 0 ),
        m_pData( NULL ),
        m_pViewer( pViewer )
    {
    }

    virtual ~CVertexBuffer();

    virtual bool                  CreateBuffers( LPDIRECT3DDEVICE8 pd3dDevice );

    virtual void                  Invalidate();

    virtual BOOL                  Restore( LPDIRECT3DDEVICE8 pd3dDevice );

    // kopiert die Daten aus dem gespeicherten Puffer in die Karte
    virtual void                  UpdateData();

    bool                          SetData( void* pData );

    CVertexBuffer*                Clone();

    bool                          Display( DWORD dwIndex = 0, DWORD dwCount = 0 );


};


#endif