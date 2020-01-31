// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <common\common.h>
#include <common\jiffy.h>

#include <seashell\menu.h>

#include "pcs100.rh"
#include "pcs100.h"
#include "screen.h"
#include "mach.h"
#include "job.h"
#include "shift.h"

void ShiftAdjustMenu(void) {
	if (mchMax > 0) {
		MenuItemEnable(ShiftMENU, ShiftMENU_AllMachines);
		MenuItemEnable(ShiftMENU, ShiftMENU_SingleMachine);
		}
	else {
		MenuItemDisable(ShiftMENU, ShiftMENU_AllMachines);
		MenuItemDisable(ShiftMENU, ShiftMENU_SingleMachine);
		}
	if (jobMax > 0) {
		if (mchMax > 0) {
			MenuItemEnable(ShiftMENU, ShiftMENU_AllJobs);
			MenuItemEnable(ShiftMENU, ShiftMENU_SingleJob);
			}
		else {
			MenuItemDisable(ShiftMENU, ShiftMENU_AllJobs);
			MenuItemDisable(ShiftMENU, ShiftMENU_SingleJob);
			}
		}
	else {
		MenuItemDisable(ShiftMENU, ShiftMENU_AllJobs);
		MenuItemDisable(ShiftMENU, ShiftMENU_SingleJob);
		}
	}


void ShiftDoMenu(short item) {
	switch (item) {
		case ShiftMENU_AllMachines:
			MachineShiftAllMonitor();
			break;
		case ShiftMENU_SingleMachine:
			MachineShiftSingleMonitor();
			break;
		case ShiftMENU_AllJobs:
			monitorShiftAllJobs();
			break;
		case ShiftMENU_SingleJob:
			monitorShiftSingleJob();
			break;
		}
	}
