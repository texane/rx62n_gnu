/******************************************************************************
* DISCLAIMER
* Please refer to http://www.renesas.com/disclaimer
******************************************************************************
  Copyright (C) 2010. Renesas Electronics Corp., All Rights Reserved.
*******************************************************************************
* File Name    : R_CAN_API_CFG.H
* Version      : 1.00
* Description  : Edit this file to configure the CAN API.
******************************************************************************
* History
* Mar 22 '10  REA	For RX62N with new CAN API.
******************************************************************************/
//Set TAB to 4 spaces!

#ifndef R_CAN_API_CFG_H
#define R_CAN_API_CFG_H

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/******************************************************************************
Typedef definitions
******************************************************************************/
/******************************************************************************
Macro definitions
******************************************************************************/
/* Mailboxes used for demo. Keep mailboxes 4 apart if you want masks 
independent - not affecting neighboring mailboxes. */
#define 	CANBOX_TX		    0x01	//Mailbox #1
#define 	CANBOX_RX 		    0x04	//Mailbox #4
/* Added to show how to use multiple mailboxes. */
#define 	CANBOX_REMOTE_RX	0x08	//Mailbox #8
#define 	CANBOX_REMOTE_TX	0x0C	//Mailbox #12

#define		CANBOX_ERROR_TX		0x10

/* Include this line if you want to poll can mailboxes for messages received 
and sent. COMMENT to use the CAN interrupts. */
#define USE_CAN_POLL			1

/*** CAN interrupt ************************************************************/
/* Include this line if you are using CAN0 interrupts. */
#define CAPI_CFG_CAN0_ISR		1

/* Level */
#define CAN0_INT_LVL			2

/* In the interrupts, USE_CAN_API_SEARCH is default. Uncomment to use the 
mailbox search register. Should be faster if a lot of mailboxes to check. 
This is NOT verified, the opposite may be true. */
#define USE_CAN_API_SEARCH		1

/*** Board specific ports ******************************************************
Tell me where you want to map the transceiver control pins.
Some transceivers may have other control pins. You would have to configure this
yourself. */

/* Configure CAN0 STBn pin. For the RDK this is the P41 pin.
Output. High = not standby. */
#define CAN0_TRX_STB_PORT 	  	4
#define CAN0_TRX_STB_PIN		1
#define CAN0_TRX_STB_LVL		1   //High = Not standby.

/* Configure CAN0 EN pin. For the RDK this is the P42 pin.
Output. High to enable CAN transceiver. */
#define CAN0_TRX_ENABLE_PORT    4
#define CAN0_TRX_ENABLE_PIN     2
#define CAN0_TRX_ENABLE_LVL     1   //High = Enable.

/*** Baudrate settings ********************************************************
	Calculation of baudrate:
	*********************************
	*	PCLK = 48 MHz = fcan.		*
	*	fcanclk = fcan/prescale		*
	*********************************
	
	Example 1) 
	Desired baudrate 500 kbps.
	Selecting prescale to 4.
    fcanclk = 48000000/4
	fcanclk = 12000000 Hz
	Bitrate = fcanclk/Tqtot
	 or,
	Tqtot = fcanclk/bitrate
	Tqtot = 12000000/500000
	Tqtot = 120/5 = 24.
    Tqtot = TSEG1 + TSEG2 + SS
	Using TSEG1 = 15 Tq
		  TSEG2 = 8 Tq
		  SS = 1 Tq always
		  Re-synchronization Control (SJW) should be 1-4 Tq (must be <=TSEG2).
	#define CAN_BRP		4
	#define CAN_TSEG1	15
	#define CAN_TSEG2	8
	#define CAN_SJW		2
	
	Example 2) Selecting prescale to 8.
    Desired baudrate 500 kbps.
	fcanclk = 48000000/8
	fcanclk = 6000000 Hz
	Tqtot = fcanclk/bitrate
	Tqtot = 6000000/500000
	Tqtot = 60/5 = 12.
	Tqtot = TSEG1 + TSEG2 + SS
	Using 	TSEG1 = 8 Tq
			TSEG2 = 3 Tq
			SS = 1 Tq always
			SJW should be 1-4 Tq (<=TSEG2). *
	#define CAN_BRP		8
	#define CAN_TSEG1	8
	#define CAN_TSEG2	3
	#define CAN_SJW		1	*/

/* bitrate configuration */
#if 0 /* 500kbps, prescale to 4 */
# define CAN_BRP 4
# define CAN_TSEG1 15
# define CAN_TSEG2 8
# define CAN_SJW 2
#elif 0 /* 500kbps, prescale to 8 */
# define CAN_BRP 8
# define CAN_TSEG1 8
# define CAN_TSEG2 3
# define CAN_SJW 1
#elif 0 /* 125kbps, prescale to 8 */
# define Fosc 48000000
# define Fbit 125000
# define CAN_TSEG1 14
# define CAN_TSEG2 8
# define CAN_SJW 2
# define Ntq (CAN_TSEG1 + CAN_TSEG2 + CAN_SJW)
# define CAN_BRP (Fosc / (Fbit * Ntq))
#elif 1 /* 125kbps, prescale to 8 */
# define Fosc 48000000
# define Fbit 125000
# define CAN_SJW 2
# define CAN_TPROP 2
# define CAN_TSEG1 (CAN_TPROP + 3) /* warning tseg1 includes tprop */
# define CAN_TSEG2 3
# define Ntq (CAN_SJW + CAN_TSEG1 + CAN_TSEG2)
# define CAN_BRP (Fosc / (Fbit * Ntq))
#endif

	
/*** Other settings  *********************************************************/
/* Max time to poll a CAN register bit for expected value. Don't set to zero. */
#define MAX_CAN_REG_POLLTIME	4

/******************************************************************************
Constant definitions
*****************************************************************************/

/******************************************************************************
Variable Externs
******************************************************************************/

/******************************************************************************
Function prototypes
******************************************************************************/
#endif	/* R_CAN_API_CFG_H */
/* eof */
