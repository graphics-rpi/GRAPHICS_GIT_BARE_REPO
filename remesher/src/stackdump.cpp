/* Modified by Derek Bruening to not need the executable name
 * hardcoded, and to handle core.pid filenames
 */
/*-----------------------------------------------------------------------*/
/* StackDump.c: written by David Spuler */
/*-----------------------------------------------------------------------*/
/*
  Very useful hack for UNIX stack traces suggested by Castor Fu:
  fork off a process which core dumps then run a debugger on it.
*/
/*-----------------------------------------------------------------------*/

#include <cstdio>
#include <cstdlib>
//Removing to make windows compatible
//#include <unistd.h>
#include <fcntl.h>
//#include <sys/file.h>
//#include <sys/wait.h>
#include <cstring>

/*-----------------------------------------------------------------------*/


#ifdef LINUX
#define DEBUGGER "gdb"
/* add -q to suppress gdb copyright notice */
#define QUIET_MODE "-q"
/* can dynamically determine executable name */
#else
#define DEBUGGER "dbx"
#define QUIET_MODE ""
/* must hardcode executable name */
//extern const char *EXECUTABLE_NAME;
const char *EXECUTABLE_NAME = "voronoi";
#endif

#define TEMPORARY_FILENAME "/tmp/voronoi.stackdump"
#define DEBUGGER_COMMAND "where\nquit\n"
#define CORE_NAME "core"

#include "stackdump.h"
//extern int global_done;

void assert_stackdump_func(const char *exp, const char *file, int line) {
  printf("Assertion failure %s @ %s:%d\n", exp, file, line);
  fflush(stdout);
  StackDump();
  fflush(stdout);
  //global_done = 1;
  exit(1);
}

/*-----------------------------------------------------------------------*/
/* StackDump(): print out a stack trace */
/*-----------------------------------------------------------------------*/
/* Procedure:
   1. Fork off a child process that dumps core; creates the "core" file
   2. Fork off a 2nd child process which:
   2a. Creates a temporary file of input commands for the debugger
   2b. Redirects stdin from this temporary file
   2c. Executes the debugger using the redirected input commands
*/
/*-----------------------------------------------------------------------*/

#ifdef LINUX
static void 
getnamefrompid(int pid, char *name)
{
  int fd,n;
  char tempstring[256],*lastpart;
  sprintf(tempstring,"/proc/%d/cmdline",pid);
  fd = open(tempstring,O_RDONLY);
  n = read(fd,tempstring,256);
  tempstring[n] = '\0';
  lastpart = rindex(tempstring,'/');
  if (lastpart == NULL)
    lastpart = tempstring;
  else
    lastpart++; // don't include last '/'
  strcpy(name,lastpart);
  close(fd);
}
#endif

void StackDump(void)
{
  int pid, core_pid;
  char exec_name[128];
  char core_name[128];
  char tmp_name[128];
  sprintf(tmp_name, "%s.%d", TEMPORARY_FILENAME, getpid());
#ifdef LINUX
  /* get name now -- will be same for children */
  getnamefrompid(getpid(), exec_name);
#else
  strcpy(exec_name, EXECUTABLE_NAME);
#endif
  /* Fork a child to dump core */
  pid = fork();
  if (pid == 0) { /* child */
    abort(); /* dump core */
    exit(0);
  }
  else if (pid == -1) {
    fprintf(stderr, "StackDump ERROR: could not fork process 1 to dump core\n");    exit(1);
  }
  /* Parent continues */
  while (wait(NULL) != pid) {} /* wait for core to be dumped */
  /* Fork a 2nd child! (Runs gdb or dbx) */
  
  core_pid = pid;

  pid = fork();
  if (pid == 0) { /* child */
    FILE *fp;
    int fd;

    /* Open a temporary file for the input: the "where" command */
    fp = fopen(tmp_name, "w");
    fprintf(fp, DEBUGGER_COMMAND);
    fclose(fp);

    fd = open(tmp_name, O_RDONLY, 0);
    if (fd < 0) {
      fprintf(stderr, "StackDump ERROR: open failed on temporary file\n");
      exit(1);
    }
    /* Redirect stdin from the temporary file */
    close(0); /* close stdin */
    dup(fd); /* replace file descriptor 0 with reference to temp file */
    close(fd); /* close the other reference to temporary file */

    /* Find the core file */
    strcpy(core_name, CORE_NAME);
    fd = open(core_name, O_RDONLY, 0);
    if (fd < 0) {
      sprintf(core_name, "%s.%d", CORE_NAME, core_pid);
      fprintf(stderr, "core not found, trying w/ %d == %s\n", core_pid, core_name);
      fd = open(core_name, O_RDONLY, 0);
      if (fd < 0) {
	fprintf(stderr, "StackDump ERROR: no core file found!\n");
	exit(1);
      }
    }
    close(fd);

    fprintf(stderr, "-------------------------------------------\n");
    fprintf(stderr, "StackDump: --- now running the debugger ---\n");
    fprintf(stderr, "%s %s %s %s\n",
	    DEBUGGER, QUIET_MODE, exec_name, core_name);
    fprintf(stderr, "-------------------------------------------\n");
    execlp(DEBUGGER, DEBUGGER, QUIET_MODE, exec_name, core_name, NULL);
    /* dbx a.out core or gdb a.out core */
    fprintf(stderr, "StackDump ERROR: execlp failed for debugger\n");
    exit(1);
  }
  else if (pid == -1) {
    fprintf(stderr, "StackDump ERROR: could not fork process 2 to run debugger\n");
    exit(1);
  }
  /* Parent continues */
  /* while(wait(NULL)>0) waits for all children, and could hang, so: */
  while(wait(NULL) != pid) {}

  /* Wait for these children to complete before returning */

  while (wait(NULL) > 0)
    { } /* empty loop */

  remove(tmp_name); /* clean up the temporary file */
  fprintf(stderr, "-------------------------------------------\n");
}
