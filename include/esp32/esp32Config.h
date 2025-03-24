//
// Created by fredr on 3/21/2025.
//
#pragma once

#include <Arduino.h>

// globally shared buffer for json payloads.
    constexpr int MAX_PAYLOAD_SIZE = 256; // max size in bytes of the json payload.
    static char BUFFER[MAX_PAYLOAD_SIZE];
    static size_t BUFFERLEN = 0;
    const size_t HEADER_SIZE = 8;
    static uint32_t expectedPayloadLength = 0;
    // Define our start and end markers.
    const String startMarker = "{\"actuators\": [ {\"";
    const String endMarker = "] }";
/** debug data storage **/
    static uint32_t lastPayloadSize = 0;
    static char payloadHeader[HEADER_SIZE + 1] = "";
/** end debug data storage **/
