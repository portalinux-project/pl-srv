/******************************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 pl-init.c: Initializes the system enough to run pl-srv
\******************************************************/
#include <libsrv.h>

// Linux-specific headers
#include <sys/mount.h>
#include <sys/reboot.h>

bool inChroot = false;
plmt_t* mt = NULL;

void signalHandler(int signal){
	plSrvInitHalt(PLSRV_HALT, mt);

	switch(signal){
		case SIGUSR2:
			puts("* Powering off...");
			reboot(RB_POWER_OFF);
			break;
		case SIGUSR1:
			puts("* Halting system...");
			reboot(RB_HALT_SYSTEM);
			break;
		case SIGTERM:
			puts("* Rebooting...");
			reboot(RB_AUTOBOOT);
			break;
	}
}

int safeMountBoot(char* dest, char* fstype){
	struct stat root;
	struct stat mountpoint;

	stat("/", &root);
	stat(dest, &mountpoint);

	printf("	%s:", dest);
	if(mountpoint.st_dev == root.st_dev){
		if(mount("none", dest, fstype, 0, "") != 0){
			puts("Error.");
			perror("		pl-init");
			return 1;
		}else{
			puts("Successfully mounted.");
		}
	}else{
		puts("Already mounted.");
	}
	return 0;
}

int main(int argc, char* argv[]){
	pid_t pid = getpid();
	uid_t uid = getuid();
	mt = plMTInit(4 * 1024 * 1024);
	puts("PortaLinux Init v0.04");
	puts("(c) 2023 pocketlinux32, Under MPLv2.0\n");

	// Argument parsing
	if(argc > 1){
		for(int i = 1; i < argc; i++){
			if(strcmp(argv[i], "--help") == 0){
				printf("Usage: %s [options]\n\n", argv[0]);
				puts("Initializes a PortaLinux System enough to run the pl-srv process supervisor.");
				puts("When ran in normal mode, it must be ran as PID 1 and by root");
				puts("--help		Shows this help");
				puts("--chroot	Run in chroot mode");
				return 0;
			}else if(strcmp(argv[i], "--chroot") == 0){
				puts("* Running in chroot mode!");
				inChroot = true;
			}
		}
	}

	if(uid != 0){
		puts("Error: Only root can run init");
		return 1;
	}

	// Simple Initialization
	if(inChroot){
		puts("Bypassing initialization and dropping you to a shell...");
		plstring_t shellArgs = plRTStrFromCStr("sh", NULL);
		plptr_t args = {
			.pointer = &shellArgs,
			.size = 1
		};
		pid_t shellID = spawnExec(args);
		int status;
		waitpid(shellID, &status, 0);
	}else{
		if(pid != 1){
			puts("Error: Init must be ran as PID 1");
			return 2;
		}

		puts("* Mounting necessary filesystems:");
		safeMountBoot("/sys", "sysfs");
		safeMountBoot("/proc", "proc");
		safeMountBoot("/dev", "devtmpfs");

		fputs("* Enabling signal handler: ", stdout);

		setSignal(SIGPWR);
		setSignal(SIGTERM);
		setSignal(SIGINT);
		setSignal(SIGUSR1);
		setSignal(SIGUSR2);
		puts("Done.");

		puts("* Running pl-srv...\n");
		pid_t exec = fork();
		if(exec == 0)
			plSrvInitHalt(PLSRV_INIT, mt);
		else
			while(true);
	}
}
