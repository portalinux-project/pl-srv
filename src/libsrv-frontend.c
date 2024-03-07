/************************************************************\
 pl-srv, v1.00
 (c) 2024 CinnamonWolfy, Under MPL 2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>

int plSrvStartStop(plsrvactions_t action, char* service, plmt_t* mt){
	char curpath[4096] = "";
	getcwd(curpath, 4096);

	plfile_t* srvFile = plSrvSafeOpen(PLSRV_START, realFilename, mt);
	plfile_t* lockFile;

	switch(action){
		case PLSRV_START: ;
			plsrv_t srvStruct = plSrvGenerateServiceStruct(srvFile, mt);
			if(srvStruct.deps.size > 0){
				for(int i = 0; i < srvStruct.deps.size; i++)
					plSrvStartStop(PLSRV_START, ((plstring_t*)srvStruct.deps.pointer)[i].data.pointer, mt);
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
			break;
		case PLSRV_STOP: ;
			printf("* Stopping service %s...\n", realFilename);
			fflush(stdout);

			lockFile = plSrvSafeOpen(PLSRV_STOP, realFilename, mt);
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
			break;
		case PLSRV_INIT:
		case PLSRV_HALT:
		case PLSRV_START_LOCK:
		default:
			// Catch all just incase a programing oopsie happens so we get a
			// proper error instead of a long gdb/lldb session.
			plRTPanic("plSrvStartStop", PLRT_ERROR | PLRT_INVALID_TOKEN, true);
			break; // never reached
		}

		return 0;
}

void plSrvInitHalt(plsrvactions_t action, plmt_t* mt){
	plptr_t dirents;
	plsrvactions_t mode;

	switch(action){
		case PLSRV_INIT:
			puts("* Starting all active services...");
			dirents = plRTGetDirents("/etc/pl-srv/srv", mt);
			mode = PLSRV_START;
			break;
		case PLSRV_HALT:
			puts("* Halting all running services...");
			dirents = plRTGetDirents("/var/pl-srv/srv", mt);
			mode = PLSRV_STOP;
			break;
		default:
			// Catch all just incase a programing oopsie happens so we get a
			// proper error instead of a long gdb/lldb session.
			plRTPanic("plSrvInitHalt", PLRT_ERROR | PLRT_INVALID_TOKEN, true);
			break; // never reached
	}

	plstring_t* direntsArr = dirents.pointer;
	struct timespec sleepconst = {
		.tv_sec = 0,
		.tv_nsec = 10000000
	};
	for(int i = 0; i < dirents.size; i++){
		plSrvStartStop(mode, direntsArr[i].data.pointer, mt);
		nanosleep(&sleepconst, NULL);
	}
	plRTFreeParsedString(dirents);

	if(action == PLSRV_HALT){
		fputs("* Force-killing all processes...", stdout);
		kill(-1, SIGKILL);
		puts("Done.");

		fputs("* Syncing cached file ops...", stdout);
		sync();
		puts("Done.");
	}
}
