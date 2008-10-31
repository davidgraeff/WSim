
/**
 *  \file   main.c
 *  \brief  WSim simulator entry point
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "machine/machine.h"
#include "libconsole/console.h"
#include "libetrace/libetrace.h"
#include "libgdb/libgdb.h"
#include "libgui/ui.h"
#include "libwsnet/libwsnet.h"
#include "src/options.h"

/* this needs to appear after ui.h which include SDL.h */
#if defined(FUNC_GETRUSAGE_DEFINED)
#include <sys/resource.h>
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void signal_quit(int signum)
{
  ERROR("wsim: received Unix signal %d (%s)\n",signum,host_signal_str(signum));
  mcu_signal_add(SIG_HOST | signum);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void main_dump_stats()
{
#define NANO  (1000*1000*1000)
#define MICRO (1000)
  int64_t unanotime = -1;
  /*  int64_t snanotime = -1; */

#if defined(FUNC_GETRUSAGE_DEFINED)
  struct rusage ru;
  getrusage(RUSAGE_SELF,&ru);
  /* explicit cast to prevent overflow */
  /* utime : user time                 */
  /* stime : system time               */
  unanotime = ((uint64_t)ru.ru_utime.tv_sec) * NANO + ((uint64_t)ru.ru_utime.tv_usec) * MICRO;
  /*  snanotime = ((uint64_t)ru.ru_stime.tv_sec) * NANO + ((uint64_t)ru.ru_stime.tv_usec) * MICRO; */
#endif


  OUTPUT("\n");
  OUTPUT("================\n");
  OUTPUT("WSim stats:\n");
  OUTPUT("-----------\n");
  if (unanotime > 0)
    {
      OUTPUT("  simulation user time          : %d.%03d s (%"PRId64" ns)\n", 
	     unanotime / NANO, unanotime / 1000000, unanotime);
      /* 
       * OUTPUT("  system simulation time        : %d.%03d s\n", 
       *	 ru.ru_stime.tv_sec, ru.ru_stime.tv_usec / 1000); 
       */
    }
  OUTPUT("  simulation backtracks         : %d\n",machine.backtrack); 

  OUTPUT("\n");
  machine_dump_stats(unanotime); /* system time */
  OUTPUT("================\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void  main_run_mode(struct options_t* o)
{
#if !defined(__MINGW32__)
  signal(SIGQUIT,signal_quit);
  signal(SIGUSR1,signal_quit);
  signal(SIGUSR2,signal_quit);
  signal(SIGPIPE,signal_quit); 
#endif

  machine_reset();
  machine_state_save();

  /* so far so good, run() */
  switch (o->sim_mode)
    {
    case SIM_MODE_CONS:
      console_mode_main();
      break;
    case SIM_MODE_GDB:
      libgdb_target_mode_main(o->gdb_port);
      break;

    case SIM_MODE_RUN:
      machine_run_free();
      main_dump_stats();
      break;
    case SIM_MODE_INSN:
      machine_run_insn(o->sim_insn);
      main_dump_stats();
      break;
    case SIM_MODE_TIME:
      machine_run_time(o->sim_time);
      main_dump_stats();
      break;

    default:
      ERROR("** Run mode error\n");
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * main : program entry point
 **/
int main(int argc, char* argv[])
{
  struct options_t o;

  /* options */
  options_start();
  ui_options_add();
  worldsens_c_options_add();
  machine_options_add();
  options_read_cmdline(&o,&argc,argv);

  /* logger creation                              */
  /* do not use logger functions before that line */
  logger_init(o.logfilename,o.verbose);

  OUTPUT("WSim %s, copyright 2005, 2006, 2007, 2008\n",PACKAGE_VERSION);
  OUTPUT("Laboratoire Citi, INRIA, INSA de Lyon\n");
  VERBOSE(2,"A. Fraboulet, G. Chelius, E. Fleury\n");
  VERBOSE(2,"wsim:pid:%d\n",getpid());

  switch (sizeof(long))
    {
    case 4:
      VERBOSE(2,"wsim: 32 bits edition\n");
      break;
    case 8:
      VERBOSE(2,"wsim: 64 bits edition\n");
      break;
    default:
      VERBOSE(2,"wsim: alien edition\n");
      break;
    }

  if (o.verbose > 1)
    {
      options_print_params(&o);
    }


  /* event tracer */
  tracer_init(o.tracefile,machine_get_nanotime);

  /* libselect init  */
  libselect_init();

  /* etrace */
  etracer_init(o.etracefile);

  /* worldsens connect */
  worldsens_c_initialize();

  /* machine creation */
  if (machine_create())
    {
      ERROR("\n");
      ERROR("wsim: ** error during machine creation **\n"); 
      ERROR("\n");
      return 1;
    }

  /* elf loading */
  if (o.progname)
    {
      if (machine_load_elf(o.progname,o.verbose))
	{
	  machine_delete();
	  return 1;
	}
    }
  else if (o.sim_mode == SIM_MODE_GDB)
    {
      fprintf(stderr,"== Running in GDB mode, not loading Elf file\n");
    }
  else 
    {
      ERROR("** Cannot load file, bailing out\n");
      machine_delete();
      exit(0);
    }

  /* what has been created so far */
  if (o.verbose > 1)
    {
      machine_print_description();
    }

  /* GUI */
  if (ui_create(machine.ui.width,machine.ui.height,machine_get_node_id()) != UI_OK)
    {
      ERROR("Cannot create displayi\n");
      return 3;
    }

  /* tracer creation */
  if (o.do_trace)
    {
      VERBOSE(2,"wsim: starting tracer\n");
      tracer_start();
    }

  if (o.do_etrace_at_begin)
    {
      VERBOSE(2,"wsim: starting eSimu tracer at wsim start\n");
      etracer_start(); 
    }

  /* go */
  main_run_mode(&o);

  /* simulation done */
  if (o.do_dump)
    {
      VERBOSE(2,"dumper: dump machine state in [%s]\n",o.dumpfile);
      machine_dump(o.dumpfile);
    }

  /* finishing traces */
  if (o.do_trace)
    {
      VERBOSE(2,"tracer: finalize trace in [%s]\n",o.tracefile);
      tracer_close();
    }

  if (o.do_etrace)
    {
      VERBOSE(2,"tracer: finalize eTrace in [%s]\n",o.etracefile);
      etracer_close();
    }

  ui_delete();
  machine_delete();
  /* close worldsens */
  worldsens_c_close();
  libselect_close();
  logger_close();
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/
