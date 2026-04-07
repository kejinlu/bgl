//
//  bgl_port.h
//  libbgl
//
//  Cross-platform compatibility macros for Windows/POSIX.
//

#ifndef bgl_port_h
#define bgl_port_h

// ---------- Platform Detection ----------
#ifdef _WIN32
    #define BGL_PLATFORM_WINDOWS 1
#else
    #define BGL_PLATFORM_WINDOWS 0
#endif

// ---------- Headers ----------
#ifdef _WIN32
    #include <io.h>
    #include <stdlib.h>  // for _strdup
#else
    #include <unistd.h>
#endif

// ---------- File Descriptor Operations ----------
#ifdef _WIN32
    #define BGL_DUP   _dup
    #define BGL_LSEEK _lseek
    #define BGL_FILENO _fileno
#else
    #define BGL_DUP   dup
    #define BGL_LSEEK lseek
    #define BGL_FILENO fileno
#endif

// ---------- String Operations ----------
#ifdef _WIN32
    #define bgl_strdup         _strdup
    #define bgl_strcasecmp    _stricmp
    #define bgl_strncasecmp   _strnicmp
#else
    #define bgl_strdup         strdup
    #define bgl_strcasecmp    strcasecmp
    #define bgl_strncasecmp   strncasecmp
#endif

#endif // bgl_port_h
