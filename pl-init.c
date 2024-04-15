/******************************************************\
 pl-srv, v1.00
 (c) 2023 pocketlinux32, Under MPL 2.0
 pl-init.c: Initializes the system enough to run pl-srv
\******************************************************/
#include <libsrv.h>

// Linux-specific headers
#include <sys/mount.h>
#include <sys/reboot.h>

// Console setup header
#include <termios.h>

bool inChroot = false;

void signalHandler(int signal){
	if(signal != SIGUSR2 && signal != SIGUSR1 && signal != SIGTERM)
		return;

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

		fputs("* Setting up console: ", stdout);

		char defaultConsole[10] = "/dev/tty1";
		char* consoleToUse;
		if((consoleToUse = getenv("CONSOLE")) == NULL){
			if((consoleToUse = getenv("console")) == NULL)
				consoleToUse = defaultConsole;
		}

		int consoleFD = open(consoleToUse, O_RDWR | O_NONBLOCK | O_NOCTTY);
		if(consoleFD < 0){
			puts("Failed.");
		}else{
			dup2(consoleFD, 0);
			dup2(consoleFD, 1);
			dup2(consoleFD, 2);

			/* Terminal Setup Routine. Taken from Toybox's pending/init.c */
			struct termios defaultTerm;
			tcgetattr(consoleFD, &defaultTerm);
			defaultTerm.c_cc[VINTR] = 3;    //ctrl-c
			defaultTerm.c_cc[VQUIT] = 28;   /*ctrl-\*/
			defaultTerm.c_cc[VERASE] = 127; //ctrl-?
			defaultTerm.c_cc[VKILL] = 21;   //ctrl-u
			defaultTerm.c_cc[VEOF] = 4;     //ctrl-d
			defaultTerm.c_cc[VSTART] = 17;  //ctrl-q
			defaultTerm.c_cc[VSTOP] = 19;   //ctrl-s
			defaultTerm.c_cc[VSUSP] = 26;   //ctrl-z

			defaultTerm.c_line = 0;
			defaultTerm.c_cflag &= PARODD|PARENB|CSTOPB|CSIZE;
			defaultTerm.c_cflag |= CLOCAL|HUPCL|CREAD;

			//Enable start/stop input and output control + map CR to NL on input
			defaultTerm.c_iflag = IXON|IXOFF|ICRNL;

			//Map NL to CR-NL on output
			defaultTerm.c_oflag = ONLCR|OPOST;
			defaultTerm.c_lflag = IEXTEN|ECHOK|ECHOE|ECHO|ICANON|ISIG;
			tcsetattr(consoleFD, TCSANOW, &defaultTerm);
		}

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
