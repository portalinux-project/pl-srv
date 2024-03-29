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

void signalHandler(int signal){
	plstring_t execArgs[2] = { plRTStrFromCStr("/usr/bin/pl-srv", NULL), plRTStrFromCStr("halt", NULL) };
	plptr_t execArr = {
		.pointer = execArgs,
		.size = 2
	};
	int status = 0;
	pid_t execPid = plRTSpawn(execArr);
	waitpid(execPid, &status, 0);

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
	puts("PortaLinux Init v1.00");
	puts("(c) 2024 CinnamonWolfy, Under MPLv2.0\n");

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
		plstring_t shellArgs = plRTStrFromCStr("/bin/sh", NULL);
		plptr_t args = {
			.pointer = &shellArgs,
			.size = 1
		};
		pid_t shellID = plRTSpawn(args);
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

		plRTSetSignal(SIGPWR);
		plRTSetSignal(SIGINT);
		plRTSetSignal(SIGTERM);
		plRTSetSignal(SIGUSR1);
		plRTSetSignal(SIGUSR2);
		puts("Done.");

		plstring_t execArgs[2] = { plRTStrFromCStr("/usr/bin/sh", NULL), plRTStrFromCStr("/etc/pl-srv/basic-startup", NULL) };
		plptr_t execArr = {
			.pointer = execArgs,
			.size = 2
		};

		if(plSrvCheckExist("/etc/pl-srv/basic-startup") != -1){
			fputs("* Running /etc/pl-srv/basic-startup...\n", stdout);
			pid_t exec = plRTSpawn(execArr);
			int status = 0;
			waitpid(exec, &status, 0);
		}

		puts("* Running pl-srv...\n");
		execArgs[0] = plRTStrFromCStr("/usr/bin/pl-srv", NULL);
		execArgs[1] = plRTStrFromCStr("init", NULL);
		plRTSpawn(execArr);

		while(true);
	}
}
