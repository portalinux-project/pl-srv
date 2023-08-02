/***************************************************************\
 pl-srv, v0.03
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv-error.c: pl-srv as a library, Error handling source file
\***************************************************************/
#include <libsrv.h>

void plSrvPanic(char* string, bool usePerror, bool developerBug){
	fputs("* ", stderr);
	plPanic(string, usePerror, developerBug);
}

void plStat(char* path, struct stat* statStruct){
	int retVal = stat(path, statStruct);

	if(retVal == -1)
		plSrvPanic("plStat: Infrastructure test failure", true, false);
}

long plSafeStrtonum(char* buffer){
	char* pointerHolder;
	long retNum = strtol(buffer, &pointerHolder, 10);

	if(pointerHolder != NULL && *pointerHolder != '\0' && *pointerHolder != '\n')
		plSrvPanic("plSafeStrtonum: Buffer was not a number", false, true);

	return retNum;
}

void plSrvInfraTest(void){
	struct stat srvDir;
	struct stat logDir;
	plStat("/etc/pl-srv", &srvDir);
	plStat("/var/pl-srv", &logDir);

	if(!S_ISDIR(srvDir.st_mode) || !S_ISDIR(logDir.st_mode))
		plSrvPanic("plSrvInfraTest: Infrastructure test failure: Not a directory", false, false);
}

plfile_t* plSrvSafeOpen(int mode, char* string, plmt_t* mt){
	if(!string || !mt)
		plSrvPanic("plSrvSafeOpen: NULL was passed as an argument", false, true);

	char curPath[256] = "";
	char fileMode[2] = "r";
	getcwd(curPath, 256);

	if(mode == PLSRV_START)
		chdir("/etc/pl-srv");
	else{
		chdir("/var/pl-srv");
		if(mode == PLSRV_START_LOCK)
			fileMode[0] = 'w';
	}

	plfile_t* retFile = plFOpen(string, fileMode, mt);
	if(!retFile)
		plSrvPanic("plSrvSafeOpen: Error opening file", true, false);

	chdir(curPath);
	return retFile;
}

void plSrvRemoveLock(char* service){
	char curPath[256] = "";
	getcwd(curPath, 256);

	chdir("/var/pl-srv");
	int retVal = remove(service);

	if(retVal == -1)
		plSrvPanic("plSrvRemoveLock: Error removing lock file", true, false);
}
