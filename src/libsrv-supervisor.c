/***************`*************************************************\
 pl-srv, v0.07
 (c) 2024 CinnamonWolfy, Under MPL 2.0
 libsrv-supervisor.c: pl-srv as a library, supervisor source file
\****************************************************************/
#include <libsrv.h>
#include <time.h>

pid_t activePid = 0;
plfile_t* logFile = NULL;

pid_t plSrvGetActivePid(void){
	return activePid;
}

plfile_t* plSrvGetLogFile(void){
	return logFile;
}

int plSrvBlockingSpawn(plptr_t args){
	int status;
	activePid = plRTSpawn(args);
	waitpid(activePid, &status, 0);

	return status;
}

int plSrvExecuteSupervisor(plsrv_t service, plmt_t* mt){
	pid_t exec = 0;
	bool isForked = (service.respawn || service.background);
	if(isForked)
		exec = fork();

	if(exec == 0){
		char stringBuffer[4096];
		int procStatus;
		struct timespec sleep = {
			.tv_sec = 0,
			.tv_nsec = 1000
		};
		nanosleep(&sleep, NULL);

		snprintf(stringBuffer, 4096, "pl-srv-%d", getpid());
		logFile = plRTLogStart(stringBuffer, mt);

		snprintf(stringBuffer, 4096, "Started supervisor process (process ID %d)", getpid());
		plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));

		if(isForked){
			plRTSetSignal(SIGTERM);
			plRTSetSignal(SIGINT);
		}

		if(service.background){
			if(service.logging){
				snprintf(stringBuffer, 4096, "/var/log/%s", basename(((char**)service.args.pointer)[0]));
				mkdir(stringBuffer, 0644);

				struct timespec timestamp;
				clock_gettime(CLOCK_REALTIME, &timestamp);
				snprintf(stringBuffer, 4096, "/var/log/%s/%d-%d.log", basename(((char**)service.args.pointer)[0]), timestamp.tv_sec, timestamp.tv_nsec);
				freopen(stringBuffer, "r", stdin);
				freopen(stringBuffer, "r", stdout);
			}else{
				freopen("/dev/null", "r", stdin);
				freopen("/dev/null", "w", stdout);
			}
		}

		snprintf(stringBuffer, 4096, "Started and supervising service %s", basename(((char**)service.args.pointer)[0]));
		plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));

		procStatus = plSrvBlockingSpawn(service.args);

		snprintf(stringBuffer, 4096, "Process %d exited with status code %d", activePid, WEXITSTATUS(procStatus));
		plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));
		if(service.respawn == true){
			while(1){
				snprintf(stringBuffer, 4096, "Restarted and supervising service %s", basename(((char**)service.args.pointer)[0]));
				plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));

				procStatus = plSrvBlockingSpawn(service.args);

				snprintf(stringBuffer, 4096, "Process %d exited with status code %d", activePid, WEXITSTATUS(procStatus));
				plRTLog(logFile, LOG_INFO, plRTStrFromCStr(stringBuffer, NULL));
			}
		}
		plRTLogStop(logFile);
		exit(0);
	}

	return exec;
}
