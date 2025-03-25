//
// Created by fredr on 3/21/2025.
//
#pragma once

#include <Arduino.h>
#define DEBUG_LEVEL 1 // 0 = no debug, 1 = debug level 1, 2 = debug level 2, 3 = debug level 3
// globally shared buffer for json payloads.
    constexpr int MAX_PAYLOAD_SIZE = 1024; // max size in bytes of the json payload.
/** obsolete
    static char BUFFER[MAX_PAYLOAD_SIZE];
    static size_t BUFFERLEN = 0;
    **/
    const size_t HEADER_SIZE = 8;
    static uint32_t expectedPayloadLength = 0;
    // Define our start and end markers.
    const String startMarker = "{\"actuators\": [ {\"";
    const String endMarker = "] }";
    static int endPos = 0;
    static int startPos = 0;
/** debug data storage **/
    static uint32_t lastPayloadSize = 0;
    static char payloadHeader[HEADER_SIZE + 1] = "";
/** end debug data storage **/

#if DEBUG_LEVEL >= 1
    #define DEBUG_PRINT_LEVEL_1(x) Serial.print(x) // Prints for level 1 and above
#else
    #define DEBUG_PRINT_LEVEL_1(x) // No operation
#endif

#if DEBUG_LEVEL >= 2
    #define DEBUG_PRINT_LEVEL_2(x) Serial.print(x) // Prints for level 2 and above
#else
    #define DEBUG_PRINT_LEVEL_2(x) // No operation
#endif

#if DEBUG_LEVEL >= 3
    #define DEBUG_PRINT_LEVEL_3(x) Serial.print(x) // Prints for level 3 and above
#else
    #define DEBUG_PRINT_LEVEL_3(x) // No operation
#endif
