// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

void	GraphAdjustMenu(void);
void	GraphDoMenu(short item);
void	GraphInit(void);

void	GraphAdjustPtrs(short counter);
void	GraphDoScreen(int ch);
void	GraphInitScreen(void);
Boolean	GraphIsDisplayed(short counter);
void	GraphPercComplete(void);
void	GraphRates(void);
void	GraphUpdate(short counter);
