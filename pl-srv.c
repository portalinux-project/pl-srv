/*****************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 pl-srv.c: Starts and supervises processes
\*****************************************/
#include <libsrv.h>

void signalHandler(int signal){
	pid_t activePid = plSrvGetActivePid();

	if(activePid > 1){
		switch(signal){
			case SIGTERM:
			case SIGINT:
				kill(activePid, SIGTERM);
				break;
		}
	}
	exit(0);
}

int main(int argc, string_t argv[]){
	plmt_t* mt = plMTInit(8 * 1024 * 1024);

	if(argc > 1){
		if(strcmp("help", argv[1]) == 0){
			puts("PortaLinux Service Supervisor v0.02");
			puts("(c) 2023 pocketlinux32, Under MPLv2.0\n");
			printf("Usage: %s {options} [value]\n\n", argv[0]);
			puts("Starts and supervises a service. All service units are stored in /etc/pl-srv");
			puts("help	Shows this help");
			puts("start	Starts a service");
			puts("stop	Stops a service");
			puts("init	Starts all services");
			puts("halt	Stops all services");
			puts("\nFor more information, please go to https://github.com/pocketlinux32/pl-srv");
			return 0;
		}else{
			plSrvInfraTest();
			if(strcmp("init", argv[1]) == 0){
				puts("* Starting all active services...");
				plSrvInitHalt(PLSRV_INIT, mt);
			}else if(strcmp("halt", argv[1]) == 0){
				puts("* Halting all running services...");
				plSrvInitHalt(PLSRV_HALT, mt);
			}else if(argc > 2){
				if(strcmp("start", argv[1]) == 0){
					for(int i = 2; i < argc; i++)
						plSrvStartStop(PLSRV_START, argv[i], mt);
				}else if(strcmp("stop", argv[1]) == 0){
					for(int i = 2; i < argc; i++)
						plSrvStartStop(PLSRV_STOP, argv[i], mt);
				}
			}else{
				puts("Error: Not enough argument");
			}
		}
	}else{
		puts("Error: Not enough arguments");
	}
}
