#ifndef WAD_FILE_H
#define WAD_FILE_H



#include <stdio.h>

#include <IO/FileStream.h>

#include <Memory/MemoryStream.h>

#include <vector>
#include <string>



namespace GR
{

class WADFile
{

  public:

    struct tWADFileEntry
    {
      GR::String        m_FileName;
      GR::u32           m_Offset,
                        m_Size;
      tWADFileEntry() :
        m_FileName( "" ),
        m_Offset( 0 ),
        m_Size( 0 )
        {
        }
    };


  private:

    bool                      Open( IIOStream& ioIn );


  protected:

    typedef std::vector<tWADFileEntry>    tVectFileEntries;


    GR::IO::FileStream        m_File;
    MemoryStream              m_Memory;

    ByteBuffer                m_TemporaryEntryData;

    GR::String                m_FileName;

    tVectFileEntries          m_Entries;


    void                      CleanUp();
    

  public:

    WADFile();
    ~WADFile();

    bool                      Open( const char* FileName );
    bool                      Open( const ByteBuffer& Source );
    void                      Close();

    int                       GetEntryIndex( const char* FileName ) const;

    GR::u32                   GetEntryCount();
    GR::u32                   GetEntryOffset( const char* FileName );
    GR::u32                   GetEntryOffset( GR::u32 Nr );
    GR::u32                   GetEntrySize( const char* FileName );
    GR::u32                   GetEntrySize( GR::u32 Nr );
    GR::String                GetEntryName( GR::u32 Nr );
    MemoryStream              GetEntryStream( GR::u32 Nr );
    GR::IO::FileStream&       GetFileHandle();

    GR::u32                   GetCompleteDataSize() const;

};


}; // namespace GR


#endif // __WAD_FILE_H__



