/******************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv.h: pl-srv as a library, Header file
\******************************************/
#define _XOPEN_SOURCE 700
#pragma once
#include <plrt.h>
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
	PLSRV_START_LOCK,
} plsrvactions_t;

typedef struct plsrv {
	string_t path;
	string_t* args;
	bool respawn;
	bool background;
} plsrv_t;

void signalHandler(int signal);
void setSignal(int signal);
int spawnExec(string_t path, string_t* args);
pid_t plSrvGetActivePid(void);
int plSrvExecuteSupervisor(plsrv_t* service);

void plSrvPanic(char* string, bool usePerror, bool developerBug);
void plStat(char* path, struct stat* statStruct);
long plSafeStrtonum(char* buffer);
void plSrvInfraTest(void);
plfile_t* plSrvSafeOpen(int mode, char* string, plmt_t* mt);
void plSrvRemoveLock(char* buffer);

plsrv_t* plSrvGenerateServiceStruct(plfile_t* srvFile, plmt_t* mt);
int plSrvStartStop(plsrvactions_t action, char* service, plmt_t* mt);
void plSrvInitHalt(plsrvactions_t action, plmt_t* mt);
