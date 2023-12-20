/****************************************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv-supervisor.c: pl-srv as a library, supervisor source file
\****************************************************************/
#include <libsrv.h>

pid_t activePid = 0;

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
			plRTSetSignal(SIGTERM);
			plRTSetSignal(SIGINT);
		}

		if(service.background){
			freopen("/dev/null", "w", stdin);
			freopen("/dev/null", "w", stdout);
		}

		int status;
		activePid = plRTSpawn(service.args);
		waitpid(activePid, &status, 0);
		if(service.respawn == true){
			while(1){
				activePid = plRTSpawn(service.args);
				waitpid(activePid, &status, 0);
			}
		}
	}

	return exec;
}
