/******************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv.h: pl-srv as a library, Header file
\******************************************/
#define _XOPEN_SOURCE 700
#include <pl32.h>
#include <plml.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PLSRV_INIT 1
#define PLSRV_HALT 2
#define PLSRV_START 3
#define PLSRV_STOP 4
#define PLSRV_START_LOCK 5

typedef struct plsrv {
	string_t path;
	string_t* args;
	bool respawn;
	bool background;
} plsrv_t;

void signalHandler(int signal);
void setSignal(int signal);
int spawnExec(string_t path, string_t* args);
pid_t plSrvGetActivePid();
int plSrvExecuteSupervisor(plsrv_t* service);

void plSrvErrorNoRet(char* string, bool usePerror, bool developerBug);
void plStat(char* path, struct stat* statStruct);
long plSafeStrtonum(char* buffer);
void plSrvInfraTest();
plfile_t* plSrvSafeOpen(int mode, char* string, plmt_t* mt);
void plSrvRemoveLock(char* buffer);

plsrv_t* plSrvGenerateServiceStruct(plfile_t* srvFile, plmt_t* mt);
int plSrvStartStop(int action, char* service, plmt_t* mt);
void plSrvInitHalt(int action, plmt_t* mt);
