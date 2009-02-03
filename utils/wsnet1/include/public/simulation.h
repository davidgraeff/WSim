/*
 *  simulation.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <public/types.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _worldsens_s;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern	uint64_t g_time;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern double g_x;
extern double g_y;
extern double g_z;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
extern int g_m_nodes;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int simulation_init (int arg, char * argv[]);
int simulation_start (void);


#endif //_SIMULATION_H
