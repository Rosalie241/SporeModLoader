#ifndef VERSION_H
#define VERSION_H

#define STR(x) #x
#define STR_MACRO(x) STR(x)

#define VERSION_MAJOR 3
#define VERSION_MINOR 2
#define VERSION_BUILD 4

#define VERSION_STR STR_MACRO(VERSION_MAJOR) "." STR_MACRO(VERSION_MINOR) "." STR_MACRO(VERSION_BUILD)

#endif // VERSION_H
