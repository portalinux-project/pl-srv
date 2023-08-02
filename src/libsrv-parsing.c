/***********************************************************************\
 pl-srv, v0.03
 (c) 2023 pocketlinux32, Under MPLv2.0
 libsrv-parsing.c: pl-srv as a library, Service file parsing source file
\***********************************************************************/
#include <libsrv.h>

plsrv_t* plSrvGenerateServiceStruct(plfile_t* srvFile, plmt_t* mt){
	if(srvFile == NULL || mt == NULL)
		return NULL;

	plsrv_t* returnStruct = plMTAllocE(mt, sizeof(plsrv_t));
	returnStruct->respawn = false;
	returnStruct->background = false;

	byte_t buffer[256] = "";
	while(plFGets(buffer, 256, srvFile) != NULL){
		plmltoken_t* plmlToken = plMLParse(buffer, mt);
		string_t tokenName;

		plMLGetTokenAttrib(plmlToken, &tokenName, PLML_GET_NAME);

		if(strcmp("exec", tokenName) == 0){
			string_t tokenVal;
			plMLGetTokenAttrib(plmlToken, &tokenVal, PLML_GET_VALUE);

			plarray_t* tokenizedVal = plParser(tokenVal, mt);
			plMTRealloc(mt, tokenizedVal->array, (tokenizedVal->size + 1) * sizeof(string_t*));
			((string_t*)tokenizedVal->array)[tokenizedVal->size] = NULL;

			returnStruct->args = tokenizedVal->array;
			returnStruct->path = ((string_t*)tokenizedVal->array)[0];

			plMTFree(mt, tokenizedVal);
		}else if(strcmp("respawn", tokenName) == 0){
			bool* tokenVal;
			plMLGetTokenAttrib(plmlToken, &tokenVal, PLML_GET_VALUE);
			returnStruct->respawn = *tokenVal;
		}else if(strcmp("background", tokenName) == 0){
			bool* tokenVal;
			plMLGetTokenAttrib(plmlToken, &tokenVal, PLML_GET_VALUE);
			returnStruct->background = *tokenVal;
		}

		plMLFreeToken(plmlToken);
	}

	return returnStruct;
}
