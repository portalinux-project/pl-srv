/*****************************************\
 pl-srv, v0.08.1
 (c) 2023 pocketlinux32, Under MPL 2.0
 pl-srv.c: Starts and supervises processes
\*****************************************/
#include <libsrv.h>

void signalHandler(int signal){
	pid_t activePid = plSrvGetActivePid();
	int status = 0;
	plfile_t* logFileHandle = plSrvGetLogFile();

	if(activePid > 1){
		switch(signal){
			case SIGTERM:
			case SIGINT:
				plRTLog(logFileHandle, LOG_INFO, plRTStrFromCStr("Halting supervising process", NULL));
				kill(activePid, SIGTERM);
				waitpid(activePid, &status, 0);
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
			puts("PortaLinux Service Supervisor v0.08");
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
				plSrvInit(mt);
			}else if(strcmp("halt", argv[1]) == 0){
				plSrvHalt(mt);
			}else if(strcmp("soft-reboot", argv[1]) == 0){
				puts("* Soft rebooting system...");
				plSrvHalt(mt);
				plSrvInit(mt);
			}else if(argc > 2){
				if(strcmp("start", argv[1]) == 0){
					for(int i = 2; i < argc; i++)
						plSrvStart(argv[i], mt);
				}else if(strcmp("stop", argv[1]) == 0){
					for(int i = 2; i < argc; i++)
						plSrvStop(argv[i], mt);
				}else if(strcmp("restart", argv[1]) == 0){
					for(int i = 2; i < argc; i++){
						plSrvStop(argv[i], mt);
						plSrvStart(argv[i], mt);
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
