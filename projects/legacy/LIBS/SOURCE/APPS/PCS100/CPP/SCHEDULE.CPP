// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <mem.h>

#include <common\common.h>
#include <common\jiffy.h>

#include "screen.h"
#include "mach.h"
#include "schedule.h"


Schedule::Schedule(Jiff hiPriority, Jiff loPriority) {
	oldTime = 0L;
	hiPri = hiPriority;
	loPri = loPriority;
	num = 0;
	}


void Schedule::Clear(void) {
	num = 0;
	}


Boolean Schedule::GetNext(Byte *addr) {
	if (num == 0 || Time() < time[0])
		return (false);
	*addr = counter[0];
	num--;
	memmove(&(time[0]), &(time[1]), sizeof(time[0])*num);
	memmove(&(counter[0]), &(counter[1]), sizeof(counter[0])*num);
	return (true);
	}


Boolean Schedule::Insert(Byte addr, Priority pri) {
	Jiff	newTime;
	short	index;

	if (num == SCHEDULE_MAX)
		return (false);
	newTime = Time() + ((pri==Pri_Low) ? loPri : hiPri);
	for (index=num; (index>0) && (newTime<time[index-1]); index--)
		;
	if (index < num) {
		memmove(&(time[index+1]), &(time[index]), sizeof(time[0])*(num-index));
		memmove(&(counter[index+1]), &(counter[index]), sizeof(counter[0])*(num-index));
		}
	time[index] = newTime;
	counter[index] = addr;
	num++;
	return (true);
	}


Boolean Schedule::Remove(Byte addr) {
	short i;

	for (i=0; i<num; i++)
		if (counter[i] == addr) {
			num--;
			if (i < num) {
				memmove(&(time[i]), &(time[i+1]), sizeof(time[0])*(num-i));
				memmove(&(counter[i]), &(counter[i+1]), sizeof(counter[0])*(num-i));
				}
			return (true);
			}
	return (false);
	}


void Schedule::SetPriority(Jiff hiPriority, Jiff loPriority) {
	hiPri = hiPriority;
	loPri = loPriority;
	}


Jiff Schedule::Time(void) {
	Jiff	curTime;
	short	i;

	curTime = JiffyTime();
	if (curTime < oldTime) {
		oldTime = 0L;
		for (i=0; i<num; i++)
			time[i] = 0L;
		}
	oldTime = curTime;
	return (curTime);
	}
