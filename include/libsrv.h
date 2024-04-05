/********************************************\
 pl-srv, v1.00
 (c) 2024 CinnamonWolfy, Under MPL 2.0
 libsrv.h: pl-srv as a library, source header
\********************************************/
#define _XOPEN_SOURCE 700
#pragma once
#include <plrt.h>
#include <libgen.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef enum plsrvactions {
	PLSRV_INIT,
	PLSRV_HALT,
	PLSRV_START,
	PLSRV_STOP,
	PLSRV_START_LOCK
} plsrvactions_t;

typedef struct plsrv {
	plptr_t args;
	plptr_t deps;
	bool respawn;
	bool background;
} plsrv_t;

void signalHandler(int signal);
pid_t plSrvGetActivePid(void);
plfile_t* plSrvGetLogFile(void);
int plSrvExecuteSupervisor(plsrv_t service, plmt_t* mt);

long plSrvStrtonum(char* buffer);
bool plSrvCheckExist(char* path);
void plSrvInfraTest(void);
plfile_t* plSrvSafeOpen(plsrvactions_t mode, char* filename, plmt_t* mt);
void plSrvRemoveLock(char* service);
plsrv_t plSrvGenerateServiceStruct(plfile_t* srvFile, plmt_t* mt);

int plSrvStart(char* service, plmt_t* mt);
int plSrvStop(char* service, plmt_t* mt);
void plSrvInit(plmt_t* mt);
void plSrvHalt(plmt_t* mt);
