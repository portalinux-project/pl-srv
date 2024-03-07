/************************************************************\
 pl-srv, v1.00
 (c) 2024 CinnamonWolfy, Under MPL 2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>
char currentPath[4096];
char realFilename[strlen(service) + 5];
plfile_t* srvFile = NULL;

void preStartStop(char* srvName){
	getcwd(currentPath);

	strcpy(realFilename, service);
	if(strstr(service, ".srv") == NULL)
		strcat(realFilename, ".srv");

	srvFile = plSrvSafeOpen(PLSRV_START, realFilename, mt);
}

int plSrvStart(char* service, plmt_t* mt){
	preStartStop();

	plfile_t* lockFile;
	plsrv_t srvStruct = plSrvGenerateServiceStruct(srvFile, mt);

	if(srvStruct.deps.size > 0){
		for(int i = 0; i < srvStruct.deps.size; i++)
			plSrvStart(((plstring_t*)srvStruct.deps.pointer)[i].data.pointer, mt);
	}

	chdir("/var/pl-srv/srv");
	if(plSrvCheckExist(realFilename) != -1){
		printf("* Service %s has already been started, skipping...\n", realFilename);
		chdir(curpath);
		return 1;
	}
	chdir(curpath);

	printf("* Starting service %s...\n", realFilename);
	fflush(stdout);

	if(srvStruct.respawn)
		lockFile = plSrvSafeOpen(PLSRV_START_LOCK, realFilename, mt);

	int servicePid = plSrvExecuteSupervisor(srvStruct, mt);
	if(servicePid > 0 && srvStruct.respawn){
		char lockBuffer[srvStruct.deps.size * 4096];
		snprintf(lockBuffer, 4096, "pid = %d\n", servicePid);
		plFPuts(plRTStrFromCStr(lockBuffer, NULL), lockFile);
		if(srvStruct.deps.size > 0){
			snprintf(lockBuffer, 4096, "deps = [ ");
			for(int i = 0; i < srvStruct.deps.size - 1; i++){
				strcat(lockBuffer, ((plstring_t*)srvStruct.deps.pointer)[i].data.pointer);
				strcat(lockBuffer, ", ");
			}
			strcat(lockBuffer, ((plstring_t*)srvStruct.deps.pointer)[srvStruct.deps.size - 1].data.pointer);
			strcat(lockBuffer, " ]\n");
			plFPuts(plRTStrFromCStr(lockBuffer, NULL), lockFile);
		}
		plFClose(lockFile);
	}else if(servicePid == -1){
		printf("* Error: Failed to start service %s", realFilename);
		return 2;
	}

	return 0;
}

int plSrvStop(char* service, plmt_t* mt){
	preStartStop();
	plfile_t* lockFile;
}
