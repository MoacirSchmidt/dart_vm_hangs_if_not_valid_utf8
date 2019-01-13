// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdafx.h"
#include "include/dart_api.h"
#include "include/dart_native_api.h"


Dart_NativeFunction ResolveName(Dart_Handle name,
                                int argc,
                                bool* auto_setup_scope);


DART_EXPORT Dart_Handle sample_extension_Init(Dart_Handle parent_library) {
  if (Dart_IsError(parent_library)) {
    return parent_library;
  }

  Dart_Handle result_code =
      Dart_SetNativeResolver(parent_library, ResolveName, NULL);
  if (Dart_IsError(result_code)) {
    return result_code;
  }

  return Dart_Null();
}


Dart_Handle HandleError(Dart_Handle handle) {
  if (Dart_IsError(handle)) {
    Dart_PropagateError(handle);
  }
  return handle;
}

void wrappedRandomArray(Dart_Port dest_port_id,
                        Dart_CObject* message) {
  Dart_Port reply_port_id = ILLEGAL_PORT;
  if (message->type == Dart_CObject_kArray &&
      1 == message->value.as_array.length) {
    // Use .as_array and .as_int32 to access the data in the Dart_CObject.
    Dart_CObject* param0 = message->value.as_array.values[0];
    if (param0->type == Dart_CObject_kSendPort) {
	    reply_port_id = param0->value.as_send_port.id;
		Dart_CObject result;
		result.type = Dart_CObject_kString;
		result.value.as_string = new char[10];
		result.value.as_string[0] = 0x4D; // 'M'; // 
		result.value.as_string[1] = 0xEA; // 'ê'; // invalid Utf-8 char (this is ANSI!)
		result.value.as_string[2] = 0x73; // 's';
		result.value.as_string[3] = '\0';
		Dart_PostCObject(reply_port_id, &result);
		// It is OK that result is destroyed when function exits.
		// Dart_PostCObject has copied its data.
		return;
      }
  }
  Dart_CObject result;
  result.type = Dart_CObject_kNull;
  Dart_PostCObject(reply_port_id, &result);
}


void randomArrayServicePort(Dart_NativeArguments arguments) {
  Dart_EnterScope();
  Dart_SetReturnValue(arguments, Dart_Null());
  Dart_Port service_port =
      Dart_NewNativePort("RandomArrayService", wrappedRandomArray, true);
  if (service_port != ILLEGAL_PORT) {
    Dart_Handle send_port = HandleError(Dart_NewSendPort(service_port));
    Dart_SetReturnValue(arguments, send_port);
  }
  Dart_ExitScope();
}


struct FunctionLookup {
  const char* name;
  Dart_NativeFunction function;
};


FunctionLookup function_list[] = {
    {"RandomArray_ServicePort", randomArrayServicePort},
    {NULL, NULL}};


Dart_NativeFunction ResolveName(Dart_Handle name,
                                int argc,
                                bool* auto_setup_scope) {
  if (!Dart_IsString(name)) {
    return NULL;
  }
  Dart_NativeFunction result = NULL;
  if (auto_setup_scope == NULL) {
    return NULL;
  }

  Dart_EnterScope();
  const char* cname;
  HandleError(Dart_StringToCString(name, &cname));

  for (int i=0; function_list[i].name != NULL; ++i) {
    if (strcmp(function_list[i].name, cname) == 0) {
      *auto_setup_scope = true;
      result = function_list[i].function;
      break;
    }
  }

  if (result != NULL) {
    Dart_ExitScope();
    return result;
  }

  Dart_ExitScope();
  return result;
}
