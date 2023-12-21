/****************************************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv-supervisor.c: pl-srv as a library, supervisor source file
\****************************************************************/
#include <libsrv.h>

pid_t activePid = 0;
plfile_t* logFile = NULL;

pid_t plSrvGetActivePid(void){
	return activePid;
}

plfile_t* plSrvGetLogFile(void){
	return logFile;
}

int plSrvExecuteSupervisor(plsrv_t service, plmt_t* mt){
	pid_t exec = 0;
	bool isForked = (service.respawn || service.background);
	if(isForked)
		exec = fork();

	if(exec == 0){
		char stringBuffer[4096];
		logFile = plRTLogStart("pl-srv", mt);
		snprintf(stringBuffer, 4096, "Started supervisor process (process ID %d)", getpid());
		plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));

		if(isForked){
			plRTSetSignal(SIGTERM);
			plRTSetSignal(SIGINT);
		}

		if(service.background){
			freopen("/dev/null", "r", stdin);
			freopen("/dev/null", "w", stdout);
		}

		int status;
		activePid = plRTSpawn(service.args);
		snprintf(stringBuffer, 4096, "Started and supervising service %s with process ID %d", basename(((char**)service.args.pointer)[0]), activePid);
		plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));
		waitpid(activePid, &status, 0);
		snprintf(stringBuffer, 4096, "Process %d exited with status code %d", activePid, status);
		plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));
		if(service.respawn == true){
			while(1){
				snprintf(stringBuffer, 4096, "Restarted and supervising service %s with process ID %d", basename(((char**)service.args.pointer)[0]), activePid);
				plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));
				activePid = plRTSpawn(service.args);
				waitpid(activePid, &status, 0);
				snprintf(stringBuffer, 4096, "Process %d exited with status code %d", activePid, status);
				plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));
			}
		}
		plRTLogStop(logFile);
	}

	return exec;
}
