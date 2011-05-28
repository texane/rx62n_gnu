/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Electronics Corp. and is only 
* intended for use with Renesas products. No other uses are authorized.

* This software is owned by Renesas Electronics Corp. and is protected under 
* all applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES 
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, 
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY 
* DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORP. NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES 
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS 
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this 
* software and to discontinue the availability of this software.  
* By using this software, you agree to the additional terms and 
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************
* Copyright (C) 2010. Renesas Electronics Corp., All Rights Reserved.
******************************************************************************
* File Name    	R_CAN_API.C
* Version 	 	1.00
* Tool-Chain   	RX Standard Toolchain 1.0.0.0
* Platform 		ROK5562N
* Description  	CAN API function definitions
* Operation    	See example usage API_DEMO.C
* Limitations  	:
******************************************************************************
* History
* Mar 22 '10  REA		For RX62N with new CAN API.
* Apr 15 '10  REA		Remote frame handling added.
* Apr 22 '10  REA		Port configuration moved to config.h. User just 
						sets port and pin number for the transceiver control 
						ports. No need to change driver.
						R_CAN_Control: Enter Sleep added.
  					    R_CAN_PortSet: Modes automatically now enter and exit
							 Halt mode, so user need just one call to change
							 mode.
					    R_CAN_TxStopMsg(): TRMREQ to 0, then TRMABT clear.
					    R_CAN_RxSetMask(): Halt CAN before mask change, resume 
						    after.
					    R_CAN_RxPoll(): Function rewritten to use INVALDATA flag.
							R_CAN_NOT_OK added; "No message waiting or currently 
							being written".
					    All RETURN values spelled out in all function headers.
						Return values added and changed for some APIs.
* Jun 7 '10  REA		Changed TRM_ACTIVE -> SENTDATA in R_CAN_WaitTxRx() since 
							TRM_ACTIVE is low for a while after TRMREQ is set high.				
						Increased MAX_CAN_SW_DELAY from 0x1000 to 0x2000. If the
							TxCheck function is not used, the timer could time out
							 and the mailbox will not send properly if the user 
							ignores the api_status warning that WaitTxRx timed out.
******************************************************************************/
//Set TAB = 4 spaces

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include <string.h>
#include "lcd.h"
#include "iodefine.h"
#include "can_config.h"
#include "can.h"

/******************************************************************************
Typedef definitions
******************************************************************************/
/******************************************************************************
Macro definitions
******************************************************************************/
/* These macros are for determining if some can while loop times out. If they do,
the canx_sw_err variable will be non zero. This is to prevent an error in the 
can peripheral and driver from blocking the CPU indefinatly. */
#define DEC_CHK_CAN_SW_TMR		(--can_tmo_cnt != 0)
#define RESET_CAN_SW_TMR		(can_tmo_cnt = MAX_CAN_SW_DELAY);

/* Max delay waiting for CAN register to flip. Set to whatever theoretical 
worst latency you could accept. It should never timeout. Do not set to 0. */
#define MAX_CAN_SW_DELAY 	   (0x2000)
#define CHECK_MBX_NR			{if (mbox_nr > 31) return R_CAN_SW_BAD_MBX;}

/* Board specific port defines. */
#define CAN_TRX_DDR(x, y)			    CAN_TRX_DDR_PREPROC(x, y)
#define CAN_TRX_DDR_PREPROC(x, y)		(PORT ## x .DDR.BIT.B ## y)
#define CAN_TRX_DR(x, y)			    CAN_TRX_DR_PREPROC(x, y)
#define CAN_TRX_DR_PREPROC(x, y)		(PORT ## x .DR.BIT.B ## y)

/******************************************************************************
Global variables and functions imported (externs)
******************************************************************************/
/******************************************************************************
Constant definitions
*****************************************************************************/
/******************************************************************************
Global variables and functions private to the file
******************************************************************************/
/* Data */
/* Functions */
static void 	CanClearSentData(const uint32_t ch_nr, const uint32_t mbox_nr);
static uint32_t R_CAN_WaitTxRx(const uint32_t ch_nr, const uint32_t mbox_nr);
#ifndef USE_CAN_POLL
static void 	R_CAN_ConfigCANinterrupts(const uint32_t ch_nr);
#endif //USE_CAN_POLL

/******************************************************************************

					    C A N 0   F U N C T I O N S

******************************************************************************/
/*******************************************************************************
Function Name:  R_CAN_Create
Description:    Configure the CAN peripheral.
Parameters: 	ch_nr
Return value: 	R_CAN_OK			Action completed successfully.
			    R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
			    R_CAN_SW_RST_ERR	The CAN peripheral did not enter Reset mode.
			    See also R_CAN_Control return values.
*******************************************************************************/
uint32_t R_CAN_Create(const uint32_t ch_nr)
{
	uint32_t api_status = R_CAN_OK;
	uint32_t i, j;
 	/* A faulty CAN peripheral block, due to HW, FW could potentially block (hang) 
	the program at a while-loop. To prevent this, a sw timer in the while-loops
	will time out enabling the CPU to continue. */
	uint32_t can_tmo_cnt = MAX_CAN_SW_DELAY;   

#define __evenaccess volatile __attribute__((aligned(4)))
#define nop() do { asm("nop"); } while (0)

	volatile struct st_can __evenaccess * can_block_p;
	
	if (ch_nr == 0)
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		return R_CAN_BAD_CH_NR;

	/* Exit Sleep mode. */
	api_status |= R_CAN_Control(ch_nr, EXITSLEEP_CANMODE);
	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "r_can_control(sleep)");
	  return api_status;
	}
	
	/* Sleep -> RESET mode. */
	api_status |= R_CAN_Control(ch_nr, RESET_CANMODE);
	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "r_can_control(reset)");
	  return api_status;
	}
	
	/*** Setting of CAN0 Control register.***/
	/* BOM:	Bus Off recovery mode acc. to IEC11898-1 */
    can_block_p->CTLR.BIT.BOM = 0;
	/* MBM: Select normal mailbox mode. */
    can_block_p->CTLR.BIT.MBM = 0;
	/* IDFM: Select standard ID mode. */
    can_block_p->CTLR.BIT.IDFM = 0;
	/* 	:	0 = Overwrite mode: Latest message overwrites old.
			1 = Overrun mode: Latest message discarded. */
    can_block_p->CTLR.BIT.MLM = 0;
	/* TPM: ID priority mode. */
    can_block_p->CTLR.BIT.TPM = 0;
	/* TSRC: Only to be set to 1 in operation mode */
    can_block_p->CTLR.BIT.TSRC = 0;
	/* TSPS: Update every 8 bit times */
    can_block_p->CTLR.BIT.TSPS = 3;

	/* Set BAUDRATE */
    R_CAN_SetBitrate(ch_nr);

	/* Mask invalid for all mailboxes by default. */
	can_block_p->MKIVLR = 0xFFFFFFFF;

	#ifndef USE_CAN_POLL
		/* Configure CAN interrupts. */ 
		R_CAN_ConfigCANinterrupts(ch_nr);
	#endif //USE_CAN_POLL

	/* Reset -> HALT mode */
	api_status |= R_CAN_Control(ch_nr, HALT_CANMODE);

	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "r_can_control(halt)");
	  return api_status;
	}
	
  	/* Configure mailboxes in Halt mode. */
	for (i = 0; i < 32; i++)
	{
	    can_block_p->MB[i].ID.LONG = 0x00;
		can_block_p->MB[i].DLC.WORD = 0x00;
		for (j = 0; j < 8; j++)
		    can_block_p->MB[i].DATA[j] = 0x00;
		for (j = 0; j < 2; j++)
		    can_block_p->MB[i].TS.WORD = 0x00;
	}	

	/* Halt -> OPERATION mode */
	/* Note: EST and BLIF flag go high here when stepping code in debugger. */
	api_status |= R_CAN_Control(ch_nr, OPERATE_CANMODE);

	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "r_can_control(operate)");
	  return api_status;
	}

	/* Time Stamp Counter reset. Set the TSRC bit to 1 in CAN Operation mode. */
	can_block_p->CTLR.BIT.TSRC = 1;
	while ((can_block_p->CTLR.BIT.TSRC) && DEC_CHK_CAN_SW_TMR) {;}
	if (can_tmo_cnt == 0) 
		api_status |= R_CAN_SW_TSRC_ERR;

	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "SW_TMR");
	  return api_status;
	}
	
	/* Check for errors so far, report, and clear. */
	if (can_block_p->STR.BIT.EST)
		api_status |= R_CAN_SW_RST_ERR;

	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "EST");
	  return api_status;
	}

	/* Clear Error Interrupt Factor Judge Register. */
	if (can_block_p->EIFR.BYTE)
		api_status |= R_CAN_SW_RST_ERR;

	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "EIFR.BYTE");
	  return api_status;
	}

	can_block_p->EIFR.BYTE = 0x00;

	/* Clear Error Code Store Register. */
	if (can_block_p->ECSR.BYTE)
		api_status |= R_CAN_SW_RST_ERR;

	if (api_status != R_CAN_OK)
	{
	  lcd_string(5, 0, "ECSR.BYTE");
	  return api_status;
	}

	can_block_p->ECSR.BYTE = 0x00;

	return api_status;
}/* end R_CAN_Create() */

/***********************************************************************************
Function Name:  R_CAN_PortSet
Description:    Configures the MCU and transceiver port pins. This function is 
			    responsible for configuring the MCU and transceiver port pins. 
			    Transceiver port pins such as Enable will vary depending on design, 
			    and this fucntion must then be modified. The function is also used 
			    to enter the CAN port test modes, such as Listen Only.
			
				Typical TJA1041 transceiver voltages with CAN active for RX/62N 
				RSK (ROK5562N0C):
					PIN:		Voltage
					1	TXD		3.25
					2	GND		0.00
					3	VCC		5.08
					4	RXD		3.20
					5	VIO		3.25
					6	EN		3.25
					7	INH		5.08

					8	ERR		0.10
					9	WAKE	0.00
					10	VBAT	5.08
					11	SPLIT	2.57
					12	CANL	2.56
					13	CANH	2.56
					14	STB		3.25
					
Parameters:	 ch_nr 
				action_types: ENABLE, DISABLE, CANPORT_TEST_LISTEN_ONLY, 
				CANPORT_TEST_0_EXT_LOOPBACK, CANPORT_TEST_1_INT_LOOPBACK, and
				CANPORT_RETURN_TO_NORMAL which is the default; no need to call 
				unless another test mode was invoked previously.
Return value:   R_CAN_OK			    Action completed successfully.
			    R_CAN_SW_BAD_MBX		Bad mailbox number.
			    R_CAN_BAD_CH_NR 	   	The channel number does not exist.
			    R_CAN_BAD_ACTION_TYPE	No such action type exists for this function.
			    R_CAN_SW_HALT_ERR		The CAN peripheral did not enter Halt mode.
			    R_CAN_SW_RST_ERR		The CAN peripheral did not enter Reset mode.
			    See also R_CAN_Control return values.
***********************************************************************************/
uint32_t R_CAN_PortSet(const uint32_t ch_nr, const uint32_t action_type)
{  
	uint32_t api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;
	
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;
		
    switch (action_type)
	{
	case ENABLE:
		/* Enable the CTX0 and CRX0 pins. */

		/* Port function control register J (PFJCAN) is used to configure I/O pin.
		1: The CTX0 and CRX0 pins are enabled. */
		IOPORT.PFJCAN.BIT.CAN0E = 1;

		/* P3_2 as CTX0 and P3_3 as CRX0. */
		PORT3.DDR.BIT.B2 = 1;	//CTX0 is output
	    PORT3.DDR.BIT.B3 = 0;	//CRX0 is input
		
		/* Set ICR for P3_3 (CRX0) in order to connect input to CAN periph. */
		PORT3.ICR.BIT.B3 = 1;

		/* Configure CAN0 STBn pin. See config.h. */
		//PORT4.DDR.BIT.B1 = 1;
	    CAN_TRX_DDR( CAN0_TRX_STB_PORT, CAN0_TRX_STB_PIN ) = 1;
		
		//PORT4.DR.BIT.B1 = 1;
	    CAN_TRX_DR(CAN0_TRX_STB_PORT, CAN0_TRX_STB_PIN) = CAN0_TRX_STB_LVL;
		
		/* Configure CAN0 EN pin. */
		//PORT4.DDR.BIT.B2 = 1;
	    CAN_TRX_DDR( CAN0_TRX_ENABLE_PORT, CAN0_TRX_ENABLE_PIN ) = 1;
		//PORT4.DR.BIT.B2 = 1;
	    CAN_TRX_DR(CAN0_TRX_ENABLE_PORT, CAN0_TRX_ENABLE_PIN) = CAN0_TRX_ENABLE_LVL;		
    break;
	
	case DISABLE:
		/* Configure CAN0 TX and RX pins. */

		/* Port function control register J (PFJCAN) is used to configure I/O pin 
		P3_2 as CTX0 and P3_3 as CRX0. 
		0: The CTX0 and CRX0 pins are disabled. */
		IOPORT.PFJCAN.BIT.CAN0E = 0;

		/* Configure CAN0 STBn pin. See config.h. */
		//PORT4.DDR.BIT.B1 = 1;
	    CAN_TRX_DDR( CAN0_TRX_STB_PORT, CAN0_TRX_STB_PIN ) = 1;
		
		//PORT4.DR.BIT.B1 = 1;
	    CAN_TRX_DR(CAN0_TRX_STB_PORT, CAN0_TRX_STB_PIN) = !CAN0_TRX_STB_LVL; //Negated level
		
		/* Configure CAN0 EN pin. */
		//PORT4.DDR.BIT.B2 = 1;
	    CAN_TRX_DDR( CAN0_TRX_ENABLE_PORT, CAN0_TRX_ENABLE_PIN ) = 1;
		//PORT4.DR.BIT.B2 = 1;
	    CAN_TRX_DR(CAN0_TRX_ENABLE_PORT, CAN0_TRX_ENABLE_PIN) = !CAN0_TRX_ENABLE_LVL; //Negated level
    break;
	
	/* Run in Listen Only test mode. */
	case CANPORT_TEST_LISTEN_ONLY:
		api_status = R_CAN_Control(0, HALT_CANMODE);
		can_block_p->TCR.BYTE = 0x03;
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);
	    api_status |= R_CAN_PortSet(0, ENABLE);
	break;
	
	/* Run in External Loopback test mode. */
	case CANPORT_TEST_0_EXT_LOOPBACK:
		api_status = R_CAN_Control(0, HALT_CANMODE);
	    can_block_p->TCR.BYTE = 0x05;
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);
	    api_status |= R_CAN_PortSet(0, ENABLE);
	break;
	
	/* Run in Internal Loopback test mode. */
	case CANPORT_TEST_1_INT_LOOPBACK:
		api_status = R_CAN_Control(0, HALT_CANMODE);
		can_block_p->TCR.BYTE = 0x07;
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);
	break;
	
	/* Return to default CAN bus mode. 
	This is the default setting at CAN reset. */
	case CANPORT_RETURN_TO_NORMAL:
		api_status = R_CAN_Control(0, HALT_CANMODE);
		can_block_p->TCR.BYTE = 0x00;
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);
	    api_status |= R_CAN_PortSet(0, ENABLE);
	break;
	
    default:
		/* Bad action type. */
	    api_status = R_CAN_BAD_ACTION_TYPE;
	break;
	}
    return api_status;
}/* end R_CAN_PortSet() */

/*******************************************************************************
Function Name:  R_CAN_Control
Description:    Controls transition to CAN operating modes determined by the CAN 
			    Control register. For example, the Halt mode should be used to 
			    later configure a recieve mailbox. 
Parameters: 	ch_nr
				action_type: EXITSLEEP_CANMODE, ENTERSLEEP_CANMODE,
			    RESET_CANMODE, HALT_CANMODE, OPERATE_CANMODE.
Return value: 	R_CAN_OK			    Action completed successfully.
			    R_CAN_SW_BAD_MBX		Bad mailbox number.
			    R_CAN_BAD_CH_NR 	   	The channel number does not exist.
			    R_CAN_BAD_ACTION_TYPE	No such action type exists for this function.
			    R_CAN_SW_WAKEUP_ERR	The CAN peripheral did not wake up from Sleep mode.
			    R_CAN_SW_SLEEP_ERR		The CAN peripheral did enter Sleep mode.
			    R_CAN_SW_RST_ERR		The CAN peripheral did not enter Halt mode.
			    R_CAN_SW_HALT_ERR		The CAN peripheral did not enter Halt mode.
			    R_CAN_SW_RST_ERR		The CAN peripheral did not enter Reset mode.
			    See also R_CAN_PortSet return values.
*******************************************************************************/
uint32_t R_CAN_Control(const uint32_t ch_nr, const uint32_t action_type)
{
    uint32_t api_status = R_CAN_OK;
	uint32_t can_tmo_cnt = MAX_CAN_SW_DELAY;
	volatile struct st_can __evenaccess * can_block_p;
	
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;

	switch (action_type)
	{
	case EXITSLEEP_CANMODE:
		/* Set to, and ensure that RCAN exits, Sleep mode. */
		can_block_p->CTLR.BIT.SLPM = CAN_NOT_SLEEP;
		while ((can_block_p->STR.BIT.SLPST) && DEC_CHK_CAN_SW_TMR)
			nop();
	    if (can_tmo_cnt == 0) 
			api_status = R_CAN_SW_WAKEUP_ERR;
	break;

	case ENTERSLEEP_CANMODE:
		/* Set to, and ensure that RCAN is in, the Sleep state. */
		can_block_p->CTLR.BIT.SLPM = CAN_SLEEP;
		while ((!can_block_p->STR.BIT.SLPST) && DEC_CHK_CAN_SW_TMR)
			nop();
	    if (can_tmo_cnt == 0) 
			api_status = R_CAN_SW_SLEEP_ERR;
	break;

	case RESET_CANMODE:
	  /* Set to, and ensure that RCAN is in, the Reset state. */
	  can_block_p->CTLR.BIT.CANM = CAN_RESET;
	  while ((!can_block_p->STR.BIT.RSTST) && DEC_CHK_CAN_SW_TMR)
	    nop();
	  if (can_tmo_cnt == 0)
	    api_status = R_CAN_SW_RST_ERR;
	break;

	case HALT_CANMODE:
		/* Set to, and ensure that RCAN is in, the Halt state. */
		/* The CAN module enters CAN Halt mode after waiting for the end of 
	    message reception or transmission. */
	    can_block_p->CTLR.BIT.CANM = CAN_HALT;
		while ((!can_block_p->STR.BIT.HLTST) && DEC_CHK_CAN_SW_TMR)
		{;}
	    if (can_tmo_cnt == 0)
			api_status = R_CAN_SW_HALT_ERR;
	break;

	case OPERATE_CANMODE:
		
		/* Take CAN out of Stop mode. */
		SYSTEM.MSTPCRB.BIT.MSTPB0  =  0;
		while ((SYSTEM.MSTPCRB.BIT.MSTPB0) && DEC_CHK_CAN_SW_TMR)
			nop();
	    
		/* Set to Operate mode. */
		can_block_p->CTLR.BIT.CANM = CAN_OPERATION;

		/* Ensure that RCAN is in Operation	mode. */
		while ((can_block_p->STR.BIT.RSTST) && DEC_CHK_CAN_SW_TMR)
		{;}
		if (can_tmo_cnt == 0)
			api_status = R_CAN_SW_RST_ERR;
	break;
	default:
		api_status = R_CAN_BAD_ACTION_TYPE;
	break;
	}

	return api_status;
}/* end R_CAN_Control() */

/*******************************************************************************
Function Name:  R_CAN_TxSet
Description:    Set up a CAN mailbox to transmit.
Parameters: 	Channel nr.
				Mailbox nr.
				frame_p - pointer to a data frame structure.
				remote - REMOTE_FRAME to send remote request, DATA_FRAME for 
				sending normal dataframe.
Return value: 	R_CAN_OK			    The mailbox was set up for transmission.
			    R_CAN_SW_BAD_MBX	    Bad mailbox number.
			    R_CAN_BAD_CH_NR 	    The channel number does not exist.
			    R_CAN_BAD_ACTION_TYPE   No such action type exists for this 
									    function.
*******************************************************************************/
uint32_t R_CAN_TxSet(	const uint32_t 			ch_nr, 
						const uint32_t 			mbox_nr,
						const can_std_frame_t* 	frame_p,
						const uint32_t 			frame_type)
{
	uint32_t api_status = R_CAN_OK;
	uint32_t i;
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;

	/* Wait for any previous transmission to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);

	/* Interrupt disable the mailbox.in case it was a receive mailbox */
	can_block_p->MIER &= ~(bit_set[mbox_nr]);

	/* Clear message mailbox control register (trmreq to 0). */
	can_block_p->MCTL[mbox_nr].BYTE = 0;
	
	
	/*** Set Mailbox. ***/
	/* Set CAN message mailbox buffer Standard ID */
	can_block_p->MB[mbox_nr].ID.BIT.SID = frame_p->id;

	/* Set the Data Length Code */
	can_block_p->MB[mbox_nr].DLC.BIT.DLC = frame_p->dlc;
	
 	/* Frame select: Data frame = 0, Remote = 1 */
	if (frame_type == REMOTE_FRAME)
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 1;
	else 
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 0;
	
	/* Frame select: Standard = 0, Extended = 1 */
	can_block_p->MB[mbox_nr].ID.BIT.IDE = 0;

	/* Copy frame data into mailbox */
	for (i = 0; ((i < frame_p->dlc) && (i<8)); i++)
	{
	    can_block_p->MB[mbox_nr].DATA[i] = frame_p->data[i];
	}
	/**********************/

	#ifndef USE_CAN_POLL
		/* Interrupt enable the mailbox */
 		can_block_p->MIER |= (bit_set[mbox_nr]);
	#endif

	R_CAN_Tx(ch_nr, mbox_nr);
	
	return api_status;
} /* end R_CAN_TxSet() */

/*******************************************************************************
Function Name:	R_CAN_Tx
Description:	Starts actual message transmission onto the CAN bus.
Parameters:		Channel nr.
				Mailbox nr.
Return value: 	R_CAN_OK			The mailbox was set to transmit a previously 
								    configured mailbox.
			    R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
			    R_CAN_SW_SET_TX_TMO	Waiting for previous transmission to finish 
								    timed out.
			    R_CAN_SW_SET_RX_TMO	Waiting for previous reception to complete 
								    timed out.
*******************************************************************************/
uint32_t R_CAN_Tx(const uint32_t ch_nr, const uint32_t mbox_nr)
{
	uint32_t api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
    if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;

	/* Wait for any previous transmission to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);

	/* Clear SentData flag since we are about to send anew. */
	CanClearSentData(ch_nr, mbox_nr);
	
	/* Set TrmReq bit to "1" */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ = 1;
	
 	return api_status;
}/* end R_CAN_Tx() */

/*****************************************************************************
Name:			R_CAN_TxCheck
Parameters:		Channel nr.
				Mailbox nr.
Description:  	Use to check a mailbox for a successful data frame transmission.
Return value: 	R_CAN_OK			Transmission was completed successfully.
			    R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
			    R_CAN_MSGLOST		Message was overwritten or lost.
			    R_CAN_NO_SENTDATA	No message was sent.
*****************************************************************************/
uint32_t R_CAN_TxCheck(const uint32_t ch_nr, const uint32_t mbox_nr)
{
    uint32_t api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
	if (ch_nr == 0)
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		return R_CAN_BAD_CH_NR;
	
	if (can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA == 0)
	    api_status = R_CAN_NO_SENTDATA;
    else
	   	/* Clear SentData flag. */
		CanClearSentData(ch_nr, mbox_nr);

    return api_status;
}/* end R_CAN_TxCheck() */

/*****************************************************************************
Name:			R_CAN_TxStopMsg
Parameters:		Channel nr.
				Mailbox nr.
Description:  	Stop a mailbox that has been asked to transmit a frame. If the 
			    message was not stopped, R_CAN_SW_ABORT_ERR is returned. Note 
			    that the cause of this could be that the message was already sent. 
Return value: 	R_CAN_OK		    Action completed successfully.
			    R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
			    R_CAN_SW_ABORT_ERR	Waiting for an abort timed out.
*****************************************************************************/
uint32_t R_CAN_TxStopMsg(const uint32_t ch_nr, const uint32_t mbox_nr)
{
    uint32_t api_status = R_CAN_OK;
	uint32_t can_tmo_cnt = MAX_CAN_SW_DELAY;   
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;
	
	/* Clear message mailbox control register. Setting TRMREQ to 0 should abort. */
	can_block_p->MCTL[mbox_nr].BYTE = 0;

	/* Wait for abort. */
	while ((can_block_p->MCTL[mbox_nr].BIT.TX.TRMABT) && DEC_CHK_CAN_SW_TMR)
	{;}
	if (can_tmo_cnt == 0)
		api_status = R_CAN_SW_ABORT_ERR;
	
	/* Clear abort flag. */
    can_block_p->MCTL[mbox_nr].BIT.TX.TRMABT = 0;
	
    return api_status;
}/* end R_CAN_TxStopMsg() */

/*****************************************************************************
Name:			CanClearSentData
Parameters:		Channel nr.
				Mailbox nr.
Description:  	Use in poll mode for checking successful data frame transmission.
Return value: 	CAN API code (CAN_R_CAN_OK if mailbox has sent.)
*****************************************************************************/
static inline void CanClearSentData(const uint32_t ch_nr, const uint32_t mbox_nr)
{
	volatile struct st_can __evenaccess * can_block_p;
	
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return;

	/* Clear SentData to 0 *after* setting TrmReq to 0. */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ = 0;
	nop();
	can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA = 0;
}/* end CanClearSentData() */   

/*******************************************************************************
Function Name:  R_CAN_RxSet
Description:    Set up a mailbox to receive. The API sets up a given mailbox to 
			    receive dataframes with the given CAN ID. Incoming data frames 
			    with the same ID will be stored in the mailbox. 
Parameters: 	ch_nr
				Mailbox nr.
				frame_p - pointer to a data frame structure.
				remote - REMOTE_FRAME to listen for remote requests, DATA_FRAME
				 for receiving normal dataframes.
Return value: 	R_CAN_OK			Action completed successfully.
			    R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
			    R_CAN_SW_SET_TX_TMO	Waiting for previous transmission to finish 
								    timed out.
			    R_CAN_SW_SET_RX_TMO	Waiting for previous reception to complete 
								    timed out.
*******************************************************************************/
uint32_t R_CAN_RxSet( 	const uint32_t 	ch_nr, 
						const uint32_t 	mbox_nr, 
						const uint32_t 	sid,
						const uint32_t 	frame_type)
{
	uint32_t api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;

	/* Wait for any previous transmission/reception to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);

	/* Interrupt disable the mailbox. */
	can_block_p->MIER &= ~(bit_set[mbox_nr]);

	/* Clear message mailbox control register. */
	can_block_p->MCTL[mbox_nr].BYTE = 0;

	/*** Set Mailbox. ***/
	/* Set mailbox standard ID. */
	can_block_p->MB[mbox_nr].ID.BIT.SID = sid;

 	/* Dataframe = 0, Remote frame = 1	*/
	if (frame_type == REMOTE_FRAME)
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 1;
	else 
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 0;
		
 	/* Frame select: Standard = 0, Extended = 1 */
	can_block_p->MB[mbox_nr].ID.BIT.IDE = 0;
	/********************/

	#ifndef USE_CAN_POLL
		/* Interrupt enable the mailbox */
		can_block_p->MIER |= (bit_set[mbox_nr]);
	#endif

	/* Request to receive the frame with RecReq bit. */
	//can_block_p->MCTL[mbox_nr].BIT.RX.recreq = 1; need to also reset newdata:
    can_block_p->MCTL[mbox_nr].BYTE = 0x40;

	return api_status;
} /* end R_CAN_RxSet() */

/*******************************************************************************
Function Name:	R_CAN_RxPoll
Description:	Checks for received message in mailbox.
Parameters:		Channel nr.
				Mailbox nr.
Return value:	R_CAN_OK			There is a message waiting.
				R_CAN_NOT_OK		No message waiting.
			    R_CAN_RXPOLL_TMO    Message pending but timed out.
				R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
*******************************************************************************/
uint32_t R_CAN_RxPoll(const uint32_t ch_nr, const uint32_t mbox_nr)
{
	uint32_t api_status = R_CAN_NOT_OK;
	uint32_t poll_delay = MAX_CAN_REG_POLLTIME;
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;
 	
	/* Wait if new data is currently being received. */
	while ((can_block_p->MCTL[mbox_nr].BIT.RX.INVALDATA) && poll_delay)
	{
	    poll_delay--;
	}
	if (poll_delay == 0)
	/* Still updating mailbox. Come back later. */
	{
		api_status = R_CAN_RXPOLL_TMO;
	}
    else /* Message received? */
	{
		/* If message received, tell user. */
		if (can_block_p->MCTL[mbox_nr].BIT.RX.NEWDATA == 1)
			api_status = R_CAN_OK;
	}
	return api_status;
}/* end R_CAN_RxPoll() */

/*******************************************************************************
Function Name:  R_CAN_RxRead
Parameters:		Mailbox nr.
				frame_p: Data frame structure
Description:	Call from CAN receive interrupt. Copies received data from 
				message mailbox to memory.
Return value:	R_CAN_OK			There is a message waiting.
			    R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
			    R_CAN_MSGLOST 	 Message was overwritten or lost.
*******************************************************************************/
uint32_t R_CAN_RxRead(	const uint32_t 			ch_nr, 
						const uint32_t 			mbox_nr, 
						can_std_frame_t * const	frame_p	)
{
	uint32_t i;
	uint32_t api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;
	
	/* Copy received data from message mailbox to memory */
	frame_p->id = can_block_p->MB[mbox_nr].ID.BIT.SID;
	frame_p->dlc = can_block_p->MB[mbox_nr].DLC.BIT.DLC;
    for (i = 0; i < can_block_p->MB[mbox_nr].DLC.BIT.DLC; i++)
	{
		frame_p->data[i] = can_block_p->MB[mbox_nr].DATA[i];
	}

	/* Check if message was lost/overwritten. */
	if (can_block_p->MCTL[mbox_nr].BIT.RX.MSGLOST)
	{
		can_block_p->MCTL[mbox_nr].BIT.RX.MSGLOST = 0;
		api_status = R_CAN_MSGLOST;
	}

	/* Set NEWDATA bit to 0 since the mailbox was just emptied and start 
	over with new RxPolls. */
	can_block_p->MCTL[mbox_nr].BIT.RX.NEWDATA = 0;
	
	return api_status;
}/* end R_CAN_RxRead() */

/*******************************************************************************
Function Name:  R_CAN_RxSetMask
Description:    Set a CAN bus mask for specified mask register. Note that the 
			    MKIVLR register is used to disable the acceptance filtering 
			    function individually for each mailbox.
Parameters: 	ch_nr
				SID mask value. For each bit that is 1; corresponding SID bit 
								is compared.
			    mbox_nr			0-31. The mailbox nr translates to mask_reg_nr:
							    0 for mailboxes 0-3
							    1 for mailboxes 4-7
							    2 for mailboxes 8-11
							    3 for mailboxes 12-15
							    4 for mailboxes 16-19
							    5 for mailboxes 20-23
							    6 for mailboxes 24-27
							    7 for mailboxes 28-31
Return value: 	-
*******************************************************************************/
void R_CAN_RxSetMask(	const uint32_t ch_nr, 
						const uint32_t mbox_nr,
						const uint32_t sid_mask_value)
{
	volatile struct st_can __evenaccess * can_block_p;
	
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return;

	/* Write to MKR0 to MKR7 in CAN reset mode or CAN halt mode. */
    R_CAN_Control(0, HALT_CANMODE);
	
	/* Set mask for the goup of mailboxes. */
	can_block_p->MKR[mbox_nr/4].BIT.SID = sid_mask_value;

	/* Set mailbox mask to be used. (0 = mask VALID.) */
   	can_block_p->MKIVLR &= ~(bit_set[mbox_nr]);
	
    R_CAN_Control(0, OPERATE_CANMODE);
			
}/* end R_CAN_RxSetMask() */

/*****************************************************************************
Name:			R_CAN_WaitTxRx
Parameters:		Channel nr.
				Mailbox nr.
Description:  	Wait for communicating mailbox to complete action. This would 
			    be apporopriate for example if a mailbox all of a sudden needs 
			    to be reconfigured but the user wants any pending receive or 
			    transmit to finish.
Return value: 	R_CAN_OK			There is a message waiting.
			    R_CAN_SW_BAD_MBX	Bad mailbox number.
			    R_CAN_BAD_CH_NR     The channel number does not exist.
			    R_CAN_SW_SET_TX_TMO	Waiting for previous transmission to finish 
								    timed out.
			    R_CAN_SW_SET_RX_TMO	Waiting for previous reception to complete 
								    timed out.
*****************************************************************************/
static inline uint32_t R_CAN_WaitTxRx(const uint32_t ch_nr, const uint32_t mbox_nr)
{
	uint32_t api_status = R_CAN_OK;
	uint32_t can_tmo_cnt = MAX_CAN_SW_DELAY;   	
	volatile struct st_can __evenaccess * can_block_p;
	
	CHECK_MBX_NR
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;

	/* Wait for any previous transmission to complete. */
	if (can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ)
	{
		while ((can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA == 1) && DEC_CHK_CAN_SW_TMR) 
		{;}
	    if (can_tmo_cnt == 0) 
			api_status = R_CAN_SW_SET_TX_TMO;
	}
	/* Wait for any previous reception to complete. */
	else if (can_block_p->MCTL[mbox_nr].BIT.RX.RECREQ)
	{
		while ((can_block_p->MCTL[mbox_nr].BIT.RX.INVALDATA == 1) && DEC_CHK_CAN_SW_TMR) 
		{;}
	    if (can_tmo_cnt == 0) 
			api_status = R_CAN_SW_SET_RX_TMO;
	}
	return api_status;
}/* end R_CAN_WaitTxRx() */

/*******************************************************************************
Function Name:  R_CAN_CheckErr
Description:    Checks CAN peripheraol error state.
Parameters:		-
Return value: 	0 = No error
				1 = CAN is in error active state
				2 = CAN is in error passive state
				4 = CAN is in bus-off state
*******************************************************************************/
uint32_t R_CAN_CheckErr(const uint32_t	ch_nr)
{
	uint32_t api_status = R_CAN_STATUS_ERROR_ACTIVE; /* Store return value */
	volatile struct st_can __evenaccess * can_block_p;
	
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return R_CAN_BAD_CH_NR;

	/* Check CAN error state */
	if (can_block_p->STR.BIT.EST)
	{
		/* Check error-passive state */
		if (can_block_p->STR.BIT.EPST)
		{
			api_status = R_CAN_STATUS_ERROR_PASSIVE;
		}

		/* Check bus-off state */
		else if (can_block_p->STR.BIT.BOST)
		{
			api_status = R_CAN_STATUS_BUSOFF;
		}
	}
	
	return api_status;
} /* end R_CAN_CheckErr() */

/*******************************************************************************
Function Name:  R_CAN_SetBitrate
Description:    Sets clock speed and bit rate for CAN as defined in 
			    config.h.
Parameters:	 -
Return value: 	-
*******************************************************************************/
void R_CAN_SetBitrate(const uint32_t ch_nr)
{	
	volatile struct st_can __evenaccess * can_block_p;
	
	if (ch_nr == 0)
		/* Point to CAN0 peripheral block. */
		can_block_p = (struct st_can __evenaccess *) 0x90200;
	else
		/* Point to CAN1 peripheral block. Not implemented this MCU type. */
		return;

	/* Set TSEG1, TSEG2 and SJW. */
	can_block_p->BCR.BIT.BRP = CAN_BRP - 1;
	can_block_p->BCR.BIT.TSEG1 = CAN_TSEG1 - 1;
	can_block_p->BCR.BIT.TSEG2 = CAN_TSEG2 - 1;
	can_block_p->BCR.BIT.SJW = CAN_SJW - 1;
}/* end R_CAN_SetBitrate() */

#ifndef USE_CAN_POLL
#ifdef CAPI_CFG_CAN0_ISR
/**********************************************************************************
Function Name:  R_CAN_ConfigCANinterrupts
Description  :  Configuration of CAN interrupts.
				
			    CAN0 interrupts:
				Source	Name	Vector	Address		IER, BIT		IPR
								nr.		offset
				============================================================
				CAN0 	ERS0 	56 		00E0 		IER07, IEN0		IPR18
				"		RXF0 	57 		00E4 		IER07, IEN1		"
				"		TXM0 	58 		00E8 		IER07, IEN2		"
				"		RXM0 	59 		00EC 		IER07, IEN3		"
				"		TXM0 	60 		00F0 		IER07, IEN4		"
				============================================================
				
				
			    CAN1 interrupts:
			    Source	Name	Vector	Address		IER, BIT		IPR
								nr.		offset
				============================================================
				
Parameters   : 	-
Return value : 	-
***********************************************************************************/
void R_CAN_ConfigCANinterrupts(const uint32_t ch_nr)
{
    if (ch_nr == 0)
	{
		/* Configure CAN Tx interrupt. */
		//ICU.IER[IER_CAN0_TXM0].BIT.IEN4 = 1;
		//ICU.IPR[IPR_CAN0_TXM0].BIT.IPR = 3;
		/* ..or use the macros! */
		IEN(CAN0, TXM0) = 1;	//1 = interrupt enabled.
		IPR(CAN0, TXM0) = CAN0_INT_LVL;	//priority
	
		/* Configure CAN Rx interrupt. */
		IEN(CAN0, RXM0) = 1;
		IPR(CAN0, RXM0) = CAN0_INT_LVL;
	
		/* Configure CAN Error interrupt. */
		IEN(CAN0, ERS0) = 1;
		IPR(CAN0, ERS0) = CAN0_INT_LVL;
		CAN0.EIER.BYTE = 0xFF;

		/* Mailbox interrupt enable registers. Disable interrupts for all slots. 
		They will be enabled individually by the API. */
	    CAN0.MIER = 0x00000000;

		/* RX CAN0 uses:
		- Interrupt Priority Register 18, IPR18. 
		- Interrupt Request Enable Register 7, IER07.
		- Vector 57 RXF0, and 58 TXM0. */
	    ICU.IPR[18].BIT.IPR = CAN0_INT_LVL;
		/* Interrupt enable bit per mailbox (can_block_p->MIER) set by 
		R_CAN_TxSet() and R_CAN_RxSet() */		
	}
}/* end R_CAN_ConfigCANinterrupts() */

/*********************************************************************************

 		Used CAN interrupts are normally in application. Below are templates.

**********************************************************************************/
#if 0
/*****************************************************************************
Name:			CAN0_TXM0_ISR
Parameters:		-
Returns:		-
Description:	CAN0 Transmit interrupt.
				Check which mailbox transmitted data and process it.
*****************************************************************************/
#pragma interrupt CAN0_TXM0_ISR(vect=VECT_CAN0_TXM0, enable) 
void CAN0_TXM0_ISR(void)
{
	uint32_t api_status = R_CAN_OK;

//	api_status = R_CAN_TxCheck(0, CANBOX_TX);
//	if (api_status == R_CAN_OK)
//		can0_tx_sentdata_flag = 1;
	/* Use mailbox search reg. Should be faster than above if a lot of mail-
	boxes to check. Not verified. */
}/* end CAN0_TXM0_ISR() */

/*****************************************************************************
Name:			CAN0_RXM0_ISR
Parameters:		-
Returns:		-
Description:	CAN0 Receive interrupt.
				Check which mailbox received data and process it.
*****************************************************************************/
#pragma interrupt CAN0_RXM0_ISR(vect=VECT_CAN0_RXM0, enable)
void CAN0_RXM0_ISR(void)
{
	/* Use CAN API. */
	uint32_t api_status = R_CAN_OK;

//	api_status = R_CAN_RxPoll(0, CANBOX_RX);
//	if (api_status == R_CAN_OK)
//		can0_rx_newdata_flag = 1;
		
	/* Use mailbox search reg. Should be faster if a lot of mailboxes to check. 
	Not verified. */
}/* end CAN0_RXM0_ISR() */

/*****************************************************************************
Name:			CAN0_ERS0_ISR
Parameters:		-
Returns:		-
Description:	CAN0 Error interrupt.
*****************************************************************************/
#pragma interrupt	CAN0_ERS0_ISR(vect=VECT_CAN0_ERS0, enable)
void CAN0_ERS0_ISR(void)
{
	 nop();
}/* end CAN0_ERS0_ISR() */
#endif //0

/*****************************************************************************
Name:			CAN0_RXF0_ISR
Parameters:		-
Returns:		-
Description:	CAN0 Rx Fifo interrupt.
*****************************************************************************/
#pragma interrupt	CAN0_RXF0_ISR(vect=VECT_CAN0_RXF0, enable)
void CAN0_RXF0_ISR(void)
{
	 nop();
}/* end CAN0_RXF0_ISR() */

/*****************************************************************************
Name:			CAN0_TXF0_ISR
Parameters:		-
Returns:		-
Description:	CAN0 Tx Fifo interrupt.
*****************************************************************************/
#pragma interrupt	CAN0_TXF0_ISR(vect=VECT_CAN0_TXF0, enable)
void CAN0_TXF0_ISR(void)
{
	 nop();
}/* end CAN0_TXF0_ISR() */

#endif //USE_CAN_POLL
#endif //CAPI_CFG_CAN0_ISR


/* can device api */

/* interrupt related flags */
#ifndef USE_CAN_POLL
static uint32_t can0_tx_sentdata_flag = 0;
static uint32_t can0_tx_remote_sentdata_flag = 0;
static uint32_t can0_rx_newdata_flag = 0;
static uint32_t can0_rx_test_newdata_flag = 0;
static uint32_t can0_rx_remote_frame_flag = 0;
#endif /* USE_CAN_POLL */

/* frames */
static can_std_frame_t tx_dataframe;
static can_std_frame_t rx_dataframe;
static can_std_frame_t remote_frame;

/* node CAN state */
static uint32_t error_bus_status;
static uint32_t error_bus_status_prev;

static int setup_can_hardware(void)
{
  /* note: assume HardwareSetup called
   */

  /* Configure mailboxes in Halt mode. */
  if (R_CAN_Control(0, HALT_CANMODE) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_control");
    return -1;
  }

  /* reset bus states */
  error_bus_status = 0;
  error_bus_status_prev = 0;

  /* reset error judge factor and error code registers */
  CAN0.EIFR.BYTE = 0;
  CAN0.ECSR.BYTE = 0;

  /* reset error counters */
  CAN0.RECR = 0;
  CAN0.TECR = 0;

  /* setup receiving mailboxes */
  if (R_CAN_RxSet(0, CANBOX_RX, 0, DATA_FRAME) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_rxset_data");
    return -1;
  }

  if (R_CAN_RxSet(0, CANBOX_REMOTE_RX, 0, REMOTE_FRAME) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_rxset_remote");
    return -1;
  }

  /* receivall mailboxes (mask == 0), ie.:
     if ((frame->sid & mbox->mask) == mbox.id) mbox.data = frame.data
     note that R_CAN_RxSetMask enables CANMODE, so we call those after RxSet.
   */
  R_CAN_RxSetMask(0, CANBOX_RX, 0);
  R_CAN_RxSetMask(0, CANBOX_REMOTE_RX, 0);
		
  /* switch to operate mode. */
  if (R_CAN_Control(0, OPERATE_CANMODE) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_control");
    return -1;
  }

  return 0;
}

can_dev_t* can_open(void)
{
  /* singleton */
  static can_dev_t dev = { 0, 0, 0, 0, 0, 0 };

  /* already initialized */
  if (dev.refn++) return &dev;

  /* initialize CAN hardware
   */

  if (R_CAN_Create(0) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_create");
    return 0;
  }

  if (R_CAN_PortSet(0, ENABLE) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_portset");
    return 0;
  }

  if (setup_can_hardware() == -1)
  {
    return 0;
  }

  /* assign internal fields
   */

  dev.sid = 0x123;
  dev.seq = 0;

  dev.tx_dataframe = &tx_dataframe;
  tx_dataframe.id = dev.sid;
  tx_dataframe.dlc = 8;

  dev.rx_dataframe = &rx_dataframe;
  rx_dataframe.id = 0;

  dev.remote_frame = &remote_frame;
  remote_frame.id = 0;

  return &dev;
}

void can_close(can_dev_t* dev)
{
  /* assume at least one previous can_open */

  if (--dev->refn) return ;

  /* todo: release device */
}

int can_poll_bus(can_dev_t* dev)
{
  can_std_frame_t err_tx_dataframe;
	
  error_bus_status = R_CAN_CheckErr(0);

  /* state has changed */
  if (error_bus_status == error_bus_status_prev)
    return 0;

  switch (error_bus_status)
  {
    /* ACTIVE state */
  case R_CAN_STATUS_ERROR_ACTIVE:
    {
      /* restart from BUSOFF state */
      if (error_bus_status_prev == R_CAN_STATUS_BUSOFF)
      {
	if (setup_can_hardware() == -1)
	  return -1;
      }
      break;
    }

    /* PASSIVE state */
  case R_CAN_STATUS_ERROR_PASSIVE:
    {
      break ;
    }

    /* BUSOFF state */
  case R_CAN_STATUS_BUSOFF:
    {
      break ;
    }

  default:
    {
      break ;
    }
  } /* switch state */

  /* update previous state */
  error_bus_status_prev = error_bus_status;

  /* transmit CAN bus status change */
  err_tx_dataframe.id = 0x700;
  err_tx_dataframe.dlc = 1;
  err_tx_dataframe.data[0] = error_bus_status;
  R_CAN_TxSet(0, CANBOX_ERROR_TX, &err_tx_dataframe, DATA_FRAME);

  return 0;
}


/* CAN low level routines. i2c equivalent here:
   trunk\Info\2010\sbc2410\i2c-testing\master
 */

#if 0 /* kpit gnu compiler uses single byte packing by default */
# define ATTRIBUTE_PACKED
#else
# define ATTRIBUTE_PACKED __attribute__((packed))
#endif


struct can_msg0
{
  uint8_t cmd;
  uint8_t value[2];
} ATTRIBUTE_PACKED;

struct can_msg3
{
  uint8_t cmd;
  uint8_t seq;
  uint16_t value[3];
} ATTRIBUTE_PACKED;


/* CAN io wrappers */

#if 1 /* LITTLE_ENDIAN */

# define mach_to_le(__val) __val
# define le_to_mach(__val) __val

#else

static inline uint16_t mach_to_le(uint16_t val)
{ return (value & 0xff) << 8 | ((value & 0xff00) >> 8); }

static inline uint16_t le_to_mach(uint16_t val)
{ return mach_to_le(val); }

#endif

static int send_frame(can_dev_t* dev)
{
  dev->tx_dataframe->id = dev->sid;
  dev->tx_dataframe->dlc = 8;

  if (R_CAN_TxSet(0, CANBOX_TX, dev->tx_dataframe, DATA_FRAME) != R_CAN_OK)
    return -1;

#ifdef USE_CAN_POLL

#if 1 /* NEEDED */

  /* the documentation suggests calling this function can be
     omitted since we can reasonably assume the message has
     been sent on TxSet success. BUT not checking it, we
     possibly destroy the frame contents before it gets sent
   */

  while (R_CAN_TxCheck(0, CANBOX_TX))
    ;

#endif /* TODO */

#else /* can interrupts */

  while (can0_tx_sentdata_flag == 0)
  {
    can_poll_bus(dev);
  }

  can0_tx_sentdata_flag = 0;

#endif /* USE_CAN_POLL */

  return 0;
}


static int recv_frame(can_dev_t* dev)
{
#ifdef USE_CAN_POLL

  /* assuming 100mhz, wait for approx N x 0.1 secs max */
  unsigned int count = 2000000;

  while (R_CAN_RxPoll(0, CANBOX_RX) != R_CAN_OK)
  {
    if (--count == 0)
      return -1;
  }

  if (R_CAN_RxRead(0, CANBOX_RX, dev->rx_dataframe) != R_CAN_OK)
    return -1;

  return 0;

#else /* can interrupts */

  if (can0_rx_newdata_flag)
  {
    can0_rx_newdata_flag = 0;
	
    if (R_CAN_RxRead(0, CANBOX_RX, dev->rx_dataframe) != R_CAN_OK)
    {
      can0_rx_newdata_flag = 0;
      return -1;
    }
  }

  if (can0_rx_test_newdata_flag)
  {
    can0_rx_test_newdata_flag = 0;
  }

  /* Set up remote reply if remote request came in. */
  if (can0_rx_remote_frame_flag == 1)
  {
    /* should not occur */
    can0_rx_remote_frame_flag = 0;
    R_CAN_TxSet(0, CANBOX_REMOTE_TX, dev->remote_frame, DATA_FRAME);
  }

#endif /* USE_CAN_POLL */
}


uint16_t can_send_msg0(can_dev_t* dev, uint8_t cmd, uint16_t value)
{
#define CONFIG_SEND_TRIALS 5
  unsigned int trials = CONFIG_SEND_TRIALS;

  struct can_msg0* msg;

  for (; trials; --trials)
  {
    msg = (struct can_msg0*)dev->tx_dataframe->data;

    msg->cmd = cmd;
    msg->value[0] = (value & 0xff00) >> 8;
    msg->value[1] = value & 0xff;

    if (send_frame(dev) == -1)
      continue ;

    msg = (struct can_msg0*)dev->rx_dataframe->data;

    /* reset command for further check */
    msg->cmd = 0;

    if (recv_frame(dev) == -1)
      continue ;

    /* not the intended message, redo */
    if (msg->cmd != cmd)
      continue ;

    break;
  }

  return le_to_mach(*(uint16_t*)msg->value);
}


int can_send_msg3(can_dev_t* dev, uint8_t cmd, uint16_t* values)
{
  /* long version */

  unsigned int trials = CONFIG_SEND_TRIALS;
  uint8_t seq;
  struct can_msg3* msg;

  /* pick a sequence number */
  seq = ++dev->seq;

  for (; trials; --trials)
  {
    msg = (struct can_msg3*)dev->tx_dataframe->data;

    msg->cmd = cmd;
    msg->value[0] = mach_to_le(values[0]);
    msg->value[1] = mach_to_le(values[1]);
    msg->value[2] = mach_to_le(values[2]);
    msg->seq = seq;

    if (send_frame(dev) == -1)
      continue ;

    msg = (struct can_msg3*)dev->rx_dataframe->data;

    /* reset command for further check */
    msg->cmd = 0;

    if (recv_frame(dev) == -1)
      continue ;

    if (msg->cmd != cmd)
      continue ;

    break;
  }

  if (trials == 0)
    return -1;

  values[0] = msg->value[0];
  values[1] = msg->value[1];
  values[2] = msg->value[2];

  return 0;
}
