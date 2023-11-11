/****************************************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv-supervisor.c: pl-srv as a library, supervisor source file
\****************************************************************/
#include <libsrv.h>

pid_t activePid = 0;

void setSignal(int signal){
	struct sigaction oldHandler;
	struct sigaction newHandler;

	newHandler.sa_handler = signalHandler;
	sigemptyset(&newHandler.sa_mask);
	newHandler.sa_flags = 0;

	sigaction(signal, NULL, &oldHandler);
	if(oldHandler.sa_handler != SIG_IGN)
		sigaction(signal, &newHandler, NULL);
}

int spawnExec(plptr_t args){
	pid_t exec = fork();
	if(exec == 0){
		sleep(1);
		char* rawArgs[args.size + 1];
		for(int i = 0; i < args.size; i++)
			rawArgs[i] = ((plstring_t*)args.pointer)[i].data.pointer;
		rawArgs[args.size] = NULL;

		char buffer[256];
		execv(realpath(rawArgs[0], buffer), rawArgs);

		plRTPanic("spawnExec", PLRT_ERROR | PLRT_ERRNO | errno, false);
	}
	return exec;
}

pid_t plSrvGetActivePid(void){
	return activePid;
}

int plSrvExecuteSupervisor(plsrv_t service){
	pid_t exec = 0;
	bool isForked = (service.respawn || service.background);
	if(isForked)
		exec = fork();

	if(exec == 0){
		if(isForked){
			setSignal(SIGTERM);
			setSignal(SIGINT);
		}

		if(service.background){
			freopen("/dev/null", "w", stdin);
			freopen("/dev/null", "w", stdout);
		}

		int status;
		activePid = spawnExec(service.args);
		waitpid(activePid, &status, 0);
		if(service.respawn == true){
			while(1){
				activePid = spawnExec(service.args);
				waitpid(activePid, &status, 0);
			}
		}
	}

	return exec;
}
