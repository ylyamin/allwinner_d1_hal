/*
 * tusb_config.h
 *
 *  Created on: May 5, 2021
 *      Author: Jeremiah McCarthy
 */

#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <platform.h>
#include <log.h>

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUSB_OS           OPT_OS_NONE
#define CFG_TUSB_MCU OPT_MCU_SUN20IW1

#define CFG_TUSB_MEM_SECTION __attribute__((section(".ramnc")))
//#define CFG_TUSB_MEM_ALIGN   __attribute__ ((aligned(64)))

#define CFG_TUSB_DEBUG_PRINTF small_printf
#define CFG_TUSB_DEBUG 3	// 0..3

//--------------------------------------------------------------------
// HOST CONFIGURATION
//--------------------------------------------------------------------

//#define TUP_USBIP_EHCI 1
#define TUP_USBIP_OHCI 1
#define LPC_USB_BASE  (0x04200000+0x400)

#define CFG_TUH_ENDPOINT_MAX 	16
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_HOST

//#define CFG_TUH_MSC	1
#define CFG_TUH_HID	8
//#define CFG_TUH_CDC	12	
//#define CFG_TUH_BTH	1	
//#define CFG_TUH_VENDOR 0
#define CFG_TUH_HUB 12
#define CFG_TUH_DEVICE_MAX	12

#define TUP_OHCI_RHPORTS 1
#define CFG_TUH_ENUMERATION_BUFSIZE	2048

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT      0
#endif

#define BOARD_TUD_MAX_SPEED   OPT_MODE_HIGH_SPEED
#define CFG_TUD_MAX_SPEED     BOARD_TUD_MAX_SPEED

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

#define CFG_TUD_DFU_XFER_BUFSIZE  64//(TUD_OPT_HIGH_SPEED ? 512 : 64)

#ifdef __cplusplus
 }
#endif

#endif /* TUSB_CONFIG_H_ */
