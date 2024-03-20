 /************************************************************\
 pl-srv, v1.00
 (c) 2024 CinnamonWolfy, Under MPL 2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>
char currentPath[4096];
char realFilename[strlen(service) + 5];
plfile_t* srvFile = NULL;

void preStartStop(char* srvName, plsrvactions_t action){
	getcwd(currentPath);

	strcpy(realFilename, service);
	if(strstr(service, ".srv") == NULL)
		strcat(realFilename, ".srv");

	srvFile = plSrvSafeOpen(action, realFilename, mt);
}

int plSrvStart(char* service, plmt_t* mt){
	preStartStop(service, PLSRV_START);

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
		plmltoken_t token = {
			.name = plRTStrFromCStr("pid", NULL),
			.type = PLML_TYPE_INT,
			.value.integer = servicePid,
			.isArray = false,
			.mt = NULL
		};
		plstring_t generatedToken = plMLGenerateTokenStr(token, mt);
		plFPuts(generatedToken, lockFile);
		plMTFree(mt, generatedToken.data.pointer);

		if(srvStruct.deps.size > 0){
			token.name = plRTStrFromCStr("deps", NULL);
			token.type = PLML_TYPE_STRING;
			token.value.array = srvStruct.deps;
			token.isArray = true;
			generatedToken = plMLGenerateTokenStr(token, mt);
			plFPuts(generatedToken, lockFile);
			plMTFree(mt, generatedToken.data.pointer);
		}

		plFClose(lockFile);
	}else if(servicePid == -1){
		printf("* Error: Failed to start service %s", realFilename);
		return 2;
	}

	return 0;
}

int plSrvStop(char* service, plmt_t* mt){
	preStartStop(service, PLSRV_STOP);

	printf("* Stopping service %s...\n", realFilename);
	fflush(stdout);

	plfile_t* lockFile = plSrvSafeOpen(PLSRV_STOP, realFilename, mt);
	char numBuffer[65536] = "";
	plstring_t buffer = {
		.data = {
			.pointer = numBuffer,
			.size = 16
		},
		.isplChar = false,
		.mt = NULL
	};
	pid_t pidNum = 0;

	plFGets(&buffer, lockFile);
	plmltoken_t pidToken = plMLParse(buffer, mt);
	if(pidToken.type != PLML_TYPE_INT)
		plRTPanic("plSrvStartStop", PLRT_ERROR | PLRT_INVALID_TOKEN, false);

	plptr_t tempDirents = plRTGetDirents("/var/pl-srv/srv", mt);
	plstring_t* tempDirentsArrPtr = tempDirents.pointer;
	buffer.data.size = 65536;
	chdir("/var/pl-srv/srv");
	for(int i = 0; i < tempDirents.size; i++){
		plfile_t* tempFile = plFOpen(tempDirentsArrPtr[i].data.pointer, "r", mt);
		plFGets(&buffer, lockFile);
		if(plFGets(&buffer, lockFile) != 1){
			plmltoken_t depsToken = plMLParse(buffer, mt);
			plptr_t* depsList = depsToken.value.array.pointer;
			int j = 0;
			while(j < depsToken.value.array.size && strcmp(depsList[j].pointer, service) != 0)
				j++;

			if(j < depsToken.value.array.size && strcmp(depsList[j].pointer, service) != 0){
				printf("Error: Service %s is depended on by service %s.\n", service, depsList[i].pointer);
				plFClose(tempFile);
				return 3;
			}
		}
		plFClose(tempFile);
	}

	chdir(curpath);
	pidNum = pidToken.value.integer;
	kill(pidNum, SIGTERM);
	plFClose(lockFile);
	plSrvRemoveLock(realFilename);
}

void plSrvInit(plmt_t* mt){
	plptr_t dirents = plRTGetDirents("/etc/pl-srv/srv", mt);
	plstring_t* direntsArr = dirents.pointer;
	struct timespec sleepconst = {
		.tv_sec = 0,
		.tv_nsec = 10000000
	};
	puts("* Starting all sevices...");

	for(int i = 0; i < dirents.size; i++){
		plSrvStart(direntsArr[i].data.pointer, mt);
		nanosleep(&sleepconst, NULL);
	}
	plRTFreeParsedString(dirents);
}

void plSrrHalt(plmt_t* mt){
	plptr_t dirents = plRTGetDirents("/var/pl-srv/srv", mt);
	plstring_t* direntsArr = dirents.pointer;
	struct timespec sleepconst = {
		.tv_sec = 0,
		.tv_nsec = 10000000
	};
	puts("* Stopping all sevices...");

	for(int i = 0; i < dirents.size; i++){
		plSrvStop(direntsArr[i].data.pointer, mt);
		nanosleep(&sleepconst, NULL);
	}
	plRTFreeParsedString(dirents);

	fputs("* Force-killing all processes...", stdout);
	kill(-1, SIGKILL);
	puts("Done.");

	fputs("* Syncing cached file ops...", stdout);
	sync();
	puts("Done.");
}
