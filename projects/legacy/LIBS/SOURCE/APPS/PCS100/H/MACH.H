// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#ifdef DEMO
#	define MAX_MACHINES	2
#else
#	define MAX_MACHINES	100
#endif

#define MCH_NONE	0x7FFF

#define MACH_CNTR_LEN	2
#define MACH_NAME_LEN	30

struct MachineStruct {
	short	counter;
	short	class_type;
	char	name[MACH_NAME_LEN+1];
	Word	date;
	};

extern short		mchMax;
extern MachineStruct	*mach[];

void	MachineAdjustMenu(void);
void	MachineAllScreenUpdate(short counter);
void	MachineCleanUp(void);
void	MachineDoMenu(short item);
short	MachineFind(short counter);
void	MachineInit(void);
Boolean	MachineIsAllDisplayed(short counter);
Boolean	MachineIsSingleDisplayed(short counter);
void	MachineScreenDoAll(int ch, QTYPE type);
void	MachineScreenDoSingle(int ch, QTYPE type);
void	MachineShiftAllScreenUpdate(short counter);
void	MachineShiftSingleScreenUpdate(short counter);
void	MachineSingleScreenUpdate(short counter);
void	MachineSubscribe(void);

void	MachineScreenPrintAll(void);
void	MachineScreenPrintSingle(void);
void	printShiftAllMchScreen(void);
void	printShiftSingleMchScreen(void);

void	MachineShiftAllMonitor(void);
void	MachineShiftSingleMonitor(void);

void	OperatorCleanUp(void);
void	OperatorDoMenu(short item);
void	OperatorInit(void);
void	OperatorSubscribe(void);

