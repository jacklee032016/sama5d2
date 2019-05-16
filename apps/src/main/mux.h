/*
* header only for app
*/

#ifndef	__MUX_H__
#define	__MUX_H__

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/resource.h>
#include <signal.h>

#include <stdarg.h>
#include <sys/poll.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdint.h>

#include <dlfcn.h>

#include "libCmn.h"
#include "libMux.h"

#include "libCmnSys.h"

#include "mux7xx.h"


/* declarations for program */
int32_t	extSystemInit(MuxMain		*muxMain);


#endif

