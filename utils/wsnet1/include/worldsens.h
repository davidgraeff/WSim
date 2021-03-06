/*
 *  worldsens.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */

/**
 * Units:
 *   - Frequency: MHz
 *   - Time: ns
 *   - Power: dBm
 **/

#ifndef _WORLDSENS_H
#define _WORLDSENS_H

#ifndef SOL_IP
#define SOL_IP 0
#endif //SOL_IP

#ifndef UNUSED
#define UNUSED __attribute__((unused))  
#endif //UNUSED

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sched.h>
#include <sys/signal.h>
#include <time.h>

#include "worldsens_debug.h"
#include "libtracer/tracer.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/* Maximum worldsens packet length */
#define WORLDSENS_MAX_PKTLENGTH		2000

/* One rp per second */
extern  uint32_t WORLDSENS_SYNCH_PERIOD;

/* Client update frequency */
#define WORLDSENS_UPDATE_PERIOD		1000000

/* Control packet type */
#define WORLDSENS_C_CONNECT		0x01
#define WORLDSENS_C_SYNCHED		0x02
#define WORLDSENS_C_TX			0x04
#define WORLDSENS_C_DISCONNECT		0x08

#define WORLDSENS_S_ATTRADDR		0x01
#define WORLDSENS_S_NOATTRADDR		0x02
#define WORLDSENS_S_SYNCH_REQ		0x04
#define WORLDSENS_S_BACKTRACK		0x08
#define WORLDSENS_S_RX			0x10

  /* TRACER 
   *
   * 1: RX connect         from node
   * 2: TX attach          to node
   * 3: RX Synch           from node
   * 4: TX Next RP         to node
   * 5: BackTrack          to node
   * 6: RX data from node  from node
   * 7: TX data to node    to node
   */

/* from node */

#define TRACER_NODE_CONNECT     0x00010100
#define TRACER_NODE_DISCONNECT  0x0200
#define TRACER_NODE_TX          0x0300
#define TRACER_NODE_SYNCHED     0x0400 /* + data */

#define TRACER_NODE_ATTRADDR    0x1100
#define TRACER_NODE_NOATTRADDR  0x1200
#define TRACER_NODE_SYNCH_REQ   0x1300
#define TRACER_NODE_BACKTRACK   0x1500
#define TRACER_NODE_RX          0x1500 /* + data */

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

extern uint16_t	g_lport;
extern uint16_t	g_mport;
extern char *	g_maddr;
extern int      simulation_keeps_going;

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

typedef void (*callback_rx_t) (void *arg, uint8_t data, 
			       uint32_t frequency, int modulation, 
			       double rx_mW, double SiNR);


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define PACKED __attribute__ ((packed))

struct PACKED _worldsens_c_connect_pkt 
{
  char type;
  int  node;
};


struct PACKED _worldsens_c_disconnect_pkt 
{
  char type;
  int  node;
};


struct PACKED _worldsens_c_synched_pkt 
{
  char type;
  int  rp_seq;
};


struct PACKED _worldsens_c_tx_pkt 
{
  char	   type;
  int	   node;
  uint64_t period;
  uint64_t duration;
  uint32_t frequency;
  int	   modulation;
/* To be more portable double value are stored into uint64_t variables to be swap and unswap */
/* To get their floating values, a cast pointer must be used : (double *) xxx */
  uint64_t tx_mW;       /* double */
  int	   pkt_seq;
  char	   data;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


struct PACKED _worldsens_s_header 
{
	char	   type;
	int	   pkt_seq;
};


struct PACKED  _worldsens_s_connect_pkt 
{
  char	   type;
  int	   pkt_seq;
  uint64_t period;
  int	   rp_seq;
  uint64_t cnx_time;
};


struct PACKED _worldsens_s_backtrack_pkt 
{
  char	   type;
  int	   pkt_seq;
  uint64_t period;
  int	   rp_seq;
};


struct PACKED _worldsens_s_saverel_pkt 
{
  char	   type;
  int	   pkt_seq;
  int	   c_rp_seq;
  uint64_t period;
  int	   n_rp_seq;
};


struct PACKED _worldsens_s_srrx_pkt 
{
  char	   type;
  int	   pkt_seq;
  int	   c_rp_seq;
  uint64_t period;
  int	   n_rp_seq;
  int	   size;
  int	   node;
  uint32_t frequency;
  int	   modulation;
};


struct PACKED _worldsens_s_rx_pkt 
{
  char	   type;
  int	   pkt_seq;
  int	   size;
  int	   node;
  uint32_t frequency;
  int	   modulation;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**
 * Worldsens Client Side
 * 
 **/
struct _worldsens_c 
{
  int		 u_fd;
  int		 m_fd;
  int		 my_addr;
  int		 pkt_seq;
  int		 my_pkt_seq;
  callback_rx_t	 callback_rx;
  void *	 arg;
  int		 rp_seq;
  uint64_t	 next_rp;
  uint64_t	 last_rp;
  uint64_t	 n_update;
  char		 pending;
  char		 tx_backtracked;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**
 * Worldsens Server Side
 *
 **/
struct _worldsens_s 
{
  int		     mfd;          /* multicast socket                          */
  struct sockaddr_in maddr;        /* multicast address                         */ 
  uint64_t	     rp;           /* Rendez-vous point :: nanosecond           */
  int		     rp_seq;       /* Rendez-vous point :: index                */
  char		     synched;      /* number of synchronizes nodes on a barrier */
  tracer_id_t        trc_mcast_rx; /*                                           */
  tracer_id_t        trc_mcast_tx; /*                                           */
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

struct PACKED _worldsens_data 
{
  int		     node;
  char		     data;
/* To be more portable double value are stored into uint64_t variables to be swap and unswap */
/* To get their floating values, a cast pointer must be used : (double *) xxx */
  uint64_t	     rx_mW; /* double */
  uint64_t	     SiNR;  /* double */
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_c_initialize (struct _worldsens_c *worldsens, 
			    char *_s_addr, uint16_t s_port, 		 
			    char *m_addr, uint16_t m_port,
			    int my_addr,
			    callback_rx_t callback_rx, 
			    void *arg);
int worldsens_c_connect    (struct _worldsens_c *worldsens);
int worldsens_c_disconnect (struct _worldsens_c *worldsens);
int worldsens_c_synched	   (struct _worldsens_c *worldsens);
int worldsens_c_parse      (struct _worldsens_c *worldsens, char *msg, int len);
int worldsens_c_clean      (struct _worldsens_c *worldsens);

int worldsens_c_tx         (struct _worldsens_c *worldsens, char data, 
			    int frequency, int modulation, 
			    double tx_mW, uint64_t duration);

int worldsens_c_update     (struct _worldsens_c *worldsens);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int worldsens_s_initialize              (struct _worldsens_s *worldsens);
int worldsens_s_connect                 (struct _worldsens_s *worldsens, 
					 struct sockaddr_in *addr, 
					 char *msg, int len);
int worldsens_s_disconnect              (struct _worldsens_s *worldsens, 
					 struct sockaddr_in *addr, 
					 char *msg, int len);
int worldsens_s_clean                   (struct _worldsens_s *worldsens);

int worldsens_s_listen_to_next_rp       (struct _worldsens_s *worldsens);
int worldsens_s_backtrack_async         (struct _worldsens_s *worldsens, uint64_t period);
int worldsens_s_save_release_request    (struct _worldsens_s *worldsens, uint64_t period);
int worldsens_s_save_release_request_rx	(struct _worldsens_s *worldsens, int node,  
					 int radio, int modulation,  
					 struct _worldsens_data *data, uint64_t period);
int worldsens_s_rx                      (struct _worldsens_s *worldsens, int node,
					 int radio,  int modulation, 
					 struct _worldsens_data *data);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#endif
