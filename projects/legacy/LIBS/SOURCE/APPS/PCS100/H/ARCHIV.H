// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

void    ArchiveSave(void);
void    ArchiveInit(void);

void	GenerateDo(short item);

Boolean existArchJob(Byte adr, DWord jobN);
Boolean existArchShift(Byte adr, DWord jobN, Byte shiftN);
Boolean existClosedJob(DWord jobNumber);
void    saveArchJob(QueryJobStruct *qj);
void    saveArchShift(QueryShiftStruct *qs);
void    saveClosedJob(DWord jobNumber);

void deleteArchives(void);
void transferArchJob(char *name, DWord jobNumber);
void transferArchShift(char *name, DWord jobNumber);

void exportOpenArchJobs(char *name);
void exportOpenArchShifts(char *name);
void exportClosedArchJobs(char *nameFrom, char *nameTo);
void exportClosedArchShifts(char *nameFrom, char *nameTo);
