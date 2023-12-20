/**********************************************************\
 pl-srv, v0.05
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
		}else if(strcmp("depends", token.name.data.pointer) == 0){
			if(token.type != PLML_TYPE_STRING || !token.isArray)
				plRTPanic("plSrvGenerateServiceStruct", PLRT_ERROR | PLRT_INVALID_TOKEN, false);

			returnStruct.deps.pointer = plMTAlloc(mt, token.value.array.size * sizeof(plstring_t));
			returnStruct.deps.size = token.value.array.size;

			for(int i = 0; i < returnStruct.deps.size; i++){
				((plstring_t*)returnStruct.deps.pointer)[i].data = ((plptr_t*)token.value.array.pointer)[i];
				((plstring_t*)returnStruct.deps.pointer)[i].mt = mt;
				((plstring_t*)returnStruct.deps.pointer)[i].isplChar = false;
			}
		}

		buffer.data.size = 256;
	}

	return returnStruct;
}
