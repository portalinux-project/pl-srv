/***************************************************************\
 pl-srv, v0.07
 (c) 2024 CinnamonWolfy, Under MPL 2.0
 libsrv-error.c: pl-srv as a library, error handling source file
\***************************************************************/
#include <libsrv.h>

long plSrvStrtonum(char* buffer){
	char* holder;
	long retNum = strtol(buffer, &holder, 10);
	if(holder != NULL && *holder != '\0' && *holder != '\n')
		plRTPanic("plSrvStrtonum", PLRT_ERROR | PLRT_INVALID_TOKEN, true);

	return retNum;
}

bool plSrvCheckExist(char* path){
	struct stat pathStruct;
	if(stat(path, &pathStruct))
		return false;
	return true;
}

void plSrvStat(char* path, struct stat* statbuf){
	int retVal = stat(path, statbuf);
	if(retVal == -1)
		plRTPanic("plSrvStat", PLRT_ERROR | PLRT_ERRNO | errno, false);
}

bool plSrvIsServiceRunning(char* filename){
	char curPath[256] = "";
	getcwd(curPath, 256);

	chdir("/var/pl-srv/srv");
	bool retVal = plSrvCheckExist(filename);
	chdir(curPath);

	return retVal;
}

void plSrvInfraTest(void){
	struct stat srvDir;
	struct stat srvStatusDir;

	plSrvStat("/etc/pl-srv/srv", &srvDir);
	plSrvStat("/var/pl-srv/srv", &srvStatusDir);

	if(!S_ISDIR(srvDir.st_mode) || !S_ISDIR(srvDir.st_mode))
		plRTPanic("plSrvInfraTest", PLRT_ERROR | PLRT_NOT_DIR, false);
}

plfile_t* plSrvSafeOpen(plsrvactions_t mode, char* filename, plmt_t* mt){
	if(filename == NULL || mt == NULL)
		plRTPanic("plSrvSafeOpen", PLRT_ERROR | PLRT_NULL_PTR, false);

	char curPath[256] = "";
	plchar_t filemode = { .bytes = { 'r', '\0', '\0', '\0'} };
	getcwd(curPath, 256);

	if(mode == PLSRV_START){
		chdir("/etc/pl-srv/srv");
	}else{
		chdir("/var/pl-srv/srv");
		if(mode == PLSRV_START_LOCK)
			filemode.bytes[0] = 'w';
	}

	plfile_t* retFile = plFOpen(filename, (char*)filemode.bytes, mt);
	chdir(curPath);
	return retFile;
}

void plSrvRemoveLock(char* service){
	char curPath[256] = "";
	getcwd(curPath, 256);

	chdir("/var/pl-srv/srv");
	int retVal = remove(service);
	chdir(curPath);

	if(retVal == -1)
		plRTPanic("plSrvRemoveLock", PLRT_ERROR | PLRT_ERRNO | errno, false);
}
