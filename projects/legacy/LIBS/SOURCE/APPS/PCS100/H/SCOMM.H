// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

void	CommInit(void);
void	CommInitData(void);
void	CommBaudDo(short item);
void	CommDataDo(short item);
void	CommParityDo(short item);
void	CommPortDo(short item);
void	CommSetupDo(short item);
void	CommStopDo(short item);
