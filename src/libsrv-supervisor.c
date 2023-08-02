/****************************************************************\
 pl-srv, v0.03
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv-supervisor.c: pl-srv as a library, Supervisor source file
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

int spawnExec(string_t path, string_t* args){
	pid_t exec = fork();
	if(exec == 0){
		sleep(1);
		char buffer[256];
		execv(realpath(path, buffer), args);

		perror("* Error executing program");
		return 255;
	}
	return exec;
}

pid_t plSrvGetActivePid(void){
	return activePid;
}

int plSrvExecuteSupervisor(plsrv_t* service){
	if(service == NULL)
		return -1;

	pid_t exec = 0;
	bool isForked = (service->respawn || service->background);
	if(isForked)
		exec = fork();

	if(exec == 0){
		if(isForked){
			setSignal(SIGTERM);
			setSignal(SIGINT);
		}

		if(service->background){
			freopen("/dev/null", "w", stdin);
			freopen("/dev/null", "w", stdout);
		}

		int status;
		activePid = spawnExec(service->path, service->args);
		waitpid(activePid, &status, 0);
		if(service->respawn == true){
			while(1){
				activePid = spawnExec(service->path, service->args);
				waitpid(activePid, &status, 0);
			}
		}
	}

	return exec;
}
