/******************************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 pl-init.c: Initializes the system enough to run pl-srv
\******************************************************/
#include <libsrv.h>

// Linux-specific headers
#include <sys/mount.h>
#include <sys/reboot.h>

bool inChroot = false;

void signalHandler(int signal){
	string_t plSrvArgs[3] = { "pl-srv", "halt", NULL };
	pid_t plSrvPid = spawnExec("/usr/bin/pl-srv", plSrvArgs);
	int status = 0;
	waitpid(plSrvPid, &status, 0);

	fputs("* Force-killing all processes...", stdout);
	kill(-1, SIGKILL);
	puts("Done.");

	fputs("* Syncing cached file ops...", stdout);
	sync();
	puts("Done.");

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

int safeMountBoot(string_t dest, string_t fstype){
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

int main(int argc, string_t argv[]){
	pid_t pid = getpid();
	uid_t uid = getuid();
	puts("PortaLinux Init v0.02");
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
		string_t args[2] = { "sh", NULL };
		execv("/bin/sh", args);
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
		string_t plSrvArgs[5] = { "pl-srv", "init", NULL };
		spawnExec("/usr/bin/pl-srv", plSrvArgs);

		while(true);
	}
}
