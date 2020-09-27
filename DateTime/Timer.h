#ifndef TIMER_H
#define TIMER_H



namespace Time
{
  class Timer
  {
    public:

      enum  eTimeFlags
      {
        TF_RESET          = 0,
        TF_START,
        TF_STOP,
        TF_ADVANCE,
        TF_GETABSOLUTETIME,
        TF_GETAPPTIME,
        TF_GETELAPSEDTIME,
        TF_REINITIALIZE,
        TF_SET_OFFSET_TIME
      };

      static double Time( eTimeFlags = TF_GETABSOLUTETIME );
  };

}

#endif // TIMER_H

