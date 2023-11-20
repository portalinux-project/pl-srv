/************************************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv-frontend.c: pl-srv as a library, Frontend source file
\************************************************************/
#include <libsrv.h>

int plSrvStartStop(plsrvactions_t action, char* service, plmt_t* mt){
	plfile_t* srvFile = plSrvSafeOpen(PLSRV_START, service, mt);
	plfile_t* lockFile;

	switch(action){
		case PLSRV_START:
			printf("* Starting service %s...\n", service);
			fflush(stdout);

			plsrv_t srvStruct = plSrvGenerateServiceStruct(srvFile, mt);
			if(srvStruct.respawn)
				lockFile = plSrvSafeOpen(PLSRV_START_LOCK, service, mt);

			int servicePid = plSrvExecuteSupervisor(srvStruct);
			if(servicePid > 0){
				char numberBuffer[16];
				snprintf(numberBuffer, 16, "%d\n", servicePid);
				plstring_t buffer = plRTStrFromCStr(numberBuffer, NULL);
				plFPuts(&buffer, lockFile);
				plFClose(lockFile);
			}else if(servicePid == -1){
				printf("* Error: Failed to start service %s", service);
				return 2;
			}
			break;
		case PLSRV_STOP:
			printf("* Stopping service %s...\n", service);
			fflush(stdout);

			lockFile = plSrvSafeOpen(PLSRV_STOP, service, mt);
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
			pidNum = plSrvStrtonum(numBuffer);

			kill(pidNum, SIGTERM);
			plFClose(lockFile);
			plSrvRemoveLock(service);
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
	DIR* directory = NULL;
	struct dirent* directoryEntry;
	int mode;

	switch(action){
		case PLSRV_INIT:
			puts("* Starting all active services...");
			directory = opendir("/etc/pl-srv");
			mode = PLSRV_START;
			break;
		case PLSRV_HALT:
			puts("* Halting all running services...");
			directory = opendir("/var/pl-srv");
			mode = PLSRV_STOP;
			break;
		default:
			// Catch all just incase a programing oopsie happens so we get a
			// proper error instead of a long gdb/lldb session.
			plRTPanic("plSrvInitHalt", PLRT_ERROR | PLRT_INVALID_TOKEN, true);
			break; // never reached
	}

	while((directoryEntry = readdir(directory)) != NULL){
		// Check to remove . and .. from directory listing
		if(strcmp(directoryEntry->d_name, ".") != 0 && strcmp(directoryEntry->d_name, "..") != 0)
			plSrvStartStop(mode, directoryEntry->d_name, mt);
	}

	if(action == PLSRV_HALT){
		fputs("* Force-killing all processes...", stdout);
		kill(-1, SIGKILL);
		puts("Done.");

		fputs("* Syncing cached file ops...", stdout);
		sync();
		puts("Done.");
	}
}
