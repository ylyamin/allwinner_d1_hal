/*
 * File: config.c
 * Author: ylyamin
 */
#ifndef CONFIG_H_
#define CONFIG_H_

#ifndef VERSION_GIT
#define VERSION_GIT "0"
#endif

#define CONFIG_LOG_LEVEL    LOG_DBG
//#define CONFIG_USE_DCACHE 1
//#define CONFIG_USE_PMP    1
//#define CONFIG_USE_MMU    1
#define CONFIG_USE_TWI      1
#define CONFIG_USE_USB      1
#define CONFIG_USE_DISPLAY  1

#endif  //CONFIG_H_