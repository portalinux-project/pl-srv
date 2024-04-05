/************************************************************\
 pl-srv, v1.00
 (c) 2024 CinnamonWolfy, Under MPL 2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>
char currentPath[4096];
char realFilename[256];
plfile_t* srvFile = NULL;

void preStartStop(char* srvName, plsrvactions_t action, plmt_t* mt){
	getcwd(currentPath, 4096);

	strcpy(realFilename, srvName);
	if(strstr(srvName, ".srv") == NULL)
		strcat(realFilename, ".srv");

	srvFile = plSrvSafeOpen(action, realFilename, mt);
}

int getStopDeps(plstring_t* buffer, plmltoken_t* tokenBuffer, char* filename, plmt_t* mt){
	plfile_t* tempFile = plSrvSafeOpen(PLSRV_STOP, filename, mt);
	plFGets(buffer, tempFile);
	if(plFGets(buffer, tempFile) != 1){
		plFClose(tempFile);
		plmltoken_t bufferToken = plMLParse(*buffer, mt);
		memcpy(tokenBuffer, &bufferToken, sizeof(plmltoken_t));
		if(tokenBuffer->type != PLML_TYPE_STRING || !tokenBuffer->isArray)
			return 1;

		return 0;
	}

	return 1;
}

int isStringInPLMLStrArray(plptr_t array, char* string){
	plptr_t* stringList = array.pointer;
	int i = 0;
	while(i < array.size && strcmp(stringList[i].pointer, string) != 0)
		i++;

	if(i < array.size)
		return i;

	return 0;
}

int plSrvStart(char* service, plmt_t* mt){
	preStartStop(service, PLSRV_START, mt);

	plfile_t* lockFile;
	plsrv_t srvStruct = plSrvGenerateServiceStruct(srvFile, mt);

	if(srvStruct.deps.size > 0){
		for(int i = 0; i < srvStruct.deps.size; i++)
			plSrvStart(((plstring_t*)srvStruct.deps.pointer)[i].data.pointer, mt);
	}

	if(plSrvCheckExist(realFilename) != -1){
		printf("* Service %s has already been started, skipping...\n", realFilename);
		chdir(currentPath);
		return 1;
	}

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
	preStartStop(service, PLSRV_STOP, mt);

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
	plmltoken_t bufferToken = plMLParse(buffer, mt);
	if(bufferToken.type != PLML_TYPE_INT)
		plRTPanic("plSrvStartStop", PLRT_ERROR | PLRT_INVALID_TOKEN, false);
	pidNum = bufferToken.value.integer;

	plptr_t tempDirents = plRTGetDirents("/var/pl-srv/srv", mt);
	plstring_t* tempDirentsArrPtr = tempDirents.pointer;
	int offsetHolder = 0;
	buffer.data.size = 65536;
	for(int i = 0; i < tempDirents.size; i++){
		if(getStopDeps(&buffer, &bufferToken, tempDirentsArrPtr[i].data.pointer, mt) == 0 && (offsetHolder = isStringInPLMLStrArray(bufferToken.value.array, service))){
			printf("Error: Service %s is depended on by service %s.\n", service, ((plptr_t*)bufferToken.value.array.pointer)[offsetHolder].pointer);
			return 3;
		}
	}

	chdir(currentPath);
	kill(pidNum, SIGTERM);
	plFClose(lockFile);
	plSrvRemoveLock(realFilename);
}

void plSrvDetermineHaltOrder(plptr_t direntArray, plmt_t* mt){
	plstring_t* rawDirentArr = direntArray.pointer;
	plstring_t workingDirentArr[direntArray.size];
	char rawBuffer[65536];
	plstring_t buffer = {
		.data = {
			.pointer = rawBuffer,
			.size = 65536
		},
		.isplChar = false,
		.mt = NULL
	};
	size_t writeMarker = 0;
	plmltoken_t depsToken;

	for(int i = 0; i < direntArray.size; i++){
		if(getStopDeps(&buffer, &depsToken, rawDirentArr[i].data.pointer, mt) == 1){
			workingDirentArr[writeMarker] = rawDirentArr[i];
			writeMarker++;

			rawDirentArr[i].data.pointer = NULL;
			rawDirentArr[i].data.size = 0;
		}
	}

	while(writeMarker < direntArray.size){
		for(int i = 0; i < direntArray.size; i++){
			int j = 0;
			size_t depCounter = 0;
			while(j < writeMarker && rawDirentArr[i].data.pointer != NULL){
				if(getStopDeps(&buffer, &depsToken, rawDirentArr[i].data.pointer, mt) == 0 && isStringInPLMLStrArray(depsToken.value.array, workingDirentArr[j].data.pointer))
					depCounter++;
				j++;
			}

			if(depCounter == depsToken.value.array.size){
				workingDirentArr[writeMarker] = rawDirentArr[i];
				writeMarker++;

				rawDirentArr[i].data.pointer = NULL;
				rawDirentArr[i].data.size = 0;
			}
		}
	}
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

void plSrvHalt(plmt_t* mt){
	plptr_t dirents = plRTGetDirents("/var/pl-srv/srv", mt);
	plstring_t* direntsArr = dirents.pointer;
	struct timespec sleepconst = {
		.tv_sec = 0,
		.tv_nsec = 10000000
	};
	puts("* Stopping all sevices...");

	plSrvDetermineHaltOrder(dirents, mt);
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
