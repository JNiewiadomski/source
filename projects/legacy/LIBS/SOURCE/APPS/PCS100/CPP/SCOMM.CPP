// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <stdio.h>
#include <string.h>

#include <common\common.h>
#include <common\coord.h>
#include <common\jiffy.h>
#include <common\keyboard.h>
#include <common\system.h>

#include <seashell\error.h>
#include <seashell\menu.h>
#include <seashell\printer.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\string.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#include "pcs100.rh"
#include "pcs100.h"
#include "screen.h"
#include "query.h"
#include "scomm.h"


static void	CommAdjustMenu(void);
static void	PrinterAdjustMenu(void);


static void CommAdjustMenu(void) {
	char	msgStr[RES_STR_LEN+1];

	if (equip.serial > 0) {
		sprintf(msgStr, StringGet(CommSTRING, CommSTRING_ComPort), getQueryPort()+1);
		MenuItemSet(PrefsMENU, PrefsMENU_Port, msgStr);
		}
	MenuItemCheck(PortMENU, PortMENU_COM1, (Boolean)(getQueryPort()==COM1));
	MenuItemCheck(PortMENU, PortMENU_COM2, (Boolean)(getQueryPort()==COM2));
	}


void CommBaudDo(short item) {
	switch (item) {
		case BaudMENU_300:
			data.comm.baud = 300;
			break;
		case BaudMENU_600:
			data.comm.baud = 600;
			break;
		case BaudMENU_1200:
			data.comm.baud = 1200;
			break;
		case BaudMENU_2400:
			data.comm.baud = 2400;
			break;
		case BaudMENU_4800:
			data.comm.baud = 4800;
			break;
		case BaudMENU_9600:
			data.comm.baud = 9600;
			break;
		case BaudMENU_19200:
			data.comm.baud = 19200;
			break;
		}
	PrinterAdjustMenu();
	PrinterSelectSerial(data.comm.port, data.comm.baud, data.comm.parity, data.comm.stop, data.comm.data);
	}


void CommDataDo(short item) {
	data.comm.data = item+4;
	PrinterAdjustMenu();
	PrinterSelectSerial(data.comm.port, data.comm.baud, data.comm.parity, data.comm.stop, data.comm.data);
	}


void CommParityDo(short item) {
	switch (item) {
		case ParityMENU_None:
			data.comm.parity = P_NONE;
			break;
		case ParityMENU_Space:
			data.comm.parity = P_S_STICK;
			break;
		case ParityMENU_Odd:
			data.comm.parity = P_ODD;
			break;
		case ParityMENU_Mark:
			data.comm.parity = P_M_STICK;
			break;
		case ParityMENU_Even:
			data.comm.parity = P_EVEN;
			break;
		}
	PrinterAdjustMenu();
	PrinterSelectSerial(data.comm.port, data.comm.baud, data.comm.parity, data.comm.stop, data.comm.data);
	}


void CommPortDo(short item) {
	item--;
	if (item == getQueryPort())
		return;
	reinstallQuery(item);
	if (equip.serial > 1) {
		data.comm.port = (getQueryPort() == COM1) ? COM2 : COM1;
		PrinterAdjustMenu();
		PrinterSelectSerial(data.comm.port, data.comm.baud, data.comm.parity, data.comm.stop, data.comm.data);
		}
	CommAdjustMenu();
	}


void CommSetupDo(short item) {
	switch (item) {
		case SetupMENU_Parallel:
			data.comm.printer = P_Parallel;
			PrinterSelectParallel();
			break;
		case SetupMENU_Serial:
			data.comm.printer = P_Serial;
			PrinterSelectSerial(data.comm.port, data.comm.baud, data.comm.parity, data.comm.stop, data.comm.data);
			break;
		}
	PrinterAdjustMenu();
	}


void CommStopDo(short item) {
	data.comm.stop = item;
	PrinterAdjustMenu();
	PrinterSelectSerial(data.comm.port, data.comm.baud, data.comm.parity, data.comm.stop, data.comm.data);
	}


void CommInit(void) {
	switch (equip.serial) {
		case 0:
			MenuErrorPost(ErrorGet(PCS100_ERROR, PCS100_ERROR_NoSerial), true);
			MenuItemDisable(PrefsMENU, PrefsMENU_Port);
			MenuItemDisable(SetupMENU, SetupMENU_Serial);
			MenuItemDisable(SetupMENU, SetupMENU_Format);
			break;
		case 1:
			MenuItemEnable(PrefsMENU, PrefsMENU_Port);
			MenuItemEnable(PortMENU, PortMENU_COM1);
			MenuItemDisable(PortMENU, PortMENU_COM2);
			data.comm.port = COM1;
			MenuItemDisable(SetupMENU, SetupMENU_Serial);
			MenuItemEnable(SetupMENU, SetupMENU_Format);
			break;
		default:
			MenuItemEnable(PrefsMENU, PrefsMENU_Port);
			MenuItemEnable(PortMENU, PortMENU_COM1);
			MenuItemEnable(PortMENU, PortMENU_COM2);
			MenuItemEnable(SetupMENU, SetupMENU_Serial);
			MenuItemEnable(SetupMENU, SetupMENU_Format);
			break;
		}
	if (equip.parallel > 0)
		MenuItemEnable(SetupMENU, SetupMENU_Parallel);
	else
		MenuItemDisable(SetupMENU, SetupMENU_Parallel);
	if ((data.comm.printer == P_Parallel) && (equip.parallel < 1))
		data.comm.printer = (equip.serial > 1) ? P_Serial : P_None;
	else if ((data.comm.printer == P_Serial) && (equip.serial < 2))
		data.comm.printer = (equip.parallel > 0) ? P_Parallel : P_None;
	switch (data.comm.printer) {
		case P_Parallel:
			PrinterSelectParallel();
			MenuItemEnable(PrefsMENU, PrefsMENU_Printer);
			break;
		case P_Serial:
			PrinterSelectSerial(data.comm.port, data.comm.baud, data.comm.parity, data.comm.stop, data.comm.data);
			MenuItemEnable(PrefsMENU, PrefsMENU_Printer);
			break;
		default:
			MenuItemDisable(PrefsMENU, PrefsMENU_Printer);
			break;
		}
	CommAdjustMenu();
	PrinterAdjustMenu();
	}


void CommInitData(void) {
	if (equip.parallel > 0)
		data.comm.printer = P_Parallel;
	else if (equip.serial > 1)
		data.comm.printer = P_Serial;
	else
		data.comm.printer = P_None;
	data.comm.port = COM2;
	data.comm.baud = 9600;
	data.comm.parity = P_NONE;
	data.comm.data = 8;
	data.comm.stop = 1;
	}


static void PrinterAdjustMenu(void) {
	char	msgStr[RES_STR_LEN+1], parity[RES_STR_LEN+1];

	switch (data.comm.parity) {
		case P_NONE:
			strcpy(parity, StringGet(CommSTRING, CommSTRING_None));
			break;
		case P_ODD:
			strcpy(parity, StringGet(CommSTRING, CommSTRING_Odd));
			break;
		case P_EVEN:
			strcpy(parity, StringGet(CommSTRING, CommSTRING_Even));
			break;
		case P_S_STICK:
			strcpy(parity, StringGet(CommSTRING, CommSTRING_Space));
			break;
		case P_M_STICK:
			strcpy(parity, StringGet(CommSTRING, CommSTRING_Mark));
			break;
		}
	sprintf(msgStr, StringGet(CommSTRING, CommSTRING_Format), data.comm.port+1, data.comm.baud, parity, data.comm.data, data.comm.stop);
	MenuItemSet(SetupMENU, SetupMENU_Format, msgStr);

	MenuItemCheck(SetupMENU, SetupMENU_Parallel, (Boolean)(data.comm.printer==P_Parallel));
	MenuItemCheck(SetupMENU, SetupMENU_Serial, (Boolean)(data.comm.printer==P_Serial));
	sprintf(msgStr, StringGet(CommSTRING, CommSTRING_ComPort), data.comm.port+1);
	MenuItemSet(FormatMENU, FormatMENU_Port, msgStr);

	sprintf(msgStr, StringGet(CommSTRING, CommSTRING_Baud), data.comm.baud);
	MenuItemSet(FormatMENU, FormatMENU_Baud, msgStr);
	MenuItemCheck(BaudMENU, BaudMENU_300, (Boolean)(data.comm.baud==300));
	MenuItemCheck(BaudMENU, BaudMENU_600, (Boolean)(data.comm.baud==600));
	MenuItemCheck(BaudMENU, BaudMENU_1200, (Boolean)(data.comm.baud==1200));
	MenuItemCheck(BaudMENU, BaudMENU_2400, (Boolean)(data.comm.baud==2400));
	MenuItemCheck(BaudMENU, BaudMENU_4800, (Boolean)(data.comm.baud==4800));
	MenuItemCheck(BaudMENU, BaudMENU_9600, (Boolean)(data.comm.baud==9600));
	MenuItemCheck(BaudMENU, BaudMENU_19200, (Boolean)(data.comm.baud==19200));

	sprintf(msgStr, StringGet(CommSTRING, CommSTRING_Parity), parity);
	MenuItemSet(FormatMENU, FormatMENU_Parity, msgStr);
	MenuItemCheck(ParityMENU, ParityMENU_None, (Boolean)(data.comm.parity==P_NONE));
	MenuItemCheck(ParityMENU, ParityMENU_Space, (Boolean)(data.comm.parity==P_S_STICK));
	MenuItemCheck(ParityMENU, ParityMENU_Odd, (Boolean)(data.comm.parity==P_ODD));
	MenuItemCheck(ParityMENU, ParityMENU_Mark, (Boolean)(data.comm.parity==P_M_STICK));
	MenuItemCheck(ParityMENU, ParityMENU_Even, (Boolean)(data.comm.parity==P_EVEN));

	sprintf(msgStr, StringGet(CommSTRING, CommSTRING_Data), data.comm.data);
	MenuItemSet(FormatMENU, FormatMENU_Data, msgStr);
	MenuItemCheck(DataMENU, DataMENU_5, (Boolean)(data.comm.data==5));
	MenuItemCheck(DataMENU, DataMENU_6, (Boolean)(data.comm.data==6));
	MenuItemCheck(DataMENU, DataMENU_7, (Boolean)(data.comm.data==7));
	MenuItemCheck(DataMENU, DataMENU_8, (Boolean)(data.comm.data==8));

	sprintf(msgStr, StringGet(CommSTRING, CommSTRING_Stop), data.comm.stop);
	MenuItemSet(FormatMENU, FormatMENU_Stop, msgStr);
	MenuItemCheck(StopMENU, StopMENU_1, (Boolean)(data.comm.stop==1));
	MenuItemCheck(StopMENU, StopMENU_2, (Boolean)(data.comm.stop==2));
	}
