/*
 * File: config.c
 * Author: ylyamin
 */
#ifndef CONFIG_H_
#define CONFIG_H_

#define PLATFORM_SIPEED     "sipeed"
#define PLATFORM_DEVTERM    "devterm"

#ifndef PLATFORM
#define PLATFORM "sipeed"
#endif

#ifndef VERSION_GIT
#define VERSION_GIT "NA"
#endif

/* #ifndef PLATFORM
#define PLATFORM    PLATFORM_SIPEED
#endif */

#define CONFIG_LOG_LEVEL    LOG_DBG
//#define CONFIG_USE_DCACHE 1
//#define CONFIG_USE_PMP    1
//#define CONFIG_USE_MMU    1
#define CONFIG_USE_TWI      1
#define CONFIG_USE_USB      1
#define CONFIG_USE_DISPLAY  1

#endif  //CONFIG_H_