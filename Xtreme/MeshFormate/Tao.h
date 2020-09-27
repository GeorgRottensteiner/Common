#ifndef MESH_OBJECT_LOADER_H
#define MESH_OBJECT_LOADER_H



#include <GR/GRTypes.h>

#include <IO/FileStream.h>
#include <IO/FileChunk.h>

#include <Xtreme/XMesh.h>

#include <debug/debugclient.h>



namespace TAO
{
  namespace CHUNK
  {
    const DWORD INVALID               = 0;
    const DWORD MAGIC_NUMBER          = 0xABCD;
    const DWORD OBJECT_HEAD           = 0xA000;
    const DWORD OBJECT_POSITION       = 0xA100;
    const DWORD VERTEX_LIST           = 0xA200;
    const DWORD FACE_LIST             = 0xA300;
    const DWORD MATERIAL_LIST         = 0xA400;
    const DWORD TEXTURE_LIST          = 0xA500;
    const DWORD FRAME_HEAD            = 0xA600;
    const DWORD JOINT_LIST            = 0xA700;
    const DWORD FACE_ALPHA            = 0xF000;
    const DWORD FACE_TEXTURE          = 0xF001;
    const DWORD FACE_DIFFUSE_RGB      = 0xF002;
    const DWORD FACE_SPECULAR_RGB     = 0xF003;
    const DWORD FACE_NORMALS          = 0xF004;
    const DWORD FACE_MATERIAL         = 0xF005;
    const DWORD FACE_TEXTURE_UV       = 0xF006;
    const DWORD END_OF_FILE           = 0xFFFF;
  };
};


class CMeshObjectLoader
{

  public:


    static XMesh*   LoadTaoObject( const char* szFileName )
    {


      GR::IO::FileStream    aFile;

      if ( !aFile.Open( szFileName, IIOStream::OT_READ_ONLY ) )
      {
        return NULL;
      }

      WORD    wMagic = aFile.ReadU16();

      if ( wMagic != TAO::CHUNK::MAGIC_NUMBER )
      {
        aFile.Close();
        dh::Log( "Loaded %s aborted (a)", szFileName );
        return NULL;
      }

      aFile.ReadU32();    // sollte die Dateigröße sein, überlesen

      if ( aFile.ReadU32() != 2 )
      {
        aFile.Close();
        dh::Log( "Loaded %s aborted (b)", szFileName );
        return NULL;
      }

      // jetzt laden
      GR::IO::FileChunk*   pChunk  = new GR::IO::FileChunk();

      XMesh*        pObject = NULL;

      bool          bError  = false;


      do
      {
        GR::u64 dwPos = aFile.GetPosition();
        if ( !pChunk->Read( aFile ) )
        {
          aFile.Close();
          delete pChunk;
          dh::Log( "Loaded %s aborted (c)", szFileName );
          return NULL;
        }

        MemoryStream   MemStream( pChunk->GetMemoryStream() );

        // Chunk aufschlüsseln
        switch ( pChunk->Type() )
        {
          case TAO::CHUNK::END_OF_FILE:
            {
            }
            break;
          case TAO::CHUNK::OBJECT_HEAD:
            {
              // ein neues Objekt
              pObject = new XMesh();
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
              DWORD   dwFrameIndex = MemStream.ReadU32();
              DWORD   dwMaxFrames = MemStream.ReadU32();

              GR::String   strFrameName;
              MemStream.ReadString( strFrameName );
            }
            break;
          case TAO::CHUNK::VERTEX_LIST:
            {
              if ( pObject == NULL )
              {
                // Vertex-List without object
                dh::Log( "Vertex-List ohne Objekt" );
                bError = true;
                break;
              }
              DWORD dwVertexCount = MemStream.ReadU32();

              for ( DWORD dwI = 0; dwI < dwVertexCount; dwI++ )
              {
                float fX = MemStream.ReadF32(),
                      fY = MemStream.ReadF32(),
                      fZ = MemStream.ReadF32();

                pObject->AddVertex( fX, fY, fZ );
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

              DWORD dwFaceCount = MemStream.ReadU32();

              for ( DWORD dwI = 0; dwI < dwFaceCount; dwI++ )
              {
                Mesh::Face   FaceDummy;

                FaceDummy.m_Vertex[0] = MemStream.ReadU32() - 1;
                FaceDummy.m_Vertex[1] = MemStream.ReadU32() - 1;
                FaceDummy.m_Vertex[2] = MemStream.ReadU32() - 1;

                FaceDummy.m_Flags = MemStream.ReadU32();

                pObject->AddFace( FaceDummy );
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

              DWORD   dwFaceNumber = MemStream.ReadU32() - 1;

              if ( dwFaceNumber >= pObject->m_Faces.size() )
              {
                // Texture with invalid face number
                OutputDebugString( "Texture invalid face\n" );
                bError = true;
                break;
              }

              pObject->m_Faces[dwFaceNumber].m_TextureX[0] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureY[0] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureX[1] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureY[1] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureX[2] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureY[2] = MemStream.ReadF32();

              GR::String   strName;

              MemStream.ReadString( strName );

              /*
              C3ditorTexture *pTex = theSettings.LoadTexture( strName.c_str() );

              C3ditorMaterial *pMat = theSettings.FindMaterial( strName.c_str() );

              pObject->m_Faces[dwFaceNumber]->m_pMaterial = pMat;
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

              DWORD   dwFaceNumber = MemStream.ReadU32() - 1;

              if ( dwFaceNumber >= pObject->m_Faces.size() )
              {
                // Texture with invalid face number
                OutputDebugString( "Texture invalid face\n" );
                bError = true;
                break;
              }

              pObject->m_Faces[dwFaceNumber].m_TextureX[0] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureY[0] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureX[1] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureY[1] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureX[2] = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber].m_TextureY[2] = MemStream.ReadF32();
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

              DWORD   dwFaceNumber = MemStream.ReadU32();

              if ( dwFaceNumber > pObject->m_Faces.size() )
              {
                // Diffuse with invalid face number
                bError = true;
                break;
              }

              pObject->m_Faces[dwFaceNumber - 1].m_DiffuseColor[0] = 0xff000000
                                                        + ( MemStream.ReadU8() << 16 )
                                                        + ( MemStream.ReadU8() <<  8 )
                                                        +   MemStream.ReadU8();
              pObject->m_Faces[dwFaceNumber - 1].m_DiffuseColor[1] = 0xff000000
                                                        + ( MemStream.ReadU8() << 16 )
                                                        + ( MemStream.ReadU8() <<  8 )
                                                        +   MemStream.ReadU8();
              pObject->m_Faces[dwFaceNumber - 1].m_DiffuseColor[2] = 0xff000000
                                                            + ( MemStream.ReadU8() << 16 )
                                                            + ( MemStream.ReadU8() <<  8 )
                                                            +   MemStream.ReadU8();
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

              DWORD   dwFaceNumber = MemStream.ReadU32();

              if ( dwFaceNumber > pObject->m_Faces.size() )
              {
                // Specular with invalid face number
                bError = true;
                break;
              }

              /*
              pObject->m_Faces[dwFaceNumber - 1].m_dwSpecularColor[0] = 0xff000000
                                                            + ( MemStream.ReadU8() << 16 )
                                                            + ( MemStream.ReadU8() <<  8 )
                                                            +   MemStream.ReadU8();
              pObject->m_Faces[dwFaceNumber - 1].m_dwSpecularColor[1] = 0xff000000
                                                            + ( MemStream.ReadU8() << 16 )
                                                            + ( MemStream.ReadU8() <<  8 )
                                                            +   MemStream.ReadU8();
              pObject->m_Faces[dwFaceNumber - 1].m_dwSpecularColor[2] = 0xff000000
                                                            + ( MemStream.ReadU8() << 16 )
                                                            + ( MemStream.ReadU8() <<  8 )
                                                            +   MemStream.ReadU8();
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

              DWORD   dwFaceNumber = MemStream.ReadU32();

              if ( dwFaceNumber > pObject->m_Faces.size() )
              {
                // Normals with invalid face number
                bError = true;
                break;
              }

              pObject->m_Faces[dwFaceNumber - 1].m_Normal[0].x = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber - 1].m_Normal[0].y = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber - 1].m_Normal[0].z = MemStream.ReadF32();

              pObject->m_Faces[dwFaceNumber - 1].m_Normal[1].x = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber - 1].m_Normal[1].y = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber - 1].m_Normal[1].z = MemStream.ReadF32();

              pObject->m_Faces[dwFaceNumber - 1].m_Normal[2].x = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber - 1].m_Normal[2].y = MemStream.ReadF32();
              pObject->m_Faces[dwFaceNumber - 1].m_Normal[2].z = MemStream.ReadF32();
            }
            break;
          case TAO::CHUNK::TEXTURE_LIST:
            {
              /*
              // Texturen
              DWORD   dwTextureCount = MemStream.ReadU32();

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
              DWORD dwMaterialCount = MemStream.ReadU32();

              for ( int i = 0; i < dwMaterialCount; i++ )
              {
                C3ditorMaterial *pMat = new C3ditorMaterial();

                pMat->m_strName = pChunk->GetString();

                DWORD   dwTextureIndex = MemStream.ReadU32();

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
          dh::Log( "Loaded %s aborted (d)", szFileName );
          return NULL;
        }


        // zur Sicherheit...
        //aFile.SetPosition( dwPos + MemStream.ReadSize() + 6, IIOStream::PT_SET );
      }
      while ( pChunk->Type() != TAO::CHUNK::END_OF_FILE );

      delete pChunk;

      return pObject;

    }

};


#endif // MESH_OBJECT_LOADER_H