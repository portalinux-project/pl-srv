/**********************************************************\
 pl-srv, v0.04
 (c) 2023 pocketlinux32, Under MPL 2.0
 libsrv-parsing.c: pl-srv as a library, parsing source file
\**********************************************************/
#include <libsrv.h>

plsrv_t plSrvGenerateServiceStruct(plfile_t* srvFile, plmt_t* mt){
	if(srvFile == NULL || mt == NULL)
		plRTPanic("plSrvGenerateServiceStruct", PLRT_ERROR | PLRT_NULL_PTR, true);

	plsrv_t returnStruct = {
		.args = {
			.pointer = NULL,
			.size = 0
		},
		.deps = {
			.pointer = NULL,
			.size = 0
		},
		.respawn = false,
		.background = false
	};

	char rawBuf[256] = "";
	plstring_t buffer = {
		.data = {
			.pointer = rawBuf,
			.size = 256
		},
		.isplChar = false,
		.mt = NULL
	};
	while(plFGets(&buffer, srvFile) != 1){
		plmltoken_t token = plMLParse(buffer, mt);

		if(strcmp("exec", token.name.data.pointer) == 0){
			if(token.type != PLML_TYPE_STRING)
				plRTPanic("plSrvGenerateServiceStruct", PLRT_ERROR | PLRT_INVALID_TOKEN, false);

			returnStruct.args = plRTParser(plRTStrFromCStr(token.value.string.pointer, NULL), mt);
		}else if(strcmp("respawn", token.name.data.pointer) == 0){
			if(token.type != PLML_TYPE_BOOL)
				plRTPanic("plSrvGenerateServiceStruct", PLRT_ERROR | PLRT_INVALID_TOKEN, false);

			returnStruct.respawn = token.value.boolean;
		}else if(strcmp("background", token.name.data.pointer) == 0){
			if(token.type != PLML_TYPE_BOOL)
				plRTPanic("plSrvGenerateServiceStruct", PLRT_ERROR | PLRT_INVALID_TOKEN, false);

			returnStruct.background = token.value.boolean;
		}

		buffer.data.size = 256;
	}

	return returnStruct;
}
