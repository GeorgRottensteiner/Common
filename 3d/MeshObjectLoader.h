#ifndef MESH_OBJECT_LOADER_H
#define MESH_OBJECT_LOADER_H



#include <IO/FileStream.h>
#include <IO/FileChunk.h>

#include "MeshObject.h"



class CMeshObjectLoader
{

  public:


    static CMesh*   LoadDX8Object( const char* szFileName )
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

      CMesh*        pObject = NULL;

      bool          bError  = false;


      do
      {
        DWORD dwPos = (DWORD)aFile.GetPosition();
        if ( !pChunk->Read( aFile ) )
        {
          aFile.Close();
          delete pChunk;
          dh::Log( "Loaded %s aborted (c)", szFileName );
          return NULL;
        }

        MemoryStream    memIn( pChunk->GetMemoryStream() );

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
              pObject = new CMesh();
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
              DWORD   dwFrameIndex = memIn.ReadU32(),
                      dwMaxFrames = memIn.ReadU32();

              GR::String   strFrameName = memIn.ReadString();

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
              DWORD dwVertexCount = memIn.ReadU32();

              for ( DWORD dwI = 0; dwI < dwVertexCount; dwI++ )
              {
                float fX = memIn.ReadF32(),
                      fY = memIn.ReadF32(),
                      fZ = memIn.ReadF32();

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

              DWORD dwFaceCount = memIn.ReadU32();

              for ( DWORD dwI = 0; dwI < dwFaceCount; dwI++ )
              {
                CFace     FaceDummy;

                FaceDummy.m_dwVertex[0] = memIn.ReadU32() - 1;
                FaceDummy.m_dwVertex[1] = memIn.ReadU32() - 1;
                FaceDummy.m_dwVertex[2] = memIn.ReadU32() - 1;

                FaceDummy.m_dwFlags = memIn.ReadU32();

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

              DWORD   dwFaceNumber = memIn.ReadU32() - 1;

              if ( dwFaceNumber >= pObject->m_vectFaces.size() )
              {
                // Texture with invalid face number
                OutputDebugString( "Texture invalid face\n" );
                bError = true;
                break;
              }

              pObject->m_vectFaces[dwFaceNumber].m_fTextureX[0] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureY[0] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureX[1] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureY[1] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureX[2] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureY[2] = memIn.ReadF32();

              GR::String   strName = memIn.ReadString();

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

              DWORD   dwFaceNumber = memIn.ReadU32() - 1;

              if ( dwFaceNumber >= pObject->m_vectFaces.size() )
              {
                // Texture with invalid face number
                OutputDebugString( "Texture invalid face\n" );
                bError = true;
                break;
              }

              pObject->m_vectFaces[dwFaceNumber].m_fTextureX[0] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureY[0] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureX[1] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureY[1] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureX[2] = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber].m_fTextureY[2] = memIn.ReadF32();
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

              DWORD   dwFaceNumber = memIn.ReadU32();

              if ( dwFaceNumber > pObject->m_vectFaces.size() )
              {
                // Diffuse with invalid face number
                bError = true;
                break;
              }

              pObject->m_vectFaces[dwFaceNumber - 1].m_dwDiffuseColor[0] = 0xff000000
                                                            + ( memIn.ReadU8() << 16 )
                                                            + ( memIn.ReadU8() <<  8 )
                                                            +   memIn.ReadU8();
              pObject->m_vectFaces[dwFaceNumber - 1].m_dwDiffuseColor[1] = 0xff000000
                                                            + ( memIn.ReadU8() << 16 )
                                                            + ( memIn.ReadU8() <<  8 )
                                                            +   memIn.ReadU8();
              pObject->m_vectFaces[dwFaceNumber - 1].m_dwDiffuseColor[2] = 0xff000000
                                                            + ( memIn.ReadU8() << 16 )
                                                            + ( memIn.ReadU8() <<  8 )
                                                            +   memIn.ReadU8();
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

              DWORD   dwFaceNumber = memIn.ReadU32();

              if ( dwFaceNumber > pObject->m_vectFaces.size() )
              {
                // Specular with invalid face number
                bError = true;
                break;
              }

              /*
              pObject->m_vectFaces[dwFaceNumber - 1].m_dwSpecularColor[0] = 0xff000000
                                                            + ( memIn.ReadU8() << 16 )
                                                            + ( memIn.ReadU8() <<  8 )
                                                            +   memIn.ReadU8();
              pObject->m_vectFaces[dwFaceNumber - 1].m_dwSpecularColor[1] = 0xff000000
                                                            + ( memIn.ReadU8() << 16 )
                                                            + ( memIn.ReadU8() <<  8 )
                                                            +   memIn.ReadU8();
              pObject->m_vectFaces[dwFaceNumber - 1].m_dwSpecularColor[2] = 0xff000000
                                                            + ( memIn.ReadU8() << 16 )
                                                            + ( memIn.ReadU8() <<  8 )
                                                            +   memIn.ReadU8();
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

              DWORD   dwFaceNumber = memIn.ReadU32();

              if ( dwFaceNumber > pObject->m_vectFaces.size() )
              {
                // Normals with invalid face number
                bError = true;
                break;
              }

              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[0].x = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[0].y = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[0].z = memIn.ReadF32();

              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[1].x = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[1].y = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[1].z = memIn.ReadF32();

              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[2].x = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[2].y = memIn.ReadF32();
              pObject->m_vectFaces[dwFaceNumber - 1].vectNormal[2].z = memIn.ReadF32();
            }
            break;
          case TAO::CHUNK::TEXTURE_LIST:
            {
              /*
              // Texturen
              DWORD   dwTextureCount = memIn.ReadU32();

              for ( int i = 0; i < dwTextureCount; i++ )
              {
                GR::String     strLine = memIn.ReadString();

                vectTextures.push_back( theSettings.LoadTexture( strLine.c_str() ) );
              }
              */
            }
            break;
          case TAO::CHUNK::MATERIAL_LIST:
            {
              /*
              // Material-Liste
              DWORD dwMaterialCount = memIn.ReadU32();

              for ( int i = 0; i < dwMaterialCount; i++ )
              {
                C3ditorMaterial *pMat = new C3ditorMaterial();

                pMat->m_strName = memIn.ReadString();

                DWORD   dwTextureIndex = memIn.ReadU32();

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
        aFile.SetPosition( dwPos + pChunk->Size() + 6 );
      }
      while ( pChunk->Type() != TAO::CHUNK::END_OF_FILE );

      delete pChunk;

      return pObject;

    }

};


#endif // MESH_OBJECT_LOADER_H