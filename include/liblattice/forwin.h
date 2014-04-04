#ifndef FORWIN_H
#define FORWIN_H

  #if defined(_WIN32) && !defined(__MINGW32__)

    #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
    #else
      #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
    #endif

    struct timezone
    {
      int  tz_minuteswest; /* minutes W of Greenwich */
      int  tz_dsttime;     /* type of dst correction */
    };

    int gettimeofday(struct timeval *tv, struct timezone *tz);

  #endif

#endif
