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

typedef struct plsrv {
	string_t path;
	string_t* args;
	bool respawn;
	bool background;
} plsrv_t;

void signalHandler(int signal);
void setSignal(int signal, struct sigaction* newHandler);
pid_t getActivePid();
int spawnExec(string_t path, string_t* args);
plsrv_t* generateServiceStruct(string_t pathname, plmt_t* mt);
void plSrvErrorNoRet(char* string);
void plSrvInfraTest(int mode, char* string);
int plSrvSystemctl(int action, char* value, plmt_t* mt);
