#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef void AsaHandler;

typedef struct {
    float time;
    size_t size;
    void* data;
} AsaData;

typedef enum {
	ASA_NUMBER,
	ASA_BOOL,
	ASA_STRING,
	ASA_UNDEFIND,
	ASA_VIDEO,
	ASA_AUDIO
} AsaValueType;
