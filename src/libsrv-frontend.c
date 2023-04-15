/************************************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>

int plSrvStartStop(int action, char* service, plmt_t* mt){
	switch(action){
		case PLSRV_START: ;
			printf("* Starting service %s...\n", value);

			plsrv_t* srvStruct = generateServiceStruct(fullPath, mt);
			int servicePid = executeSupervisor(srvStruct);

			if(servicePid > 0){
				strncpy(fullPath, "/var", 4);
				plfile_t* lockFile = plSrvSafeOpen(fullPath, "w", mt);
				char numberBuffer[16];
				snprintf(numberBuffer, 16, "%d", servicePid);
				plFPuts(numberBuffer, lockFile);
				plFClose(lockFile);
				return 0;
			}else if(servicePid == -1){
				printf("* Error: Failed to start service %s", value);
				return 2;
			}
			break;
		case PLSRV_STOP: ;
			printf("* Stopping service %s...\n", value);

			plfile_t* lockFile = plFOpen(fullPath, "r", mt);
			char numBuffer[16] = "";
			char* pointerHolder;
			pid_t pidNum;
			plFGets(numBuffer, 16, lockFile);
			pidNum = strtol(numBuffer, &pointerHolder, 10);
			kill(pidNum, SIGTERM);
			plFClose(lockFile);
			remove(fullPath);
			break;
	}
}

void plSrvInitHalt(int action, plmt_t* mt){
	DIR* directory;
	struct dirent directoryEntry;
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
