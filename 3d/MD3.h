#ifndef MD3_H
#define MD3_H

#include <DX8/DX8Object.h>

// This file stores all of our structures and classes (besides the modular model ones in main.h)
// in order to read in and display a Quake3 character.  The file format is of type
// .MD3 and comes in many different files for each main body part section.  We convert
// these Quake3 structures to our own structures in Md3.cpp so that we are not dependant
// on their model data structures.  You can do what ever you want, but I like mine :P :)

// This holds the header information that is read in at the beginning of the file
struct tMd3Header
{ 
    char    fileID[4];                  // This stores the file ID - Must be "IDP3"
    int     version;                    // This stores the file version - Must be 15
    char    strFile[68];                // This stores the name of the file
    int     numFrames;                  // This stores the number of animation frames
    int     numTags;                    // This stores the tag count
    int     numMeshes;                  // This stores the number of sub-objects in the mesh
    int     numMaxSkins;                // This stores the number of skins for the mesh
    int     headerSize;                 // This stores the mesh header size
    int     tagStart;                   // This stores the offset into the file for tags
    int     tagEnd;                     // This stores the end offset into the file for tags
    int     fileSize;                   // This stores the file size
};

// This structure is used to read in the mesh data for the .md3 models
struct tMd3MeshInfo
{
    char    meshID[4];                  // This stores the mesh ID (We don't care)
    char    strName[68];                // This stores the mesh name (We do care)
    int     numMeshFrames;              // This stores the mesh aniamtion frame count
    int     numSkins;                   // This stores the mesh skin count
    int     numVertices;                // This stores the mesh vertex count
    int     numTriangles;               // This stores the mesh face count
    int     triStart;                   // This stores the starting offset for the triangles
    int     headerSize;                 // This stores the header size for the mesh
    int     uvStart;                    // This stores the starting offset for the UV coordinates
    int     vertexStart;                // This stores the starting offset for the vertex indices
    int     meshSize;                   // This stores the total mesh size
};

// This is our tag structure for the .MD3 file format.  These are used link other
// models to and the rotate and transate the child models of that model.
struct tMd3Tag
{
    char          strName[64];            // This stores the name of the tag (I.E. "tag_torso")
    math::vector3 vPosition;              // This stores the translation that should be performed
    float         rotation[3][3];         // This stores the 3x3 rotation matrix for this frame
};

// This stores the bone information (useless as far as I can see...)
struct tMd3Bone
{
    float   mins[3];                    // This is the min (x, y, z) value for the bone
    float   maxs[3];                    // This is the max (x, y, z) value for the bone
    float   position[3];                // This supposedly stores the bone position???
    float   scale;                      // This stores the scale of the bone
    char    creator[16];                // The modeler used to create the model (I.E. "3DS Max")
};


// This stores the normals and vertex indices 
struct tMd3Triangle
{
   signed short  vertex[3];             // The vertex for this face (scale down by 64.0f)
   unsigned char normal[2];             // This stores some crazy normal values (not sure...)
};


// This stores the indices into the vertex and texture coordinate arrays
struct tMd3Face
{
   int vertexIndices[3];                
};


// This stores UV coordinates
struct tMd3TexCoord
{
   float textureCoord[2];
};


// This stores a skin name (We don't use this, just the name of the model to get the texture)
struct tMd3Skin 
{
    char strName[68];
};


// This class handles all of the main loading code
class CLoadMD3
{

  public:

    // This inits the data members
    CLoadMD3();                             

    // This is the function that you call to load the MD3 model
    CDX8Object*       ImportMD3( const char* strFileName );


  private:


    CDX8Object*           m_pLoadingObject;

    int                   m_iVertexOffset,
                          m_iFaceOffset;


    // This reads in the data from the MD3 file and stores it in the member variables,
    // later to be converted to our cool structures so we don't depend on Quake3 stuff.
    void ReadMD3Data();

    // This converts the member variables to our pModel structure, and takes the model
    // to be loaded and the mesh header to get the mesh info.
    void ConvertDataStructures( tMd3MeshInfo meshHeader );

    // This frees memory and closes the file
    void CleanUp();
    
    // Member Variables     

    // The file pointer
    FILE *m_FilePointer;

    tMd3Header              m_Header;           // The header data

    tMd3Skin                *m_pSkins;          // The skin name data (not used)
    tMd3TexCoord            *m_pTexCoords;      // The texture coordinates
    tMd3Face                *m_pTriangles;      // Face/Triangle data
    tMd3Triangle            *m_pVertices;       // Vertex/UV indices
    tMd3Bone                *m_pBones;          // This stores the bone data (not used)
};



#endif
