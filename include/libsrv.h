/********************************************\
 pl-srv, v0.05
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv.h: pl-srv as a library, source header
\********************************************/
#define _XOPEN_SOURCE 700
#pragma once
#include <plrt.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
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
void setSignal(int signal);
int spawnExec(plptr_t args);
pid_t plSrvGetActivePid(void);
int plSrvExecuteSupervisor(plsrv_t service);

long plSrvStrtonum(char* buffer);
int plSrvCheckExist(char* path);
void plSrvInfraTest(void);
plfile_t* plSrvSafeOpen(plsrvactions_t mode, char* filename, plmt_t* mt);
void plSrvRemoveLock(char* service);
plsrv_t plSrvGenerateServiceStruct(plfile_t* srvFile, plmt_t* mt);

int plSrvStartStop(plsrvactions_t action, char* service, plmt_t* mt);
void plSrvInitHalt(plsrvactions_t action, plmt_t* mt);
