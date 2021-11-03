#pragma once
#include <stdint.h>
#include <vector>
#include <iostream>
#include <string>

#ifndef _COMMON_H
#define _COMMON_H

char neeble_to_hex(uint8_t byte);
void print_payload(std::ostream &output, std::vector<uint8_t> payload);

typedef enum error_code_e
{
    CLIENT__ERROR = 0,
    CLIENT__SUCCESS,
    CLIENT__CONNECTION_FAILED,
    CLIENT__FAILED_TO_GET_SERVER_INFO,
    CLIENT__INVLAID_SERVER_INFO,
    CLIENT__FALID_TO_SEND_DATA,
    CLIENT__FAILED_TO_RECV_DATA,
    REQUEST__ERROR,
    REQUEST__SUCCESS,
    REQUEST__FAILED_TO_BUILD,
    REQUEST__INVALID_REQUEST,
    REGISTER_OPERATION__ERROR,
    REGISTER_OPERATION__SUCCESS,

} error_code_t;


#define SUPPORTED_VERSION  (2)
#define PADDING_BYTE (0)
#define FILLING_BYTE ("\x11")

#define PUBLIC_KEY_SIZE (160)
#define NAME_SIZE (255)
#define SYMMETRIC_KEY_SIZE (64)

#define MAX_PAYLOAD_SIZE (65536)

#define SERVER_INFO_FILE ("server.info")
#define CLIENT_ME_FILE   ("me.info")


#endif // _COMMON_H