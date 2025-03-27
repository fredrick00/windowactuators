//
// Created by fredr on 3/21/2025.
//
#pragma once

#include <Arduino.h>
#define DEBUG_LEVEL 4 // 0 = no debug, 1 = debug level 1, 2 = debug level 2, 3 = debug level 3
// globally shared buffer for json payloads.
    constexpr int MAX_PAYLOAD_SIZE = 2048; // max size in bytes of the json payload.
/** obsolete
    static char BUFFER[MAX_PAYLOAD_SIZE];
    static size_t BUFFERLEN = 0;
    **/
    const size_t HEADER_SIZE = 8;
    static uint32_t expectedPayloadLength = 0;
    constexpr int JSON_FIELD_COUNT = 11;
    // Define our start and end markers.
    const String startMarker = "{\"actuators\": [ {\"";
    const String endMarker = "] }";
    const char arrayStart = '{';
    const char arrayEnd = '}';
    static int endPos = 0;
    static int startPos = 0;
    static String bugLog = ""; // let users store bug output until they print it.
/** debug data storage **/
    static uint32_t lastPayloadSize = 0;
    static char payloadHeader[HEADER_SIZE + 1] = "";
/** end debug data storage **/

// allow changing the log level for various code segments.
#define CURRENT_LOG_LEVEL 1 //
#define SET_BUG_LOG(val) \
    do { \
        if (CURRENT_LOG_LEVEL <= DEBUG_LEVEL) { \
            bugLog += String(val); \
        } \
    } while (0)

    #define DEBUG_PRINT(x) \
        do {\
          if (CURRENT_LOG_LEVEL <= DEBUG_LEVEL && bugLog.length() > 0) { \
            Serial.println(String("[") + __FUNCTION__ + "] " + String(bugLog) + String("[/end_") +  __FUNCTION__ + "]"); \
            bugLog = "";\
          } \
        } while (0)

