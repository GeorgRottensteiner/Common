#ifndef MEMORYLEAK_CHECKER_H
#define MEMORYLEAK_CHECKER_H

#if _DEBUG
#include <crtdbg.h>
#endif

struct tMemoryLeakChecker
{

public:
  tMemoryLeakChecker()
  {
#if _DEBUG // start memory leak checker
  //_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF );
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF );
#endif
  }

};

//#if _DEBUG
static tMemoryLeakChecker g_StaticMemoryLeakChecker;
//#endif


#endif // MEMORYLEAK_CHECKER_H