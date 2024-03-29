/*****************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 pl-srv.c: Starts and supervises processes
\*****************************************/
#include <libsrv.h>

void signalHandler(int signal){
	pid_t activePid = plSrvGetActivePid();
	plfile_t* logFileHandle = plSrvGetLogFile();

	if(activePid > 1){
		switch(signal){
			case SIGTERM:
			case SIGINT:
				plRTLog(logFileHandle, LOG_INFO, plRTStrFromCStr("Halting supervising process", NULL));
				kill(activePid, SIGTERM);
				break;
		}
	}
	plRTLog(logFileHandle, LOG_INFO, plRTStrFromCStr("Halting supervisor", NULL));
	plRTLogStop(logFileHandle);
	exit(0);
}

int main(int argc, char* argv[]){
	plmt_t* mt = plMTInit(8 * 1024 * 1024);

	if(argc > 1){
		if(strcmp("help", argv[1]) == 0){
			puts("PortaLinux Service Supervisor v1.00");
			puts("(c) 2024 CinnamonWolfy, Under MPL 2.0\n");
			printf("Usage: %s {options} [value]\n\n", argv[0]);
			puts("Starts and supervises a service. All service units are stored in /etc/pl-srv");
			puts("help		Shows this help");
			puts("start		Starts a service");
			puts("stop		Stops a service");
			puts("restart		Restarts a service");
			puts("soft-reboot	Soft-reboots the system (goes through a normal shutdown procedure, then through a normal init procedure without shutting off)");
			puts("\nFor more information, please go to https://github.com/pocketlinux32/pl-srv");
			return 0;
		}else{
			plSrvInfraTest();
			if(strcmp("init", argv[1]) == 0){
				plSrvInitHalt(PLSRV_INIT, mt);
			}else if(strcmp("halt", argv[1]) == 0){
				plSrvInitHalt(PLSRV_HALT, mt);
			}else if(strcmp("soft-reboot", argv[1]) == 0){
				puts("* Soft rebooting system...");
				plSrvInitHalt(PLSRV_HALT, mt);
				plSrvInitHalt(PLSRV_INIT, mt);
			}else if(argc > 2){
				if(strcmp("start", argv[1]) == 0){
					for(int i = 2; i < argc; i++)
						plSrvStartStop(PLSRV_START, argv[i], mt);
				}else if(strcmp("stop", argv[1]) == 0){
					for(int i = 2; i < argc; i++)
						plSrvStartStop(PLSRV_STOP, argv[i], mt);
				}else if(strcmp("restart", argv[1]) == 0){
					for(int i = 2; i < argc; i++){
						plSrvStartStop(PLSRV_STOP, argv[i], mt);
						plSrvStartStop(PLSRV_START, argv[i], mt);
					}
				}
			}else{
				puts("Error: Unknown command");
			}
		}
	}else{
		puts("Error: Not enough arguments");
	}
}
