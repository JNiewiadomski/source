// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <common\common.h>

#include "pcs100.rh"
#include "screen.h"
#include "query.h"
#include "archiv.h"
#include "report.h"

void DoReportMenu(short item) {
	switch (item) {
		case ReportMENU_Print:
			ScreenPrint();
			break;
		case ReportMENU_Delete:
			deleteArchives();
			break;
		}
	}
