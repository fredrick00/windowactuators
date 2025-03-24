//
// Created by fredr on 3/14/2025.
//
/* esp32_main.cpp */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "StatusReportFormatter.h"
#include "esp32Config.h"

namespace ActuatorsController {

  String accumulateSerialInput(Stream &stream);
  class StatusReportProcessor {
  public:
    // Constructor: takes a reference to an input Stream (e.g., Serial2)
    StatusReportProcessor(Stream &inputStream) : inStream(inputStream), report {} {}
    // Process incoming JSON data. If a complete report is read, fills in 'report'
    // and returns true; otherwise returns false.
    bool process(Stream &dataStream) {
      if (inStream.available()) {
        String jsonReport = accumulateSerialInput(inStream);
        // Check that the complete JSON was received before parsing
        if (jsonReport.length() == 0) {
          // debug output Serial.println ("no full report");
          return false;
        }
        Serial.print("have full report sized: ");
        Serial.print (jsonReport.length());
        Serial.print (" expected: ");
        Serial.println (payloadHeader);
        Serial.println(jsonReport);
        Serial.println ("end report\n");
        return parseReport(jsonReport, report);
      }
      return false;
    }


    // getReport simply returns a constant reference to the internal
    // report data structure.
    const StatusReportData &getReport() const {
      return report;
    }


    static void printReport(const StatusReportData &report) {
      for (uint8_t i = 0; i < report.actuatorCount; i++) {
        const ActuatorData &act = report.actuators[i];
        Serial.print("Timestamp: ");
        Serial.print(act.timestamp);
        Serial.print("Force mode: ");
        Serial.print(act.forceMode ? "true" : "false");
        Serial.print("Actuator ");
        Serial.print(act.index);
        Serial.print(": active=");
        Serial.print(act.active ? "true" : "false");
        Serial.print(", mode=");
        Serial.print(act.mode);
        Serial.print(", position=");
        Serial.print(act.position);
        Serial.print(", maxDuration=");
        Serial.println(act.maxDuration);
      }
    }

  private:
    Stream &inStream;
    StatusReportData report;
    // Parses the JSON report from the given String and populates 'report'.
    // Returns true if parsing was successful; false otherwise.
    bool parseReport(const String &jsonReport, StatusReportData &report) {
      StaticJsonDocument<MAX_PAYLOAD_SIZE> doc;
      DeserializationError error = deserializeJson(doc, jsonReport);
      if (error) {
        Serial.print("Failed to parse status report: ");
        Serial.println(error.f_str());
        return false;
      }
      JsonArray actuatorsArray = doc["actuators"].as<JsonArray>();
      // Loop through the actuators, ensuring we do not exceed the maximum allowed.
      for (JsonObject actuator : actuatorsArray) {
        // create an alias of act for actuators["index"]
        int idx = actuator["index"] | 0;
        if(idx < 0 || idx >= StatusReportData::MAX_ACTUATORS) {
          // Optionally handle error or skip this actuator entry.
          Serial.println ("Actuator index out of range");
          continue;
        }
        ActuatorData &act = report.actuators[idx];
        // Extract top-level data
        report.actuatorCount = actuator["actuatorCount"] | 0;
        act.timestamp = actuator["timestamp"] | 0;
        act.forceMode = actuator["forceMode"] | false;
        act.active = actuator["active"] | false;
        act.mode = actuator["mode"] | "";
        act.position = actuator["position"] | 0;
        act.maxDuration = actuator["maxDuration"] | 0;
        act.name = actuator["actuatorName"] | "";
      }
      return true;
    }

    bool payloadIsOversized(const String &payload) {
      return payload.length() > MAX_PAYLOAD_SIZE;
    }


    String accumulateSerialInput(Stream &stream) {
      // Append new characters from the stream into the global BUFFER.
      while (stream.available()) {
        char ch = stream.read();
        if (BUFFERLEN < MAX_PAYLOAD_SIZE - 1) {
          // Ensure room for null terminator.
          BUFFER[BUFFERLEN] = ch;
          BUFFERLEN++;
        }
      }
      BUFFER[BUFFERLEN] = '\0';
      // Always null-terminate the buffer.
      // Convert our global BUFFER into an Arduino String for easier searching.
      String bufStr(BUFFER);

      // Look for a valid start marker.
      int startIndex = bufStr.indexOf(startMarker);
      if (startIndex != -1) {
        // If there is extraneous data prior to the start marker,
        // discard it by using the substring that starts at startIndex.
        if (startIndex > 0) {
          bufStr = bufStr.substring(startIndex);
          startIndex = 0;
        }
        // Now search for the end marker after the start marker.
        int endIndex = bufStr.indexOf(endMarker, startIndex);
        if (endIndex != -1) {
          // The complete message runs from the start marker through the end marker.
          int completeMessageEnd = endIndex + endMarker.length();
          // Extract the full message.
          String message = bufStr.substring(0, completeMessageEnd);
          // Any remaining data after the complete message (e.g., the beginning of the next message)
          // should be shifted into the beginning of BUFFER for future processing.
          String remaining = bufStr.substring(completeMessageEnd);
          BUFFERLEN = remaining.length();
          if (BUFFERLEN > 0) {
            // Copy the remaining part back into BUFFER.
            strncpy(BUFFER, remaining.c_str(), MAX_PAYLOAD_SIZE);
          } else {
            BUFFER[0] = '\0';
          }
          // Return the complete JSON message.
          return message;
        }
      }
        // If we fall here, no complete message was found—so just return an empty String.
        return "";
    }

    /** **  Begin accumulateSerialInput which uses a header size. ** **
    char BUFFER[MAX_PAYLOAD_SIZE];
    size_t BUFFERLEN = 0;
    size_t expectedPayloadLength = 0;
    size_t lastPayloadSize = 0;
    String accumulateSerialInput(Stream &stream) {
      // // 0 means header not yet parsed
      // // Read available data from the stream up to the maximum buffer size.
      while (stream.available() > 0 && BUFFERLEN < MAX_PAYLOAD_SIZE) {
        int c = stream.read();
        if (c < 0) break;
        BUFFER[BUFFERLEN++] = static_cast<char>(c);
        // seek through input until we find a valid header
        while (BUFFERLEN >= HEADER_SIZE && expectedPayloadLength == 0) {
          // // extract the header and convert it from hexadecimal to a payload length.
            char header[HEADER_SIZE + 1] = { 0 };
            // // 8 characters plus a null terminator
            memcpy(header, BUFFER, HEADER_SIZE);
            header[HEADER_SIZE] = '\0';
            bool headerValid = true;
            for (size_t i = 0; i < HEADER_SIZE; ++i) {
              if (!isxdigit (header[i])) {
                headerValid = false;
                break;
              }
            }
          if (!headerValid) {
            // header is invalid.   Discard and seek again.
            memmove(BUFFER, BUFFER + 1, BUFFERLEN - 1);
            BUFFERLEN--;
            // skip ahead seeking a new header candidate or end of msg.
            continue;
          }
            expectedPayloadLength = strtoul (header, nullptr, 16);
            // Sanity check: if the expected payload is too large, reset the buffer.
            if (expectedPayloadLength > MAX_PAYLOAD_SIZE - HEADER_SIZE) {
              Serial.print("ERROR: Expected header payload too large. Resetting buffer:  ");
              Serial.println(expectedPayloadLength);

              Serial.print("BUFFER contents: ");
              for (size_t i = 0; i < BUFFERLEN; ++i) {
                Serial.print(BUFFER[i]);
              }
              Serial.println();

              expectedPayloadLength = 0;
              BUFFERLEN = 0;
              BUFFER[0] = '\0';
              return String();
            }
            // Reset for the next message.
            // expectedPayloadLength = 0;
        }
        // // If we know how long the payload should be and we have enough data,
        // // break out of the loop.
        if (expectedPayloadLength > 0 && BUFFERLEN >= (HEADER_SIZE + expectedPayloadLength)) {
          break;
        }
      }
      size_t remaining = 0;
      size_t result = 0;
      size_t completeMessageLength = 0;
      String message = "";
      // // Check if a complete message (header + payload) has been received.
      if (expectedPayloadLength > 0 && BUFFERLEN >= (HEADER_SIZE + expectedPayloadLength)) {
        // // Extract the JSON payload after the header.
        message = String(BUFFER + HEADER_SIZE, expectedPayloadLength);
        // If there is extra data beyond the complete message, shift it to the front of BUFFER.
        completeMessageLength = HEADER_SIZE + expectedPayloadLength;
        remaining = BUFFERLEN - completeMessageLength;
        if (remaining > 0) {
          memmove(BUFFER, BUFFER + completeMessageLength, remaining);
        }
        BUFFERLEN = remaining;

        // Complete message detected; reset the header state so that the next complete message can be processed.
        lastPayloadSize = expectedPayloadLength;
        expectedPayloadLength = 0;
        // Return the accumulated payload.
        return message;
      }
      // neither header nor complete message were received, so continue watching for the message.
      return String();
    }
    ** **  End accumulateSerialInput which uses a header size. **/


    inline bool payloadIsOversized(size_t payloadLength) {
      return payloadLength > MAX_PAYLOAD_SIZE;
      // Assuming MAX_PAYLOAD_SIZE is defined elsewhere.
    }


    // Extract the valid JSON portion from a raw payload that may include additional framing data.
    inline String processPayload(const char* payload, size_t length) {
      // Build a String from the raw data.
      String rawPayload(payload, length);
      // Find the start and end of the JSON object based on curly braces
      int jsonStart = rawPayload.indexOf('{');
      int jsonEnd = rawPayload.lastIndexOf('}');
      // If valid markers are found, extract the substring.
      if (jsonStart != -1 && jsonEnd != -1 && jsonEnd > jsonStart) {
        return rawPayload.substring(jsonStart, jsonEnd + 1);
      }
      // If markers are not found, fallback to trimming whitespace.
      rawPayload.trim();
      return rawPayload;
    }

    /**
    inline String processPayload(const char* payload, size_t length) {
      // Create a JSON document sized appropriately.
      // (Adjust the capacity to match your JSON payload sizes.)
      // Construct a temporary String for deserialization.
      String jsonPayload(payload);
      jsonPayload = jsonPayload.substring(0, length);
      DeserializationError error = deserializeJson(doc, jsonPayload);
      if (error) { Serial.print("JSON parsing failed: ");
        Serial.println(error.f_str());
        return "";
      }
      String jsonReport;
      serializeJson(doc, jsonReport);
      return jsonReport;
    }
**/

  };
} // namespace ActuatorsController