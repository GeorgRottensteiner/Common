#ifndef MESH_T3D_LOADER_H
#define MESH_T3D_LOADER_H



#include <GR/GRTypes.h>

#include <IO/FileStream.h>
#include <IO/FileChunk.h>

#include <Xtreme/XMesh.h>

#include <debug/debugclient.h>



namespace T3D
{
  namespace CHUNK
  {
    const GR::u32 INVALID               = 0;
    const GR::u32 MAGIC_NUMBER          = 0xABCD;
    const GR::u32 OBJECT_HEAD           = 0xA000;
    const GR::u32 OBJECT_POSITION       = 0xA100;
    const GR::u32 VERTEX_LIST           = 0xA200;
    const GR::u32 FACE_LIST             = 0xA300;
    const GR::u32 MATERIAL_LIST         = 0xA400;
    const GR::u32 TEXTURE_LIST          = 0xA500;
    const GR::u32 FRAME_HEAD            = 0xA600;
    const GR::u32 JOINT_LIST            = 0xA700;
    const GR::u32 FACE_ALPHA            = 0xF000;
    const GR::u32 FACE_TEXTURE          = 0xF001;
    const GR::u32 FACE_DIFFUSE_RGB      = 0xF002;
    const GR::u32 FACE_SPECULAR_RGB     = 0xF003;
    const GR::u32 FACE_NORMALS          = 0xF004;
    const GR::u32 FACE_MATERIAL         = 0xF005;
    const GR::u32 FACE_TEXTURE_UV       = 0xF006;
    const GR::u32 FACE_DIFFUSE_ARGB     = 0xF012;
    const GR::u32 FACE_SPECULAR_ARGB    = 0xF013;
    const GR::u32 END_OF_FILE           = 0xFFFF;
  };
};


class CT3DMeshLoader
{

  public:


    static XMesh*   Load( const char* szFileName )
    {
      GR::IO::FileStream    ioIn( szFileName );
      return Load( ioIn );
    }

    static XMesh*   Load( IIOStream& fileObject )
    {

      XMesh*        pObject = NULL;

      GR::u32       dwDummy,
                    dwChunkSize,
                    dwFilePos,
                    dwI;

      float         fPos[3];

      GR::u16       wChunk;


      if ( !fileObject.IsGood() )
      {
        return NULL;
      }

      Mesh::Face*   pFace;

      GR::u32         dwActFace;


      pObject = new XMesh();

      wChunk = fileObject.ReadU16();
      if ( wChunk == T3D::CHUNK::MAGIC_NUMBER )
      {
        dwChunkSize = fileObject.ReadU32();
        dwDummy     = fileObject.ReadU32();   // Version
        dwFilePos   = 10;

        // jetzt alle Chunks abarbeiten
        do
        {
          wChunk = fileObject.ReadU16();
          dwChunkSize = (GR::u32)fileObject.ReadU32();
          dwFilePos += 6;

          if ( wChunk == T3D::CHUNK::OBJECT_HEAD )
          {
          }
          else if ( wChunk == T3D::CHUNK::OBJECT_POSITION )
          {
            fileObject.ReadBlock( (GR::u8*)&fPos[0], 4 );
            fileObject.ReadBlock( (GR::u8*)&fPos[1], 4 );
            fileObject.ReadBlock( (GR::u8*)&fPos[2], 4 );
            //pObject->SetOffset( fPos[0], fPos[1], fPos[2] );
            fileObject.ReadBlock( (GR::u8*)&fPos[0], 4 );
            fileObject.ReadBlock( (GR::u8*)&fPos[1], 4 );
            fileObject.ReadBlock( (GR::u8*)&fPos[2], 4 );
            //pObject->SetRotation( fPos[0], fPos[1], fPos[2] );
            dwDummy = fileObject.ReadU32();
            //pObject->SetFlags( dwDummy );
          }
          else if ( wChunk == T3D::CHUNK::FRAME_HEAD )
          {
            GR::u32   dwFrameIndex = fileObject.ReadU32(),
                    dwMaxFrames = fileObject.ReadU32();

            GR::String   strFrameName;

            GR::u32 dwLength= fileObject.ReadU32();
            for ( GR::u32 i = 0; i < dwLength; i++ )
            {
              strFrameName += (char)fileObject.ReadU8();
            }
          }
          else if ( wChunk == T3D::CHUNK::VERTEX_LIST )
          {
            dwDummy = fileObject.ReadU32();
            for ( dwI = 0; dwI < dwDummy; dwI++ )
            {
              fileObject.ReadBlock( (GR::u8*)&fPos[0], 4 );
              fileObject.ReadBlock( (GR::u8*)&fPos[1], 4 );
              fileObject.ReadBlock( (GR::u8*)&fPos[2], 4 );

              pObject->AddVertex( Mesh::Vertex( fPos[0], fPos[1], fPos[2] ) );
            }
          }
          else if ( wChunk == T3D::CHUNK::FACE_LIST )
          {
            dwDummy = fileObject.ReadU32();
            for ( dwI = 0; dwI < dwDummy; dwI++ )
            {
              pObject->m_Faces.push_back( Mesh::Face() );
              Mesh::Face* pFaceDummy = &pObject->m_Faces.back();


              pFaceDummy->m_Vertex[0] = (GR::u32)fileObject.ReadU32() - 1;
              pFaceDummy->m_Vertex[1] = (GR::u32)fileObject.ReadU32() - 1;
              pFaceDummy->m_Vertex[2] = (GR::u32)fileObject.ReadU32() - 1;

              pFaceDummy->m_Flags = fileObject.ReadU32();
            }
          }
          // TU, TV
          else if ( wChunk == T3D::CHUNK::FACE_TEXTURE_UV )
          {
            dwActFace = fileObject.ReadU32();

            pFace = &pObject->m_Faces[dwActFace - 1];
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureX[0], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureY[0], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureX[1], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureY[1], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureX[2], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureY[2], 4 );
          }
          else if ( wChunk == T3D::CHUNK::FACE_TEXTURE )
          {
            dwActFace = fileObject.ReadU32();

            pFace = &pObject->m_Faces[dwActFace - 1];
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureX[0], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureY[0], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureX[1], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureY[1], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureX[2], 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_TextureY[2], 4 );

            // Texture-Name
            dwDummy = fileObject.ReadU32();

            char*   pHS = new char[dwDummy];

            fileObject.ReadBlock( (GR::u8*)pHS, dwDummy );

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
          else if ( wChunk == T3D::CHUNK::FACE_DIFFUSE_RGB )
          {
            GR::u8      bColor[3];

            dwDummy = fileObject.ReadU32();

            pFace = &pObject->m_Faces[dwDummy - 1];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_DiffuseColor[0] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_DiffuseColor[1] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_DiffuseColor[2] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];
          }
          else if ( wChunk == T3D::CHUNK::FACE_DIFFUSE_RGB )
          {
            GR::u8      bColor[3];

            dwDummy = fileObject.ReadU32();

            pFace = &pObject->m_Faces[dwDummy - 1];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_DiffuseColor[0] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_DiffuseColor[1] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_DiffuseColor[2] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];
          }
          else if ( wChunk == T3D::CHUNK::FACE_DIFFUSE_ARGB )
          {
            dwDummy = fileObject.ReadU32();

            pFace = &pObject->m_Faces[dwDummy - 1];

            pFace->m_DiffuseColor[0] = fileObject.ReadU32();
            pFace->m_DiffuseColor[1] = fileObject.ReadU32();
            pFace->m_DiffuseColor[2] = fileObject.ReadU32();
          }
          else if ( wChunk == T3D::CHUNK::FACE_SPECULAR_ARGB )
          {
            dwDummy = fileObject.ReadU32();

            pFace = &pObject->m_Faces[dwDummy - 1];

            pFace->m_SpecularColor[0] = fileObject.ReadU32();
            pFace->m_SpecularColor[1] = fileObject.ReadU32();
            pFace->m_SpecularColor[2] = fileObject.ReadU32();
          }
          else if ( wChunk == T3D::CHUNK::FACE_SPECULAR_RGB )
          {
            /*
            GR::u8      bColor[3];

            dwDummy = fileObject.ReadU32();

            pFace = pObject->m_vectFaces[dwDummy - 1];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_dwSpecularColor[0] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_dwSpecularColor[1] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];

            bColor[0] = fileObject.ReadU8();
            bColor[1] = fileObject.ReadU8();
            bColor[2] = fileObject.ReadU8();
            pFace->m_dwSpecularColor[2] = 0xff000000 + ( bColor[0] << 16 ) + ( bColor[1] << 8 ) + bColor[2];
            */
          }
          else if ( wChunk == T3D::CHUNK::FACE_NORMALS )
          {
            dwActFace = fileObject.ReadU32();

            pFace = &pObject->m_Faces[dwActFace - 1];
            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[0].x, 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[0].y, 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[0].z, 4 );

            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[1].x, 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[1].y, 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[1].z, 4 );

            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[2].x, 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[2].y, 4 );
            fileObject.ReadBlock( (GR::u8*)&pFace->m_Normal[2].z, 4 );
          }
          // ans Ende des Chunks setzen
          dwFilePos += dwChunkSize;
          fileObject.SetPosition( dwFilePos, IIOStream::PT_SET );
        }
        while ( wChunk != T3D::CHUNK::END_OF_FILE );
      }
      fileObject.Close();

      return pObject;
    }

};


#endif // MESH_OBJECT_LOADER_H