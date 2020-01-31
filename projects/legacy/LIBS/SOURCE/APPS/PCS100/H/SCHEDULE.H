// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#define SCHEDULE_MAX	MAX_MACHINES

enum Priority { Pri_Low, Pri_High };

class Schedule {
	Jiff	time[SCHEDULE_MAX], oldTime, hiPri, loPri;
	Byte	counter[SCHEDULE_MAX];
	short	num;

	Jiff	Time(void);
public:
	// constructors
	Schedule(Jiff hiPriority, Jiff loPriority);

	// methods
	void	Clear(void);
	Boolean	GetNext(Byte *addr);
	Boolean	Insert(Byte addr, Priority pri);
	Boolean	Remove(Byte addr);
	void	SetPriority(Jiff hiPriority, Jiff loPriority);
	};
