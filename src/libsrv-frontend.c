/************************************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>

int plSrvStartStop(int action, char* service, plmt_t* mt){
	plfile_t* srvFile = plSrvSafeOpen(PLSRV_START, service, mt);
	plfile_t* lockFile;

	switch(action){
		case PLSRV_START: ;
			printf("* Starting service %s...\n", service);

			lockFile = plSrvSafeOpen(PLSRV_START_LOCK, service, mt);
			plsrv_t* srvStruct = plSrvGenerateServiceStruct(srvFile, mt);
			int servicePid = plSrvExecuteSupervisor(srvStruct);

			if(servicePid > 0){
				char numberBuffer[16];
				snprintf(numberBuffer, 16, "%d", servicePid);
				plFPuts(numberBuffer, lockFile);
				plFClose(lockFile);
			}else if(servicePid == -1){
				printf("* Error: Failed to start service %s", service);
				return 2;
			}
			break;
		case PLSRV_STOP: ;
			printf("* Stopping service %s...\n", service);

			lockFile = plSrvSafeOpen(PLSRV_START_LOCK, service, mt);
			char numBuffer[16] = "";
			char* pointerHolder;
			pid_t pidNum;

			plFGets(numBuffer, 16, lockFile);
			pidNum = plSafeStrtonum(numBuffer);

			kill(pidNum, SIGTERM);
			plFClose(lockFile);
			plSrvRemoveLock(service);
			break;
	}

	return 0;
}

void plSrvInitHalt(int action, plmt_t* mt){
	DIR* directory;
	struct dirent* directoryEntry;
	int mode;

	if(action == PLSRV_INIT){
		directory = opendir("/etc/pl-srv");
		mode = PLSRV_START;
	}else{
		directory = opendir("/var/pl-srv");
		mode = PLSRV_STOP;
	}

	readdir(directory);
	readdir(directory);

	while((directoryEntry = readdir(directory)) != NULL){
		plSrvStartStop(mode, directoryEntry->d_name, mt);
	}
}
