#pragma once


namespace GR
{
  namespace Gamebase
  {
    namespace FileChunks
    {
      enum FileChunkTypes
      {
        IMAGE_CONTAINER       = 0x6000,     // container of 1 or more IMAGE_FRAME
        IMAGE_FRAME           = 0x6010,     // container of IMAGE_DATA [IMAGE_MASK_DATA] [PALETTE] [ANIMATION_INFO]
        IMAGE_DATA            = 0x6011,     // u32 w, u32 h, u32 format, data
        IMAGE_MASK_DATA       = 0x6012,     // u32 w, u32 h, u32 format, data
        ANIMATION_INFO        = 0x6013,     // f32 - delay in ms

        PALETTE               = 0x6020,     // u32 num entries, n * u8 triplets of RGB

        // layered map
        MAP                   = 0x7000,     // map base info
        LAYER_TILES           = 0x7010,
        LAYER_FREE_SECTION    = 0x7011,
        LAYER_FLAGS           = 0x7012,
        LAYER_OBJECTS         = 0x7020,
        LAYER_OBJECT          = 0x7021,
        MAP_REGION            = 0x7030,
        MAP_TRIGGER           = 0x7031,
        MAP_EXTRADATA         = 0x7032,
        MAP_MOVEMENT_PATH     = 0x7033,

        SCRIPT                = 0x8000,

        OBJECT_HEAD           = 0xA000,
        OBJECT_POSITION       = 0xA100,
        VERTEX_LIST           = 0xA200,
        FACE_LIST             = 0xA300,
        MATERIAL_LIST         = 0xA400,
        TEXTURE_LIST          = 0xA500,
        FRAME_HEAD            = 0xA600,
        JOINT_LIST            = 0xA700,
        VERTEX_GROUPS         = 0xA800,
        FACE_GROUPS           = 0xA900,
        SURFACE_LIST          = 0xAA00,
        SURFACE               = 0xAA01,
        SURFACE_FACE_INDICES  = 0xAA02,
        SURFACE_GROUPS        = 0xAB00,

        MAGIC_NUMBER          = 0xABCD,

        FACE_ALPHA            = 0xF000,
        FACE_TEXTURE          = 0xF001,
        FACE_DIFFUSE_RGB      = 0xF002,
        FACE_SPECULAR_RGB     = 0xF003,
        FACE_NORMALS          = 0xF004,
        FACE_MATERIAL         = 0xF005,
        FACE_TEXTURE_UV       = 0xF006,
        FACE_DIFFUSE_ARGB     = 0xF012,
        FACE_SPECULAR_ARGB    = 0xF013,

        FRAMEWORK_SETTINGS    = 0xF100,
        
        END_OF_FILE           = 0xFFFF
      };
    }
  }
}