/************************************************************\
 pl-srv, v0.05
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>

int plSrvStartStop(plsrvactions_t action, char* service, plmt_t* mt){
	char realFilename[strlen(service) + 5];
	realFilename[0] = '\0';
	strcpy(realFilename, service);
	if(strstr(service, ".srv") == NULL)
		strcat(realFilename, ".srv");

	plfile_t* srvFile = plSrvSafeOpen(PLSRV_START, realFilename, mt);
	plfile_t* lockFile;

	switch(action){
		case PLSRV_START: ;
			plsrv_t srvStruct = plSrvGenerateServiceStruct(srvFile, mt);
			if(srvStruct.deps.size > 0){
				for(int i = 0; i < srvStruct.deps.size; i++)
					plSrvStartStop(PLSRV_START, ((plstring_t*)srvStruct.deps.pointer)[i].data.pointer, mt);
			}
			if(srvStruct.respawn)
				lockFile = plSrvSafeOpen(PLSRV_START_LOCK, realFilename, mt);

			printf("* Starting service %s...\n", realFilename);
			fflush(stdout);

			int servicePid = plSrvExecuteSupervisor(srvStruct);
			if(servicePid > 0){
				char pidBuffer[32];
				snprintf(pidBuffer, 32, "pid = %d\n", servicePid);
				plFPuts(plRTStrFromCStr(pidBuffer, NULL), lockFile);
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
			char numBuffer[16] = "";
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
			plmltoken_t plmlToken = plMLParse(buffer, mt);
			if(plmlToken.type != PLML_TYPE_INT)
				plRTPanic("plSrvStartStop", PLRT_ERROR | PLRT_INVALID_TOKEN, false);

			pidNum = plmlToken.value.integer;
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
	for(int i = 0; i < dirents.size; i++){
		plSrvStartStop(mode, direntsArr[i].data.pointer, mt);
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
