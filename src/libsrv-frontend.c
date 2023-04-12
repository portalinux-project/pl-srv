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
				plfile_t* lockFile = plFOpen(fullPath, "w", mt);
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

			break;
		case PLSRV_STOP: ;
			break;
	}
}

int plSrvInitHalt(int action, plmt_t* mt){
	DIR* directory;
	struct dirent directoryEntry;
}

int plSrvSystemctl(int action, char* value, plmt_t* mt){
	switch(action){
		case PLSRV_START: ;
		case PLSRV_STOP: ;
			printf("* Stopping service %s...\n", value);

			if(stat(fullPath, &checkExistence) == -1){
				perror("plSrvSystemctl");
				return 1;
			}

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
		case PLSRV_INIT: ;
			DIR* directorySrv = opendir("/etc/pl-srv");
			struct dirent* dirEntriesSrv;

			if(directorySrv == NULL){
				puts("Error: Service directory not found");
				return 3;
			}

			readdir(directorySrv);
			readdir(directorySrv);
			while((dirEntriesSrv = readdir(directorySrv)) != NULL){
				plSrvSystemctl(PLSRV_START, strtok(dirEntriesSrv->d_name, "."), mt);
			}
			break;
		case PLSRV_HALT: ;
			DIR* directoryActive = opendir("/var/pl-srv");
			struct dirent* dirEntriesActive;

			if(directorySrv == NULL){
				puts("Error: Service directory not found");
				return 3;
			}

			readdir(directory);
			readdir(directory);
			while((dirEntriesActive = readdir(directoryActive)) != NULL){
				plSrvSystemctl(PLSRV_STOP, strtok(dirEntriesActive->d_name, "."), mt);
			}
			break;
	}
	return 0;
}
