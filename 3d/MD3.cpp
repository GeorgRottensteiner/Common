#include <debug/debugclient.h>

#include "Md3.h"



CLoadMD3::CLoadMD3() :
  m_pLoadingObject( NULL )
{

  // Here we initialize our structures to 0
  memset(&m_Header, 0, sizeof(tMd3Header));

  // Set the pointers to null
  m_pSkins=NULL;
  m_pTexCoords=NULL;
  m_pTriangles=NULL;
  m_pBones=NULL;

}


///////////////////////////////// IMPORT MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////   This is called by the client to open the .Md3 file, read it, then clean up
/////
///////////////////////////////// IMPORT MD3 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CDX8Object* CLoadMD3::ImportMD3( const char* strFileName )
{

  char strMessage[255] = {0};

  // This function handles the entire model loading for the .md3 models.
  // What happens is we load the file header, make sure it's a valid MD3 model,
  // then load the rest of the data, then call our CleanUp() function.

  // Open the MD3 file in binary
  m_FilePointer = fopen(strFileName, "rb");

  // Make sure we have a valid file pointer (we found the file)
  if ( !m_FilePointer ) 
  {
    // Display an error message and don't load anything if no file was found
    dh::Log( "Unable to find the file: %s!", strFileName );
    return NULL;
  }

  // Now that we know the file was found and it's all cool, let's read in
  // the header of the file.  If it has the correct 4 character ID and version number,
  // we can continue to load the rest of the data, otherwise we need to print an error.

  // Read the header data and store it in our m_Header member variable
  fread(&m_Header, 1, sizeof(tMd3Header), m_FilePointer);

  // Get the 4 character ID
  char *ID = m_Header.fileID;

  // The ID MUST equal "IDP3" and the version MUST be 15, or else it isn't a valid
  // .MD3 file.  This is just the numbers ID Software chose.

  // Make sure the ID == IDP3 and the version is this crazy number '15' or else it's a bad egg
  if((ID[0] != 'I' || ID[1] != 'D' || ID[2] != 'P' || ID[3] != '3') || m_Header.version != 15)
  {
    // Display an error message for bad file format, then stop loading
    dh::Log( "Invalid file format (Version not 15): %s!", strFileName );
    return NULL;
  }

  m_pLoadingObject = new CDX8Object();
  
  // Read in the model and animation data
  ReadMD3Data();

  // Clean up after everything
  CleanUp();

  // Return a success
  return m_pLoadingObject;

}


///////////////////////////////// READ MD3 DATA \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////   This function reads in all of the model's data, except the animation frames
/////
///////////////////////////////// READ MD3 DATA \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CLoadMD3::ReadMD3Data()
{

  int i = 0;

  // This member function is the BEEF of our whole file.  This is where the
  // main data is loaded.  The frustrating part is that once the data is loaded,
  // you need to do a billion little things just to get the model loaded to the screen
  // in a correct manner.

  // Here we allocate memory for the bone information and read the bones in.
  m_pBones = new tMd3Bone [m_Header.numFrames];
  fread(m_pBones, sizeof(tMd3Bone), m_Header.numFrames, m_FilePointer);

  // Since we don't care about the bone positions, we just free it immediately.
  // It might be cool to display them so you could get a visual of them with the model.

  // Free the unused bones
  delete [] m_pBones;

  // Next, after the bones are read in, we need to read in the tags.  Below we allocate
  // memory for the tags and then read them in.  For every frame of animation there is
  // an array of tags.

  fseek( m_FilePointer, m_Header.numFrames * m_Header.numTags, SEEK_CUR );
  /*
  pModel->pTags = new tMd3Tag [m_Header.numFrames * m_Header.numTags];
  fread(pModel->pTags, sizeof(tMd3Tag), m_Header.numFrames * m_Header.numTags, m_FilePointer);

  // Assign the number of tags to our model
  pModel->numOfTags = m_Header.numTags;
  */
  
  // Now we want to initialize our links.  Links are not read in from the .MD3 file, so
  // we need to create them all ourselves.  We use a double array so that we can have an
  // array of pointers.  We don't want to store any information, just pointers to t3DModels.

  /*
  pModel->pLinks = (t3DModel **) malloc(sizeof(t3DModel) * m_Header.numTags);
  
  // Initilialize our link pointers to NULL
  for (i = 0; i < m_Header.numTags; i++)
      pModel->pLinks[i] = NULL;
  */

  // Now comes the loading of the mesh data.  We want to use ftell() to get the current
  // position in the file.  This is then used to seek to the starting position of each of
  // the mesh data arrays.

  // Get the current offset into the file
  long meshOffset = ftell(m_FilePointer);

  // Create a local meshHeader that stores the info about the mesh
  tMd3MeshInfo meshHeader;

  m_pLoadingObject->AddFrame();

  m_iVertexOffset = 0;
  m_iFaceOffset   = 0;

  // Go through all of the sub-objects in this mesh
  for (i = 0; i < m_Header.numMeshes; i++)
  {
    // Seek to the start of this mesh and read in it's header
    fseek(m_FilePointer, meshOffset, SEEK_SET);
    fread(&meshHeader, sizeof(tMd3MeshInfo), 1, m_FilePointer);

    // Here we allocate all of our memory from the header's information
    m_pSkins     = new tMd3Skin [meshHeader.numSkins];
    m_pTexCoords = new tMd3TexCoord [meshHeader.numVertices];
    m_pTriangles = new tMd3Face [meshHeader.numTriangles];
    m_pVertices  = new tMd3Triangle [meshHeader.numVertices * meshHeader.numMeshFrames];

    // Read in the skin information
    fread(m_pSkins, sizeof(tMd3Skin), meshHeader.numSkins, m_FilePointer);
    
    // Seek to the start of the triangle/face data, then read it in
    fseek(m_FilePointer, meshOffset + meshHeader.triStart, SEEK_SET);
    fread(m_pTriangles, sizeof(tMd3Face), meshHeader.numTriangles, m_FilePointer);

    // Seek to the start of the UV coordinate data, then read it in
    fseek(m_FilePointer, meshOffset + meshHeader.uvStart, SEEK_SET);
    fread(m_pTexCoords, sizeof(tMd3TexCoord), meshHeader.numVertices, m_FilePointer);

    // Seek to the start of the vertex/face index information, then read it in.
    fseek(m_FilePointer, meshOffset + meshHeader.vertexStart, SEEK_SET);
    fread(m_pVertices, sizeof(tMd3Triangle), meshHeader.numMeshFrames * meshHeader.numVertices, m_FilePointer);

    // Now that we have the data loaded into the Quake3 structures, let's convert them to
    // our data types like t3DModel and t3DObject.  That way the rest of our model loading
    // code will be mostly the same as the other model loading tutorials.
    ConvertDataStructures( meshHeader );

    // Free all the memory for this mesh since we just converted it to our structures
    delete [] m_pSkins;    
    delete [] m_pTexCoords;
    delete [] m_pTriangles;
    delete [] m_pVertices;   

    // Increase the offset into the file
    meshOffset += meshHeader.meshSize;
  }
}


///////////////////////////////// CONVERT DATA STRUCTURES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////   This function converts the .md3 structures to our own model and object structures
/////
///////////////////////////////// CONVERT DATA STRUCTURES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CLoadMD3::ConvertDataStructures( tMd3MeshInfo meshHeader)
{

  int i = 0;

  // This is function takes care of converting all of the Quake3 structures to our
  // structures that we have been using in all of our mode loading tutorials.  You
  // do not need this function if you are going to be using the Quake3 structures.
  // I just wanted to make it modular with the rest of the tutorials so you (me really) 
  // can make a engine out of them with an abstract base class.  Of course, each model
  // has some different data variables inside of the, depending on each format, but that
  // is perfect for some cool inheritance.  Just like in the .MD2 tutorials, we only
  // need to load in the texture coordinates and face information for one frame
  // of the animation (eventually in the next tutorial).  Where, the vertex information
  // needs to be loaded for every new frame, since it's vertex key frame animation 
  // used in .MD3 models.  Half-life models do NOT do this I believe.  It's just
  // pure bone/skeletal animation.  That will be a cool tutorial if the time ever comes.

  // Increase the number of objects (sub-objects) in our model since we are loading a new one
  //pModel->numOfObjects++;
      
  // Create a empty object structure to store the object's info before we add it to our list
  //t3DObject currentMesh = {0};

  // Copy the name of the object to our object structure
  //strcpy(currentMesh.strName, meshHeader.strName);

  // Assign the vertex, texture coord and face count to our new structure
  /*
  currentMesh.numOfVerts   = meshHeader.numVertices;
  currentMesh.numTexVertex = meshHeader.numVertices;
  currentMesh.numOfFaces   = meshHeader.numTriangles;
  */

  // Allocate memory for the vertices, texture coordinates and face data.
  // Notice that we multiply the number of vertices to be allocated by the
  // number of frames in the mesh.  This is because each frame of animation has a 
  // totally new set of vertices.  This will be used in the next animation tutorial.
  /*
  currentMesh.pVerts    = new CVector3 [currentMesh.numOfVerts * meshHeader.numMeshFrames];
  currentMesh.pTexVerts = new CVector2 [currentMesh.numOfVerts];
  currentMesh.pFaces    = new tFace [currentMesh.numOfFaces];
  */

  int     iCurrentVertexOffset = m_iVertexOffset;

  // Go through all of the vertices and assign them over to our structure
  for (i=0; i < meshHeader.numVertices * meshHeader.numMeshFrames; i++)
  {
    // For some reason, the ratio 64 is what we need to divide the vertices by,
    // otherwise the model is gargantuanly huge!  If you use another ratio, it
    // screws up the model's body part position.  I found this out by just
    // testing different numbers, and I came up with 65.  I looked at someone
    // else's code and noticed they had 64, so I changed it to that.  I have never
    // read any documentation on the model format that justifies this number, but
    // I can't get it to work without it.  Who knows....  Maybe it's different for
    // 3D Studio Max files verses other software?  You be the judge.  I just work here.. :)
    /*
      currentMesh.pVerts[i].x =  m_pVertices[i].vertex[0] / 64.0f;
      currentMesh.pVerts[i].y =  m_pVertices[i].vertex[1] / 64.0f;
      currentMesh.pVerts[i].z =  m_pVertices[i].vertex[2] / 64.0f;
      */
    m_pLoadingObject->AddVertex( m_pVertices[i].vertex[0] / 64.0f, 
                                 m_pVertices[i].vertex[1] / 64.0f,
                                 m_pVertices[i].vertex[2] / 64.0f );
    ++m_iVertexOffset;
  }

  // Go through all of the uv coords and assign them over to our structure
  for (i=0; i < meshHeader.numVertices; i++)
  {
    // Since I changed the images to bitmaps, we need to negate the V ( or y) value.
    // This is because I believe that TARGA (.tga) files, which were originally used
    // with this model, have the pixels flipped horizontally.  If you use other image
    // files and your texture mapping is crazy looking, try deleting this negative.
    /*
    currentMesh.pTexVerts[i].x =  m_pTexCoords[i].textureCoord[0];
    currentMesh.pTexVerts[i].y = -m_pTexCoords[i].textureCoord[1];
    */
  }

  // Go through all of the face data and assign it over to OUR structure
  for(i=0; i < meshHeader.numTriangles; i++)
  {
    // Assign the vertex indices to our face data
    size_t  iFace = m_pLoadingObject->AddFace( CDX8Face( iCurrentVertexOffset + m_pTriangles[i].vertexIndices[1], 
                                                         iCurrentVertexOffset + m_pTriangles[i].vertexIndices[0], 
                                                         iCurrentVertexOffset + m_pTriangles[i].vertexIndices[2] ) );
    /*
    currentMesh.pFaces[i].vertIndex[0] = m_pTriangles[i].vertexIndices[0];
    currentMesh.pFaces[i].vertIndex[1] = m_pTriangles[i].vertexIndices[1];
    currentMesh.pFaces[i].vertIndex[2] = m_pTriangles[i].vertexIndices[2];
    */

    // Assign the texture coord indices to our face data (same as the vertex indices)
    m_pLoadingObject->m_vectFaces[iFace].m_fTextureX[1] = m_pTexCoords[m_pTriangles[i].vertexIndices[1]].textureCoord[1];
    m_pLoadingObject->m_vectFaces[iFace].m_fTextureY[1] = m_pTexCoords[m_pTriangles[i].vertexIndices[1]].textureCoord[1];
    m_pLoadingObject->m_vectFaces[iFace].m_fTextureX[0] = m_pTexCoords[m_pTriangles[i].vertexIndices[0]].textureCoord[0];
    m_pLoadingObject->m_vectFaces[iFace].m_fTextureY[0] = m_pTexCoords[m_pTriangles[i].vertexIndices[0]].textureCoord[0];
    m_pLoadingObject->m_vectFaces[iFace].m_fTextureX[2] = m_pTexCoords[m_pTriangles[i].vertexIndices[2]].textureCoord[2];
    m_pLoadingObject->m_vectFaces[iFace].m_fTextureY[2] = m_pTexCoords[m_pTriangles[i].vertexIndices[2]].textureCoord[2];

    /*
    currentMesh.pFaces[i].coordIndex[0] = m_pTriangles[i].vertexIndices[0];
    currentMesh.pFaces[i].coordIndex[1] = m_pTriangles[i].vertexIndices[1];
    currentMesh.pFaces[i].coordIndex[2] = m_pTriangles[i].vertexIndices[2];
    */
  }

  // Here we add the current object to our list object list
  //pModel->pObject.push_back(currentMesh);
}


///////////////////////////////// CLEAN UP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////   This function cleans up our allocated memory and closes the file
/////
///////////////////////////////// CLEAN UP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CLoadMD3::CleanUp()
{
    // Since we free all of the member variable arrays in the same function as
    // we allocate them, we don't need to do any other clean up other than
    // closing the file pointer, which could probably also be done in the same
    // function.  I left it here so you can add more of your cleanup if you add
    // to this class. 

    // Close the current file pointer
    fclose(m_FilePointer);                      
}


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// I hope you didn't throw up when you saw all the code :)  It is a beast to start out,
// but you should try writing it from scratch! :)  And besides, when doing character
// animation, you should expect it to be huge because this is some pretty advanced stuff.
// Since this is proprietary, all we have to work on is the file format.  If you find 
// anything wrong or just plain stupid, let me know and I will make the fix some others 
// don't have to suffer in ignorance like myself :)  Let's go over the whole shebang:
// 
// The first thing you have to understand about characters in Quake3, is that the model
// is split into multiple files.  There is a upper.md3, a head.md3 and a lower.md3.
// These hold the upper body, the head and legs models.  Only the upper body and the
// lower body have animations assigned to them.  This is why you will see a little
// head in the middle of the body in wire frame mode because the Lara Croft model
// needed animation in her head, so the artist cut off the head and put it in a secret
// spot (her tummy... oooooow), then made her head and pony tail part of the upper body.
// A cool trick eh? (just wanted the Canadians to feel like I support them).  
// 
// Speaking of animation, so how does this stuff all work?  Is it bone animation or isn't it?
// Well, yes and now.  First, yes, there is bone animation because the legs are the parent
// node, and then the upper body attaches to the legs, then the head attaches to the upper 
// body (or in Lara's model, the torso).  Wherever the legs move, the upper body is then
// translated and rotated to the correct position that it should be, and then the head
// is in the same matrix scope so it goes where the upper body goes.  Once again, we do not
// handle rotating in this tutorial.  That requires a ton of matrix code that will be used
// in the next animation tutorial with quaternions (The room goes silent.....).
//
// And as for the second point, no the animation isn't exactly bone animation.  Each
// animation is stored in vertex key frames, just like the .MD2 files were.  The only
// difference is that we have 3 body parts to keep track of.  That way it is more realistic
// in the way of, you can have Lara Croft doing something totally different with her legs,
// than what the top part of her is doing.  Such as, when she is doing the jumping animation,
// her upper body could be doing a shooting, picking up, or dropping animation.  Make sense?
// So to the question of if this is bone animation, I can say a firm!!!..... Yes/No :)
//
// The .md3 files has skins and shader files associated with them.  It has a skin if it's
// a body part and a shader file if it's a weapon model.  These files store, for the most
// part, the textures associated with each model.  Sometimes the model has multiple textures,
// one for each sub-object.  I left the skin files the same, but I deleted a lot of the
// garbage that we didn't need in the .shader file.  Look at LoaderShader() for more info.
//
// There is also a config (.cfg) file for the animations of the model.  This stores the
// first frame, the amount of frames for that animation, the looping frame count (not used),
// and the frames per second that that animation should run.  We don't do anything with this
// file in this tutorial, but in the next tutorial we will.
//
// Finally, this brings us to the most confusing part in the .md3 model format, the tags.
// What the heck are tags?  Tags can be thought of as joints.  If you look in the .skin files
// you will see the tags on a separate line, right before the texture info.  These are basically
// joints that we need to connect other models too.  These are the ones I have seen:
//
// tag_torso  - The joint that connects the hips and the upper body together
// tag_head   - The joing that connects the neck and the head model too
// tag_weapon - The tag that connects the hand of the upper body and the weapon too.
//
// Now, that's the easy part to understand... it's like the Velcro that each body part
// sticks too right?  Well, the next part is where the confusion comes in.  Besides being
// a joint it stores the bone rotations and translations that need to be applied to the
// model that is connected to that joint.  For instance, we never rotate or translate the
// lower.md3 model.  That is all taken care of in the vertex key frame animation, but, the
// model stores tags for the "tag_torso" joint, which tells the upper body where to move and
// rotate, depending on what the legs are doing.
//
// A good example of this is in the death animations.  One of Lara's death animations makes
// her do a back flip and land on her face.  Well, the legs animation has key frames that
// it interpolates to, to perform this back flip, but the rest of the model doesn't.  To
// compensate for that, each frame, the lower body model sends a message to the upper body
// model to move and rotate to a certain degree and position, along a certain axis.  In
// programming terms, this means that we push on a new matrix and then apply some translations
// and rotations to the rest of the body parts (upper, head and weapon parts).  You don't
// directly apply it to each one, you just push on a matrix, apply the rotations and 
// translations, then anything drawn after that will be apart of that new matrix.  Simple huh?
// The rotation is stored in a 3x3 rotation matrix.  That is why we didn't bother with it
// in this tutorial because there is no animation so no need to add more code to scare you away.
// The translation is a simple (x, y, z) position that can be easily passed in to glTranslatef().
// This can all be seen in DrawLink().  In the next tutorial, we will not use glTranslatef(),
// but we will just include the translation in our matrix that we create to rotate the model.
// Then we can just do a simple call to glMultMatrix() to do both the rotation and translation
// at the same time.
//
// As a final explanation about the tags, let me address the model linking.  This just
// attaches 2 models together at the desired tag (or joint). For example, we attach the
// the upper.md3 model to the lower.md3 model at the "tag_torso" joint.  Now, when ever
// the legs do their animation, it sends the translation and rotation messages to all of
// it's children, which in this case, is just the upper body model, which effects the
// upper body's children (The weapon and head models).  The link array has nothing to do
// with the file format being loaded in.  That is just the way that I chose to handle it.
// I found some source code at www.planetquake.com by Lonerunner.  This guy/gal is awesome!
// He has a ton of formats that he did, which can be found at Romka's site too.  
//
// That is pretty much the crux of the .MD3 loading.  There is some bone animation to be
// loaded, but I never really figured it out.  The position was always (0, 0, 0) so I don't
// know what those are for.  I was going to display the bones, but it doesn't seem to save
// the position.  Who knows... if you know how, let me know.  Maybe I am ignoring something.
// It has min/max values, but I didn't bother to try and figure it out.
// 
// I would like to point out, this is just the solution I came up with.  There is probably
// a lot more intelligent ways to handle this, but it works great for me.  When the Quake3
// source code comes out, maybe we will all learn a few things from the mast'ehr :)
//
// I would like The author of this fabulous model who allowed me to use it, his handle is:
//
//          - Pornstar (nickelbag@nyc.com).  Tasteless name, but he sure does a cool model :)
//
//
// Let me know if this helps you out!
// 
// 
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
// The Quake3 .Md3 file format is owned by ID Software.  This tutorial is being used 
// as a teaching tool to help understand model loading and animation.  This should
// not be sold or used under any way for commercial use with out written consent
// from ID Software.
//
// Quake, Quake2 and Quake3 are trademarks of ID Software.
// Lara Croft is a trademark of Eidos and should not be used for any commercial gain.
// All trademarks used are properties of their respective owners. 
//
//
//

