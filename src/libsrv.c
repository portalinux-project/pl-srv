/******************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv.h: pl-srv as a library, Source file
\******************************************/
#include <libsrv.h>

pid_t activePid = 0;

void setSignal(int signal, struct sigaction* newHandler){
	sigemptyset(&newSigAction.sa_mask);
	newSigAction.sa_flags = 0;

	struct sigaction oldHandler;
	sigaction(signal, NULL, &oldHandler);
	if(oldHandler.sa_handler != SIG_IGN)
		sigaction(signal, newHandler, NULL);
}

pid_t getActivePid(){
	return activePid;
}

int spawnExec(string_t path, string_t* args){
	pid_t exec = fork();
	int status;
	if(exec == 0){
		sleep(1);
		char buffer[256];
		execv(realpath(path, buffer), args);

		perror("* Error excecuting program");
		return 255;
	}else{
		activePid = exec;
		waitpid(exec, &status, 0);
	}
	return status;
}

int executeSupervisor(plsrv_t* service, struct sigaction* signalHandler){
	if(service == NULL)
		return -1;

	pid_t exec = 0;
	if(service->respawn == true || service->background == true)
		exec = fork();

	if(exec == 0){
		if(service->background == true){
			freopen("/dev/null", "w", stdin);
			freopen("/dev/null", "w", stdout);
		}

		setSignal(SIGTERM, signalHandler);
		setSignal(SIGINT, signalHandler);

		spawnExec(service->path, service->args);
		if(service->respawn == true){
			while(1)
				spawnExec(service->path, service->args);
		}

		activePid = 0;
	}

	return exec;
}

plsrv_t* generateServiceStruct(string_t pathname, plmt_t* mt){
	if(pathname == NULL || mt == NULL)
		return NULL;

	plfile_t* srvFile = plFOpen(pathname, "r", mt);
	plsrv_t* returnStruct = plMTAllocE(mt, sizeof(plsrv_t));
	returnStruct->respawn = false;
	returnStruct->background = false;

	if(srvFile == NULL)
		return NULL;

	byte_t buffer[256] = "";
	while(plFGets(buffer, 256, srvFile) != NULL){
		plmltoken_t* plmlToken = plMLParse(buffer, mt);
		string_t tokenName;

		plMLGetTokenAttrib(plmlToken, &tokenName, PLML_GET_NAME);

		if(strcmp("exec", tokenName) == 0){
			string_t tokenVal;
			plMLGetTokenAttrib(plmlToken, &tokenVal, PLML_GET_VALUE);

			plarray_t* tokenizedVal = plParser(tokenVal, mt);
			plMTRealloc(mt, tokenizedVal->array, (tokenizedVal->size + 1) * sizeof(string_t*));
			((string_t*)tokenizedVal->array)[tokenizedVal->size] = NULL;

			returnStruct->args = tokenizedVal->array;
			returnStruct->path = ((string_t*)tokenizedVal->array)[0];

			plMTFree(mt, tokenizedVal);
		}else if(strcmp("respawn", tokenName) == 0){
			bool* tokenVal;
			plMLGetTokenAttrib(plmlToken, &tokenVal, PLML_GET_VALUE);
			returnStruct->respawn = *tokenVal;
		}else if(strcmp("background", tokenName) == 0){
			bool* tokenVal;
			plMLGetTokenAttrib(plmlToken, &tokenVal, PLML_GET_VALUE);
			returnStruct->background = *tokenVal;
		}

		plMLFreeToken(plmlToken);
	}

	return returnStruct;
}

void plSrvInfraTest(char* ){
	
}

int plSrvStartStop(int action, char* value, plmt_t* mt){
	struct sigaction sigAction;
	newSigAction.sa_handler = supervisorSignalHandler;

	struct stat checkExistence;
}

int plSrvInitHalt(int action, char* value, plmt_t* mt){
	DIR* directory;
	struct dirent directoryEntry;
}

int plSrvSystemctl(int action, char* value, plmt_t* mt){
	char* fullPath = NULL;
	struct stat checkExistence;

	if(action == PLSRV_START || action == PLSRV_STOP){
		fullPath = plMTAllocE(mt, (18 + strlen(value)) * sizeof(char));
		if(action == PLSRV_START)
			strcpy(fullPath, "/etc");
		else
			strcpy(fullPath, "/var");

		strcat(fullPath, "/pl-srv/");

		strcat(fullPath, value);
		strcat(fullPath, ".srv");
	}

	switch(action){
		case PLSRV_START: ;
			printf("* Starting service %s...\n", value);

			if(stat(fullPath, &checkExistence) == -1){
				perror("plSrvSystemctl");
				return 1;
			}

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
