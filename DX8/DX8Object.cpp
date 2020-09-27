#include <DX8\DX8Object.h>
#include <DX8\DX8Viewer.h>

#include <IO\FileStream.h>
#include <IO\FileChunk.h>

#include <Debug\debugclient.h>

#include <Misc/Misc.h>

#include <vector>

#include <d3d8.h>
#include <d3dx8.h>



void CDX8Face::CalcNormal( CDX8Frame& Frame, bool bFlip )
{
  D3DXVECTOR3       vectDummy1,
                    vectDummy2;

  vectDummy1 = Frame.m_vectVertices[m_dwVertex[0]].vectPos - Frame.m_vectVertices[m_dwVertex[1]].vectPos;
  vectDummy2 = Frame.m_vectVertices[m_dwVertex[0]].vectPos - Frame.m_vectVertices[m_dwVertex[2]].vectPos;

  D3DXVECTOR3 vResult;
  D3DXVec3Cross( &vResult, &vectDummy1, &vectDummy2 );
  D3DXVec3Normalize( &vectNormal[0], &vResult );

  if ( bFlip )
  {
    vectNormal[0] = -vectNormal[0];
  }
  vectNormal[1] = vectNormal[0];
  vectNormal[2] = vectNormal[0];
}



void CDX8Frame::CalculateNormals( CDX8Object* pObject )
{

  if ( pObject == NULL )
  {
    return;
  }
  tVectFaces::iterator    it( pObject->m_vectFaces.begin() );
  while ( it != pObject->m_vectFaces.end() )
  {
    CDX8Face&   Face = *it;

    Face.CalcNormal( *this );

    ++it;
  }

}



void CDX8Frame::Scale( const float fX, const float fY, const float fZ )
{

  tVectVertices::iterator    it( m_vectVertices.begin() );
  while ( it != m_vectVertices.end() )
  {
    CDX8Vertex&   Vertex = *it;

    Vertex.vectPos.x *= fX;
    Vertex.vectPos.y *= fY;
    Vertex.vectPos.z *= fZ;

    ++it;
  }

}



void CDX8Frame::Align( const int iFlags, const math::vector3& vAlignTo )
{

  tVectVertices::iterator    it( m_vectVertices.begin() );
  while ( it != m_vectVertices.end() )
  {
    CDX8Vertex&   Vertex = *it;

    if ( iFlags & 4 )
    {
      Vertex.vectPos.z -= m_BoundingBox.UpperLeftCorner().z + vAlignTo.z;
    }

    ++it;
  }

}



CDX8Object::~CDX8Object()
{

  m_vectFaces.clear();
  m_mapFrames.clear();

}



CDX8Object *LoadObject( char *szName, BOOL bLoadTextures )
{

  CDX8Object    *pObject = NULL;

  GR::IO::FileStream*    pFile;

  DWORD         dwDummy,
                dwChunkSize,
                dwFilePos,
                dwI;

  float         fPos[3];

  WORD          wChunk;


  pFile = new GR::IO::FileStream( szName  );
  if ( !pFile->IsGood() )
  {
    delete pFile;
    dh::Log( "File %s not found", szName );
    return NULL;
  }

  CDX8Face  *pFace;

  CDX8Frame*   pFrame = NULL;

  DWORD     dwActFace;


  pObject = new CDX8Object();

  wChunk = pFile->ReadU16();
  if ( wChunk == TAO::CHUNK::MAGIC_NUMBER )
  {
    dwChunkSize = pFile->ReadU32();
    dwDummy = pFile->ReadU32();   // Version
    dwFilePos = 10;

    // jetzt alle Chunks abarbeiten
    do
    {
      wChunk = pFile->ReadU16();
      dwChunkSize = (DWORD)pFile->ReadU32();
      dwFilePos += 6;
      if ( wChunk == TAO::CHUNK::OBJECT_HEAD )
      {
      }
      else if ( wChunk == TAO::CHUNK::OBJECT_POSITION )
      {
        pFile->ReadBlock( (BYTE*)&fPos[0], 4 );
        pFile->ReadBlock( (BYTE*)&fPos[1], 4 );
        pFile->ReadBlock( (BYTE*)&fPos[2], 4 );
        //pObject->SetOffset( fPos[0], fPos[1], fPos[2] );
        pFile->ReadBlock( (BYTE*)&fPos[0], 4 );
        pFile->ReadBlock( (BYTE*)&fPos[1], 4 );
        pFile->ReadBlock( (BYTE*)&fPos[2], 4 );
        //pObject->SetRotation( fPos[0], fPos[1], fPos[2] );
        dwDummy = pFile->ReadU32();
        //pObject->SetFlags( dwDummy );
      }
      else if ( wChunk == TAO::CHUNK::FRAME_HEAD )
      {
        DWORD   dwFrameIndex = pFile->ReadU32(),
                dwMaxFrames = pFile->ReadU32();

        GR::String   strFrameName;

        DWORD dwLength= pFile->ReadU32();
        for ( DWORD i = 0; i < dwLength; i++ )
        {
          strFrameName += (char)pFile->ReadU8();
        }

        pObject->m_mapFrames[strFrameName] = CDX8Frame();
        pFrame = &pObject->m_mapFrames[strFrameName];
        pFrame->m_strName = strFrameName;
      }
      else if ( wChunk == TAO::CHUNK::VERTEX_LIST )
      {
        dwDummy = pFile->ReadU32();
        for ( dwI = 0; dwI < dwDummy; dwI++ )
        {
          pFile->ReadBlock( (BYTE*)&fPos[0], 4 );
          pFile->ReadBlock( (BYTE*)&fPos[1], 4 );
          pFile->ReadBlock( (BYTE*)&fPos[2], 4 );

          pFrame->m_vectVertices.push_back( CDX8Vertex( fPos[0], fPos[1], fPos[2] ) );
        }
      }
      else if ( wChunk == TAO::CHUNK::FACE_LIST )
      {
        dwDummy = pFile->ReadU32();
        for ( dwI = 0; dwI < dwDummy; dwI++ )
        {
          pObject->m_vectFaces.push_back( CDX8Face() );
          CDX8Face* pFaceDummy = &pObject->m_vectFaces.back();


          pFaceDummy->m_dwVertex[0] = (DWORD)pFile->ReadU32() - 1;
          pFaceDummy->m_dwVertex[1] = (DWORD)pFile->ReadU32() - 1;
          pFaceDummy->m_dwVertex[2] = (DWORD)pFile->ReadU32() - 1;

          pFaceDummy->m_dwFlags = pFile->ReadU32();
        }
      }
      // TU, TV
      else if ( wChunk == TAO::CHUNK::FACE_TEXTURE_UV )
      {
        dwActFace = pFile->ReadU32();

        pFace = &pObject->m_vectFaces[dwActFace - 1];
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureX[0], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureY[0], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureX[1], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureY[1], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureX[2], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureY[2], 4 );
      }
      else if ( wChunk == TAO::CHUNK::FACE_TEXTURE )
      {
        char    hs[MAX_PATH];
        dwActFace = pFile->ReadU32();

        pFace = &pObject->m_vectFaces[dwActFace - 1];
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureX[0], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureY[0], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureX[1], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureY[1], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureX[2], 4 );
        pFile->ReadBlock( (BYTE*)&pFace->m_fTextureY[2], 4 );

        // Texture-Name
        dwDummy = pFile->ReadU32();
        pFile->ReadBlock( (BYTE*)hs, dwDummy );

        /*
        C3ditorTexture *pTex = theSettings.LoadTexture( hs );
        if ( pTex != NULL )
        {
          pFace->m_pTexture = pTex->m_pTexture;
        }
        else
          */
        {
          //pFace->m_pTexture = NULL;
          //LogText( "loadtexture (%s) failed", hs );
        }
      }
      else if ( wChunk == TAO::CHUNK::FACE_DIFFUSE_RGB )
      {
        BYTE      bColor[3];

        dwDummy = pFile->ReadU32();

        pFace = &pObject->m_vectFaces[dwDummy - 1];

        bColor[0] = pFile->ReadU8();
        bColor[1] = pFile->ReadU8();
        bColor[2] = pFile->ReadU8();
        pFace->m_dwDiffuseColor[0] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

        bColor[0] = pFile->ReadU8();
        bColor[1] = pFile->ReadU8();
        bColor[2] = pFile->ReadU8();
        pFace->m_dwDiffuseColor[1] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

        bColor[0] = pFile->ReadU8();
        bColor[1] = pFile->ReadU8();
        bColor[2] = pFile->ReadU8();
        pFace->m_dwDiffuseColor[2] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];
      }
      else if ( wChunk == TAO::CHUNK::FACE_SPECULAR_RGB )
      {
        /*
        BYTE      bColor[3];

        dwDummy = pFile->ReadU32();

        pFace = pObject->m_vectFaces[dwDummy - 1];

        bColor[0] = pFile->ReadU8();
        bColor[1] = pFile->ReadU8();
        bColor[2] = pFile->ReadU8();
        pFace->m_dwSpecularColor[0] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

        bColor[0] = pFile->ReadU8();
        bColor[1] = pFile->ReadU8();
        bColor[2] = pFile->ReadU8();
        pFace->m_dwSpecularColor[1] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

        bColor[0] = pFile->ReadU8();
        bColor[1] = pFile->ReadU8();
        bColor[2] = pFile->ReadU8();
        pFace->m_dwSpecularColor[2] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];
        */
      }
      else if ( wChunk == TAO::CHUNK::FACE_NORMALS )
      {
        dwActFace = pFile->ReadU32();

        pFace = &pObject->m_vectFaces[dwActFace - 1];
        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[0].x, 4 );
        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[0].y, 4 );
        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[0].z, 4 );

        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[1].x, 4 );
        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[1].y, 4 );
        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[1].z, 4 );

        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[2].x, 4 );
        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[2].y, 4 );
        pFile->ReadBlock( (BYTE*)&pFace->vectNormal[2].z, 4 );
      }
      // ans Ende des Chunks setzen
      dwFilePos += dwChunkSize;
      pFile->SetPosition( dwFilePos, IIOStream::PT_SET );
    }
    while ( wChunk != TAO::CHUNK::END_OF_FILE );
  }
  pFile->Close();
  delete pFile;

  return pObject;

}



/*-CalculateBoundingBox-------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Object::CalculateBoundingBox()
{

  std::map<GR::String,CDX8Frame>::iterator   it( m_mapFrames.begin() );
  while ( it != m_mapFrames.end() )
  {
    CDX8Frame&    Frame = it->second;

    Frame.m_BoundingBox.Clear();

    std::vector<CDX8Vertex>::iterator   itVertex( Frame.m_vectVertices.begin() );
    while ( itVertex != Frame.m_vectVertices.end() )
    {
      CDX8Vertex&   Vertex = *itVertex;

      Frame.m_BoundingBox.ErweitereUmVertex( (math::vector3)Vertex.vectPos );

      ++itVertex;
    }


    ++it;
  }

}



/*-CreateVertexBuffer---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CDX8Object::CreateVertexBuffers( CD3DViewer& aViewer, DWORD dwFVF )
{

  if ( m_vectFaces.empty() )
  {
    dh::Log( "CDX8Object::CreateVertexBuffer no Faces\n" );
    return FALSE;
  }

  DWORD   dwVertexSize = 0;

  int     iOffsetXYZ = -1,
          iOffsetRHW = -1,
          iOffsetNormal = -1,
          iOffsetDiffuse = -1,
          iOffsetSpecular = -1,
          iOffsetTexCoord = -1;


  if ( dwFVF & D3DFVF_XYZ )
  {
    iOffsetXYZ = dwVertexSize;
    dwVertexSize += 3 * sizeof( float );
  }
  else if ( dwFVF & D3DFVF_XYZRHW )
  {
    iOffsetXYZ = dwVertexSize;
    dwVertexSize += 3 * sizeof( float );
    iOffsetRHW = dwVertexSize;
    dwVertexSize += sizeof( float );
  }
  else if ( ( dwFVF & D3DFVF_XYZB1 )
  ||        ( dwFVF & D3DFVF_XYZB2 )
  ||        ( dwFVF & D3DFVF_XYZB3 )
  ||        ( dwFVF & D3DFVF_XYZB4 )
  ||        ( dwFVF & D3DFVF_XYZB5 ) )
  {
    dh::Log( "CDX8Object::CreateVertexBuffer Blend Weights not supported yet.\n" );
  }
  if ( dwFVF & D3DFVF_NORMAL )
  {
    iOffsetNormal = dwVertexSize;
    dwVertexSize += 3 * sizeof( float );
  }
  if ( dwFVF & D3DFVF_PSIZE )
  {
    dh::Log( "CDX8Object::CreateVertexBuffer Point Sizes not supported yet.\n" );
  }
  if ( dwFVF & D3DFVF_DIFFUSE )
  {
    iOffsetDiffuse = dwVertexSize;
    dwVertexSize += sizeof( DWORD );
  }
  if ( dwFVF & D3DFVF_SPECULAR )
  {
    iOffsetSpecular = dwVertexSize;
    dwVertexSize += sizeof( DWORD );
  }
  // BAUSTELLE - Texture Koordinaten sollten die dwFVF-Flags richtig parsen
  if ( dwFVF & D3DFVF_TEX1 )
  {
    iOffsetTexCoord = dwVertexSize;
    dwVertexSize += 2 * sizeof( float );
  }

  if ( dwVertexSize == 0 )
  {
    dh::Log( "CDX8Object::CreateVertexBuffer Created without content!\n" );
    return FALSE;
  }

  BYTE*       pVertexCache;


  std::map<GR::String,CDX8Frame>::iterator    it( m_mapFrames.begin() );
  while ( it != m_mapFrames.end() )
  {
    CDX8Frame*  pFrame = &it->second;

    CVertexBuffer* pBuffer = aViewer.CreateVertexBuffer( (DWORD)m_vectFaces.size(),
                                       dwVertexSize,
                                       (DWORD)( m_vectFaces.size() * dwVertexSize * 3 ),
                                       D3DPT_TRIANGLELIST,
                                       D3DUSAGE_WRITEONLY,//D3DUSAGE_DYNAMIC,
                                       dwFVF,
                                       D3DPOOL_DEFAULT );

    pVertexCache = (BYTE*)pBuffer->m_pData;

    BYTE*       pVertexCacheStart = pVertexCache;

    for ( size_t i = 0; i < m_vectFaces.size(); i++ )
    {
      for ( int j = 0; j < 3; j++ )
      {
        if ( iOffsetXYZ != -1 )
        {
          *(D3DXVECTOR3*)( pVertexCache + iOffsetXYZ ) = pFrame->m_vectVertices[m_vectFaces[i].m_dwVertex[j]].vectPos;
        }
        if ( iOffsetNormal != -1 )
        {
          *(D3DXVECTOR3*)( pVertexCache + iOffsetNormal ) = m_vectFaces[i].vectNormal[j];
        }
        if ( iOffsetDiffuse != -1 )
        {
          *(DWORD*)( pVertexCache + iOffsetDiffuse ) = m_vectFaces[i].m_dwDiffuseColor[j];
        }
        if ( iOffsetSpecular != -1 )
        {
          *(DWORD*)( pVertexCache + iOffsetSpecular ) = 0xffffffff;//m_vectFaces[i].m_dwSpecularColor[j];
        }
        if ( iOffsetTexCoord != -1 )
        {
          (*(D3DXVECTOR2*)( pVertexCache + iOffsetTexCoord ) ).x = m_vectFaces[i].m_fTextureX[j];
          (*(D3DXVECTOR2*)( pVertexCache + iOffsetTexCoord ) ).y = m_vectFaces[i].m_fTextureY[j];
        }
        pVertexCache += dwVertexSize;
      }
    }

    pBuffer->UpdateData();

    pFrame->m_pVertexBuffer = pBuffer;

    it++;
  }

  return TRUE;

}



void CDX8Object::CenterAbout( const math::vector3& vCenter )
{

  D3DXVECTOR3   vMin( FLT_MAX, FLT_MAX, FLT_MAX );
  D3DXVECTOR3   vMax( FLT_MIN, FLT_MIN, FLT_MIN );

  CalculateBoundingBox();

  std::map<GR::String,CDX8Frame>::iterator   it( m_mapFrames.begin() );
  while ( it != m_mapFrames.end() )
  {
    CDX8Frame&    Frame = it->second;

    D3DXVECTOR3     vectOffset = vCenter - ( Frame.m_BoundingBox.UpperLeftCorner() + Frame.m_BoundingBox.LowerRightCorner() ) * 0.5f;

    tVectVertices::iterator   itV( Frame.m_vectVertices.begin() );
    while ( itV != Frame.m_vectVertices.end() )
    {
      CDX8Vertex&   Vertex = *itV;

      Vertex.vectPos += vectOffset;

      ++itV;
    }

    ++it;
  }

}



CDX8Object* LoadDX8Object( const char* szFileName )
{
  GR::IO::FileStream    aFile;

  if ( !aFile.Open( szFileName ) )
  {
    return NULL;
  }

  WORD    wMagic = aFile.ReadU16();

  if ( wMagic != TAO::CHUNK::MAGIC_NUMBER )
  {
    aFile.Close();
    OutputDebugString( CMisc::printf( "Loaded %s aborted (a)", szFileName ) );
    return NULL;
  }

  aFile.ReadU32();    // sollte die Dateigröße sein, überlesen

  if ( aFile.ReadU32() != 2 )
  {
    aFile.Close();
    OutputDebugString( CMisc::printf( "Loaded %s aborted (b)", szFileName ) );
    return NULL;
  }

  // jetzt laden
  GR::IO::FileChunk*   pChunk  = new GR::IO::FileChunk();

  CDX8Object*   pObject = NULL;

  CDX8Frame*    pFrame  = NULL;

  bool          bError  = false;


  do
  {
    DWORD dwPos = (DWORD)aFile.GetPosition();
    if ( !pChunk->Read( aFile ) )
    {
      aFile.Close();
      delete pChunk;
      OutputDebugString( CMisc::printf( "Loaded %s aborted (c)", szFileName ) );
      return NULL;
    }

    // Chunk aufschlüsseln
    MemoryStream    memChunk( pChunk->GetMemoryStream() );
    switch ( pChunk->Type() )
    {
      case TAO::CHUNK::END_OF_FILE:
        {
        }
        break;
      case TAO::CHUNK::OBJECT_HEAD:
        {
          // ein neues Objekt
          pObject = new CDX8Object();
        }
        break;
      case TAO::CHUNK::INVALID:
        {
          bError = true;
        }
        break;
      case TAO::CHUNK::FRAME_HEAD:
        {
          if ( pObject == NULL )
          {
            // Frame ohne Objekt!
            OutputDebugString( "Frame ohne Objekt\n" );
            bError = true;
            break;
          }
          DWORD   dwFrameIndex = memChunk.ReadU32(),
                  dwMaxFrames = memChunk.ReadU32();

          GR::String   strFrameName = memChunk.ReadString();

          pObject->m_mapFrames[strFrameName] = CDX8Frame();
          pFrame = &pObject->m_mapFrames[strFrameName];

          pFrame->m_strName = strFrameName;
        }
        break;
      case TAO::CHUNK::JOINT_LIST:
        {
          if ( pObject == NULL )
          {
            // Joint-List without object
            OutputDebugString( "Joint-List ohne Objekt\n" );
            bError = true;
            break;
          }
          if ( pFrame == NULL )
          {
            // Joint-List without frame
            OutputDebugString( "Joint-List ohne Frame\n" );
            bError = true;
            break;
          }
          DWORD   dwCount = memChunk.ReadU32();

          for ( DWORD i = 0; i < dwCount; i++ )
          {
            CDX8Joint   Joint;

            Joint.m_strName = memChunk.ReadString();
            Joint.m_vectPos.x = memChunk.ReadF32();
            Joint.m_vectPos.y = memChunk.ReadF32();
            Joint.m_vectPos.z = memChunk.ReadF32();
            Joint.m_fYaw = memChunk.ReadF32();
            Joint.m_fPitch = memChunk.ReadF32();
            Joint.m_fRoll = memChunk.ReadF32();

            pFrame->SetJoint( i, Joint );
          }
        }
        break;
      case TAO::CHUNK::VERTEX_LIST:
        {
          if ( pObject == NULL )
          {
            // Vertex-List without object
            OutputDebugString( "Vertex-List ohne Objekt\n" );
            bError = true;
            break;
          }
          if ( pFrame == NULL )
          {
            // eigentlich ein Fehler
            OutputDebugString( "Inserting dummy frame\n" );
            pObject->m_mapFrames["AltesFileFrame"] = CDX8Frame();
            pFrame = &pObject->m_mapFrames["AltesFileFrame"];

            pFrame->m_strName = "AltesFileFrame";

          }

          DWORD dwVertexCount = memChunk.ReadU32();

          for ( DWORD dwI = 0; dwI < dwVertexCount; dwI++ )
          {
            float fX = memChunk.ReadF32(),
                  fY = memChunk.ReadF32(),
                  fZ = memChunk.ReadF32();

            while ( pFrame->m_vectVertices.size() <= dwI )
            {
              pFrame->m_vectVertices.push_back( CDX8Vertex() );
            }
            pFrame->m_vectVertices[dwI].vectPos.x = fX;
            pFrame->m_vectVertices[dwI].vectPos.y = fY;
            pFrame->m_vectVertices[dwI].vectPos.z = fZ;
          }
        }
        break;
      case TAO::CHUNK::FACE_LIST:
        {
          if ( pObject == NULL )
          {
            // Face-List without object
            OutputDebugString( "Face-List ohne Objekt\n" );
            bError = true;
            break;
          }
          if ( pFrame == NULL )
          {
            // Face-List without Frame
            OutputDebugString( "Face-List ohne Frame\n" );
            bError = true;
            break;
          }

          DWORD dwFaceCount = memChunk.ReadU32();

          for ( DWORD dwI = 0; dwI < dwFaceCount; dwI++ )
          {
            CDX8Face     FaceDummy;

            FaceDummy.m_dwVertex[0] = memChunk.ReadU32() - 1;
            FaceDummy.m_dwVertex[1] = memChunk.ReadU32() - 1;
            FaceDummy.m_dwVertex[2] = memChunk.ReadU32() - 1;

            FaceDummy.m_dwFlags = memChunk.ReadU32();

            pObject->m_vectFaces.push_back( FaceDummy );
          }
        }
        break;
      case TAO::CHUNK::FACE_TEXTURE:
        {
          if ( pObject == NULL )
          {
            // Texture without object
            OutputDebugString( "Texture ohne Objekt\n" );
            bError = true;
            break;
          }

          DWORD   dwFaceNumber = memChunk.ReadU32() - 1;

          if ( dwFaceNumber >= pObject->m_vectFaces.size() )
          {
            // Texture with invalid face number
            OutputDebugString( "Texture invalid face\n" );
            bError = true;
            break;
          }

          pObject->m_vectFaces[dwFaceNumber].m_fTextureX[0] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureY[0] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureX[1] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureY[1] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureX[2] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureY[2] = memChunk.ReadF32();

          GR::String   strName = memChunk.ReadString();

          /*
          C3ditorTexture *pTex = theSettings.LoadTexture( strName.c_str() );

          C3ditorMaterial *pMat = theSettings.FindMaterial( strName.c_str() );

          pObject->m_vectFaces[dwFaceNumber]->m_pMaterial = pMat;
          */
        }
        break;
      case TAO::CHUNK::FACE_TEXTURE_UV:
        {
          if ( pObject == NULL )
          {
            // Texture without object
            OutputDebugString( "Texture ohne Objekt\n" );
            bError = true;
            break;
          }

          DWORD   dwFaceNumber = memChunk.ReadU32() - 1;

          if ( dwFaceNumber >= pObject->m_vectFaces.size() )
          {
            // Texture with invalid face number
            OutputDebugString( "Texture invalid face\n" );
            bError = true;
            break;
          }

          pObject->m_vectFaces[dwFaceNumber].m_fTextureX[0] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureY[0] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureX[1] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureY[1] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureX[2] = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber].m_fTextureY[2] = memChunk.ReadF32();
        }
        break;
      case TAO::CHUNK::FACE_DIFFUSE_RGB:
        {
          if ( pObject == NULL )
          {
            // Diffuse without object
            OutputDebugString( "Diffuse ohne Objekt\n" );
            bError = true;
            break;
          }

          DWORD   dwFaceNumber = memChunk.ReadU32();

          if ( dwFaceNumber > pObject->m_vectFaces.size() )
          {
            // Diffuse with invalid face number
            bError = true;
            break;
          }

          pObject->m_vectFaces[dwFaceNumber - 1].m_dwDiffuseColor[0] = 0xff000000
                                                         + ( memChunk.ReadU8() << 16 )
                                                         + ( memChunk.ReadU8() <<  8 )
                                                         +   memChunk.ReadU8();
          pObject->m_vectFaces[dwFaceNumber - 1].m_dwDiffuseColor[1] = 0xff000000
                                                         + ( memChunk.ReadU8() << 16 )
                                                         + ( memChunk.ReadU8() <<  8 )
                                                         +   memChunk.ReadU8();
          pObject->m_vectFaces[dwFaceNumber - 1].m_dwDiffuseColor[2] = 0xff000000
                                                         + ( memChunk.ReadU8() << 16 )
                                                         + ( memChunk.ReadU8() <<  8 )
                                                         +   memChunk.ReadU8();
        }
        break;
      case TAO::CHUNK::FACE_SPECULAR_RGB:
        {
          if ( pObject == NULL )
          {
            // Specular without object
            bError = true;
            break;
          }

          DWORD   dwFaceNumber = memChunk.ReadU32();

          if ( dwFaceNumber > pObject->m_vectFaces.size() )
          {
            // Specular with invalid face number
            bError = true;
            break;
          }

          /*
          pObject->m_vectFaces[dwFaceNumber - 1].m_dwSpecularColor[0] = 0xff000000
                                                         + ( pChunk->GetByte() << 16 )
                                                         + ( pChunk->GetByte() <<  8 )
                                                         +   pChunk->GetByte();
          pObject->m_vectFaces[dwFaceNumber - 1].m_dwSpecularColor[1] = 0xff000000
                                                         + ( pChunk->GetByte() << 16 )
                                                         + ( pChunk->GetByte() <<  8 )
                                                         +   pChunk->GetByte();
          pObject->m_vectFaces[dwFaceNumber - 1].m_dwSpecularColor[2] = 0xff000000
                                                         + ( pChunk->GetByte() << 16 )
                                                         + ( pChunk->GetByte() <<  8 )
                                                         +   pChunk->GetByte();
          */
        }
        break;
      case TAO::CHUNK::FACE_NORMALS:
        {
          if ( pObject == NULL )
          {
            // Normals without object
            bError = true;
            break;
          }

          DWORD   dwFaceNumber = memChunk.ReadU32();

          if ( dwFaceNumber > pObject->m_vectFaces.size() )
          {
            // Normals with invalid face number
            bError = true;
            break;
          }

          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[0].x = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[0].y = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[0].z = memChunk.ReadF32();

          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[1].x = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[1].y = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[1].z = memChunk.ReadF32();

          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[2].x = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[2].y = memChunk.ReadF32();
          pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[2].z = memChunk.ReadF32();
        }
        break;
      case TAO::CHUNK::TEXTURE_LIST:
        {
          /*
          // Texturen
          DWORD   dwTextureCount = pChunk->GetU32();

          for ( int i = 0; i < dwTextureCount; i++ )
          {
            GR::String     strLine = pChunk->GetString();

            vectTextures.push_back( theSettings.LoadTexture( strLine.c_str() ) );
          }
          */
        }
        break;
      case TAO::CHUNK::MATERIAL_LIST:
        {
          /*
          // Material-Liste
          DWORD dwMaterialCount = pChunk->GetU32();

          for ( int i = 0; i < dwMaterialCount; i++ )
          {
            C3ditorMaterial *pMat = new C3ditorMaterial();

            pMat->m_strName = pChunk->GetString();

            DWORD   dwTextureIndex = pChunk->GetU32();

            if ( dwTextureIndex > 0 )
            {
              // Texturindex raussuchen
              tVectTextures::iterator   it( vectTextures.begin() );
              while ( it != vectTextures.end() )
              {
                dwTextureIndex--;
                if ( dwTextureIndex == 0 )
                {
                  // das ist die Textur
                  pMat->m_pTexture = *it;
                  break;
                }
                it++;
              }
            }

            pChunk->GetBlock( &pMat->m_d3d8mat );

            theSettings.AddMaterial( pMat );
          }
          */
        }
        break;
     default:
        {
          //TRACE( "unprocessed Chunk read %0x (%d)\n", pChunk->GetType(), pChunk->GetSize() );
          //bError = true;
        }
        break;
    }

    if ( bError )
    {
      aFile.Close();
      delete pChunk;
      OutputDebugString( CMisc::printf( "Loaded %s aborted (d)", szFileName ) );
      return NULL;
    }


    // zur Sicherheit...
    aFile.SetPosition( dwPos + pChunk->Size() + 6, IIOStream::PT_SET );
  }
  while ( pChunk->Type() != TAO::CHUNK::END_OF_FILE );

  delete pChunk;

  return pObject;
}



void CDX8Object::Clear()
{

  m_mapFrames.clear();
  m_vectFaces.clear();

}



size_t CDX8Object::AddVertex( float fX, float fY, float fZ )
{

  size_t   dwResult = 0;

  tMapFrames::iterator  itPos( m_mapFrames.begin() );
  while ( itPos != m_mapFrames.end() )
  {
    dwResult = itPos->second.AddVertex( CDX8Vertex( fX, fY, fZ ) );

    itPos++;
  }

  return dwResult;

}



void CDX8Object::RemoveVertex( int iNr )
{

  tMapFrames::iterator  itPos( m_mapFrames.begin() );
  while ( itPos != m_mapFrames.end() )
  {
    int   iDummy = iNr;

    CDX8Frame&    Frame = itPos->second;

    tVectVertices::iterator itV( Frame.m_vectVertices.begin() );
    while ( itV != Frame.m_vectVertices.end() )
    {
      if ( iDummy == 0 )
      {
        Frame.m_vectVertices.erase( itV );
        break;
      }
      iDummy--;

      ++itV;
    }
    itPos++;
  }

}



size_t CDX8Object::AddFace( CDX8Face& Face )
{

  m_vectFaces.push_back( Face );

  return m_vectFaces.size() - 1;

}



void CDX8Object::RemoveFace( CDX8Face* pFace )
{

  if ( pFace == NULL )
  {
    return;
  }
  tVectFaces::iterator    it( m_vectFaces.begin() );
  while ( it != m_vectFaces.end() )
  {
    if ( &(*it) == pFace )
    {
      m_vectFaces.erase( it );
      return;
    }

    ++it;
  }

}



/*-AddFrame-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Frame* CDX8Object::AddFrame( const char *szName, CDX8Frame& Frame )
{

  if ( szName )
  {
  Frame.m_strName = szName;
  }
  if ( !m_mapFrames.empty() )
  {
    // es gibt schon Frames, den neuen Frame mit Vertices auffüllen
    CDX8Frame& Frame2 = m_mapFrames.begin()->second;
    if ( Frame2.m_vectVertices.size() > Frame.m_vectVertices.size() )
    {
      for ( size_t i = Frame.m_vectVertices.size(); i < Frame2.m_vectVertices.size(); i++ )
      {
        Frame.AddVertex( Frame2.m_vectVertices[i] );
      }
    }
    // dito Joints
    if ( Frame2.m_vectJoints.size() > Frame.m_vectJoints.size() )
    {
      for ( size_t i = Frame.m_vectJoints.size(); i < Frame2.m_vectJoints.size(); i++ )
      {
        Frame.AddJoint( Frame2.m_vectJoints[i] );
      }
    }
  }
  m_mapFrames[Frame.m_strName] = Frame;

  return &m_mapFrames[Frame.m_strName];

}



/*-GetFrame-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Frame* CDX8Object::GetFrame( const char* szName )
{

  if ( m_mapFrames.empty() )
  {
    return NULL;
  }
  if ( szName == NULL )
  {
    return &m_mapFrames.begin()->second;
  }

  tMapFrames::iterator    it( m_mapFrames.find( szName ) );
  if ( it == m_mapFrames.end() )
  {
    return NULL;
  }
  return &it->second;

}



/*-AddJoint-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CDX8Object::AddJoint( float fX, float fY, float fZ, const char* szName )
{

  size_t   dwResult = 0;

  tMapFrames::iterator  itPos( m_mapFrames.begin() );
  while ( itPos != m_mapFrames.end() )
  {
    CDX8Joint   Joint( fX, fY, fZ );

    Joint.m_strName = szName;
    dwResult = itPos->second.AddJoint( Joint );

    itPos++;
  }

  return dwResult;

}



/*-RemoveJoint----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Object::RemoveJoint( CDX8Joint* pJoint )
{

  tMapFrames::iterator  itPos( m_mapFrames.begin() );
  while ( itPos != m_mapFrames.end() )
  {
    CDX8Frame*   pFrame = &itPos->second;

    pFrame->RemoveJoint( pJoint );
    itPos++;
  }

}



