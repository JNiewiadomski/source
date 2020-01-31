#include <bios.h>
#include <stdarg.h>
#include <stdio.h>

#include <common\common.h>
#include <common\old\colors.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\color.h>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\printer.h>
#include <seashell\resource.h>
#include <seashell\string.h>
#include <seashell\video.h>
#include <seashell\window.h>

#define ON		1

#define PSTAT_TIMEOUT	0x01
#define PSTAT_IOERR	0x08
#define PSTAT_SELECT	0x10
#define PSTAT_NOPAPER	0x20
#define PSTAT_ACK	0x40

#define SERIAL_MODE	(ASOUT | ASCII | NORMALRX)
#define SERIAL_RXLEN	MIN_BUF_SIZE
#define SERIAL_TXLEN	256

#define PRINT_LEN	255

static short	pVirt, pLow, pHigh;
static Boolean	pOk=False;
static P_Type	pSelect=P_None;
static int	printHotKey, pPort, pBaud, pParity, pData, pStop;

static void	openParallelPrinter(void);
static void	openSerialPrinter(void);


// PrinterClose -- call this routine when finished printing.  The dialog
// box is removed and the screen is restored to the way it looked before
// PrinterOpen was called.

void PrinterClose(void) {
//	if (pSelect == P_Serial)
//		asiquit(pPort);
	VirtualDispose(pVirt);
	pOk = False;
	}


int PrinterGetKey(void) {
	return (printHotKey);
	}


// PrinterInit -- call this routine once before using any other routines
// from the printer manager.  This routine makes PRINTER_NONE the default
// printer.  You must call either PrinterSelectParallel() or
// PrinterSelectSerial() to designate which printer will be used.

void PrinterInit(int key) {
	pLow = ColorGet(_PrinterCOLOR, _COLOR_Low);
	pHigh = ColorGet(_PrinterCOLOR, _COLOR_High);
	printHotKey = key;
	}


// PrinterOpen -- call this routine when ready to print.  The routine
// displays a dialog box showing the printer status on the screen and
// returns True if the printer path has been successfully opened.  If
// neither the Parallel nor Serial printer has been selected,
// PrinterOpen returns False.

Boolean PrinterOpen(void) {
	Rect	r;
	C_Event	event;

	if (pOk)
		return (pOk);
	pVirt = VirtualGetNew(_PrinterWINDOW, &r);
	VirtualFill(pVirt, GIANT_RECT, ' ', pLow);
	switch (pSelect) {
		case P_Parallel:
			openParallelPrinter();
			break;
		case P_Serial:
			openSerialPrinter();
			break;
		default:
			VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Default), pHigh);
			VirtualWrite(pVirt, 2, 1, GIANT, StringGet(_GlobalSTRING, _GlobalSTRING_PressAny), pHigh);
			ScreenSynch();
			ErrorSound();
			event.WaitKey();
			PrinterClose();
			break;
		}
	ScreenSynch();
	return (pOk);
	}


// openParallelPrinter -- this routine is called internally to prepare the
// parallel printer for printing.

static void openParallelPrinter(void) {
	int	status;
	C_Event	event;

	pOk = False;
	status = biosprint(2, 0, 0);
	if (status & PSTAT_TIMEOUT)
		VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Timeout), pHigh);
	else if (!(status & PSTAT_SELECT))
		VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Off), pHigh);
	else if (status & PSTAT_NOPAPER)
		VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Paper), pHigh);
	else if (status & PSTAT_IOERR)
		VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Offline), pHigh);
	else
		pOk = True;
	if (pOk) {
		VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Parallel), pHigh);
		VirtualWrite(pVirt, 2, 1, GIANT, StringGet(_GlobalSTRING, _GlobalSTRING_EscCancel), pHigh);
		}
	else {
		VirtualWrite(pVirt, 2, 1, GIANT, StringGet(_GlobalSTRING, _GlobalSTRING_PressAny), pHigh);
		ScreenSynch();
		ErrorSound();
		event.WaitKey();
		PrinterClose();
		}
	}


// openSerialPrinter -- this routine is called internally to prepare the
// serial printer for printing.

static void openSerialPrinter(void) {
	C_Event	event;

//	if (asiopen(pPort, SERIAL_MODE, SERIAL_RXLEN, SERIAL_TXLEN, pBaud, pParity, pStop, pData, ON, ON)==ASSUCCESS) {
//		asicheck(pPort, IGCTS, ON);
//		VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Serial), pHigh);
//		VirtualWrite(pVirt, 2, 1, GIANT, StringGet(_GlobalSTRING, _GlobalSTRING_EscCancel), pHigh);
//		pOk = True;
//		}
//	else {
		VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Avail), pHigh);
		VirtualWrite(pVirt, 2, 1, GIANT, StringGet(_GlobalSTRING, _GlobalSTRING_PressAny), pHigh);
		ScreenSynch();
		ErrorSound();
		event.WaitKey();
		PrinterClose();
//		}
	}


// PrinterSelectParallel -- call this routine to make the Parallel printer
// the default printer.  This routine will do nothing and return False if
// a printer is currently opened.  Otherwise it will make the Parallel
// printer the default printer and return True.

Boolean PrinterSelectParallel(void) {
	if (!pOk)
		pSelect = P_Parallel;
	return (pOk ? False : True);
	}


// PrinterSelectSerial -- call this routine to make the Serial printer the
// default printer.  This routine will do nothing and return False if a
// printer is currently opened.  Otherwise it will make the Serial printer
// the default printer and return True.

Boolean PrinterSelectSerial(int port, int baud, int parity, int stop, int data) {
	if (!pOk) {
		pPort = port;
		pBaud = baud;
		pParity = parity;
		pStop = stop;
		pData = data;
		pSelect = P_Serial;
		}
	return (pOk ? False : True);
	}


void PrinterSetKey(int key) {
	printHotKey = key;
	}


// PrinterWrite -- call PrinterWrite with the character string that you
// wish to print.  PrinterWrite returns True if the entire string was
// successfully printed and the <Esc> key was not pressed.  Strings of
// no more than PRINT_LEN length will be printed.

Boolean PrinterWrite(char *format, ...) {
	va_list	ptr;
	char	pStr[PRINT_LEN+1], *pp;
	int	status;
	C_Event	event;

	if (!pOk)
		return (False);
	va_start(ptr, format);
	vsprintf(pStr, format, ptr);
	va_end(ptr);
	*(pStr+PRINT_LEN) = '\0';
	pp = pStr;
	if (event.GetNext(EVT_EVERY) && event.What()==EVT_KEY && event.Message()==kK_ESCAPE) {
//		if (pSelect == P_Serial)
//			asiputc(pPort, '\f');
//		else
			biosprint(0, '\f', 0);
		pOk = False;
		}
	while (*pp && pOk) {
//		if (pSelect == P_Serial) {
//			if (asiputc(pPort, *pp) == ASSUCCESS)
//				pp++;
//			}
//		else if (pSelect == P_Parallel) {
		if (pSelect == P_Parallel) {
			pOk = False;
			status = biosprint(0, *pp, 0);
			if (status & PSTAT_TIMEOUT)
				VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Timeout), pHigh);
			else if (!(status & PSTAT_SELECT))
				VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Off), pHigh);
			else if (status & PSTAT_NOPAPER)
				VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Paper), pHigh);
			else if (status & PSTAT_IOERR)
				VirtualWrite(pVirt, 0, 1, GIANT, StringGet(_PrinterSTRING, _PrinterSTRING_Offline), pHigh);
			else
				pOk = True;
			if (pOk)
				pp++;
			else {
				VirtualWrite(pVirt, 2, 1, GIANT, StringGet(_GlobalSTRING, _GlobalSTRING_PressAny), pHigh);
				ScreenSynch();
				ErrorSound();
				event.WaitKey();
				}
			}
		}
	return (pOk);
	}
