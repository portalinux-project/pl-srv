/***************************************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv-error.c: pl-srv as a library, Error handling source file
\***************************************************************/
#include <libsrv.h>

void plSrvErrNoRet(char* string, bool perrorFlag){
	if(perrorFlags)
		perror(string);
	else
		puts(string);
	exit(1);
}

void plStat(char* path, struct stat* statStruct){
	int retVal = stat(path, statStruct);

	if(retVal == -1)
		plSrvErrorNoRet("* Infrastructure test failure", true);
}

plfile_t* plSrvOpenFile(int mode, char* string){
	struct stat srvDir;
	struct stat logDir;
	

	plStat("/etc/pl-srv", &srvDir)
	plStat("/var/pl-srv", &logDir)

	if(!S_ISDIR(srvDir->st_mode) || !S_ISDIR(logDir->st_mode))
		plSrvErrorNoRet("* Infrastructure test failure: Not a directory", false);

}
