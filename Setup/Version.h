#ifndef VERSION_H
#define VERSION_H



#include <GR/GRTypes.h>

#include <windows.h>
#include <stdio.h>



bool GetFileVersion( const GR::String& FileName, GR::u32& Major, GR::u32& Minor, GR::u32& Revision, GR::u32& BuildNumber );
bool GetResourceVersion( HINSTANCE hInstance, const GR::Char* szResName, GR::u32& Major, GR::u32& Minor, GR::u32& Revision, GR::u32& BuildNumber );
#endif // __VERSION_H__



