#ifndef GAME_EXTRADATA_H
#define GAME_EXTRADATA_H



namespace GR
{
  namespace Game
  {
    namespace ExtraDataType
    {
      enum Value
      {
        CUSTOM = 0,       // any value usage
        SCRIPT,           // Param = script
        WARP,             // Param1 = x, Param2 = y, Param = map
        PATH              // Param = path name
      };
    }

    struct ExtraData
    {
      ExtraDataType::Value  Type;
      GR::i32               ID;
      GR::i32               Param1;
      GR::i32               Param2;
      GR::i32               Param3;
      GR::i32               Param4;
      GR::String            Param;

      ExtraData() :
        ID( -1 ),
        Param1( 0 ),
        Param2( 0 ),
        Param3( 0 ),
        Param4( 0 ),
        Type( ExtraDataType::CUSTOM )
      {
      }
    };

  }
}

#endif // GAME_EXTRADATA_H