#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef void AsaHandler;

#define HANDLER_INTERFACE_VERSION_MAJOR 1

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t frame[];
} AsaVideoData;

typedef struct {
    uint32_t length;
    char data[];
} AsaStringData;

typedef enum {
    ASA_STATUS_NORMAL,
    ASA_STATUS_FATAL,
    ASA_STATUS_AGAIN,
    ASA_STATUS_EOI,
} AsaDataStatus;

typedef struct {
    AsaDataStatus status;
    float time;
    size_t size;
    uint8_t* data;
    char* error;
} AsaData;

typedef enum {
	ASA_NUMBER,
	ASA_BOOL,
	ASA_STRING,
	ASA_UNDEFIND,
	ASA_VIDEO,
	ASA_AUDIO
} AsaValueType;
