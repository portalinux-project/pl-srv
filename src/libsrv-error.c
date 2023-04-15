/***************************************************************\
 pl-srv, v0.02
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv-error.c: pl-srv as a library, Error handling source file
\***************************************************************/
#include <libsrv.h>

void plSrvErrNoRet(char* string, bool usePerror, bool developerBug){
	if(usePerror)
		perror(string);
	else
		puts(string);

	if(developerBug)
		puts("* If you're seeing this error message, this is a bug. Please report this error to the developers of this application");

	exit(1);
}

void plStat(char* path, struct stat* statStruct){
	int retVal = stat(path, statStruct);

	if(retVal == -1)
		plSrvErrorNoRet("* Infrastructure test failure", true, false);
}

void plSrvInfraTest(){
	struct stat srvDir;
	struct stat logDir;
	plStat("/etc/pl-srv", &srvDir);
	plStat("/var/pl-srv", &logDir);

	if(!S_ISDIR(srvDir.st_mode) || !S_ISDIR(logDir.st_mode))
		plSrvErrorNoRet("* Infrastructure test failure: Not a directory", false, false);
}

plfile_t* plSrvSafeOpen(int mode, char* string, plmt_t* mt){
	if(!string || !mt)
		plSrvErrNoRet("* plSrvSafeOpen: NULL was passed as an argument", false, true);

	char curPath[256] = "";
	getcwd(curPath, 256);

	if(mode == PLSRV_START)
		chdir("/etc/pl-srv");
	else
		chdir("/var/pl-srv");

	plfile_t* retFile = plFOpen(string, NULL, mt);
	if(!retFile)
		plSrvErrorNoRet("* Error opening file", true, false);

	return retFile;
}
