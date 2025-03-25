//
// Created by fredr on 3/14/2025.
//
/* esp32_main.cpp */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "StatusReportFormatter.h"
#include "esp32Config.h"

#define DEBUG_LEVEL_1
namespace ActuatorsController {

  // predeclarations
  String accumulateSerialInput(Stream &stream);
  String appendStreamToBuffer(Stream &stream);
  bool purgeUntilStartMarker(String &buffer);
  String extractCompleteMessage(String &buffer);
  bool payloadIsOversized(String &buffer);
  String processPayload(const char* payload, size_t length);

  class StatusReportProcessor {
  public:
    // Constructor: takes a reference to an input Stream (e.g., Serial2)
    StatusReportProcessor(Stream &inputStream) : inStream(inputStream), report {} {}
    // Process incoming JSON data. If a complete report is read, fills in 'report'
    // and returns true; otherwise returns false.

    bool process(Stream &dataStream) {
      if (inStream.available()) {
        String jsonReport = accumulateSerialInput(inStream);
        DEBUG_PRINT_LEVEL_2 ("have report sized: ");
        DEBUG_PRINT_LEVEL_2  (jsonReport.length());
        DEBUG_PRINT_LEVEL_2  (" for stream with contents:");
        DEBUG_PRINT_LEVEL_2 (jsonReport);
        DEBUG_PRINT_LEVEL_2 ("\nEnd Report\n");
        // Check that the complete JSON was received before parsing
        if (jsonReport.length() == 0) {
          // debug output
          DEBUG_PRINT_LEVEL_2  ("no full report");

          return false;
        }
        DEBUG_PRINT_LEVEL_2 ("have full report sized: ");
        DEBUG_PRINT_LEVEL_2  (jsonReport.length());
        DEBUG_PRINT_LEVEL_2  (" expected: ");
        DEBUG_PRINT_LEVEL_2  (payloadHeader);
        DEBUG_PRINT_LEVEL_2 (jsonReport);
        DEBUG_PRINT_LEVEL_2  ("end report\n");
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
        DEBUG_PRINT_LEVEL_2 ("Timestamp: ");
        DEBUG_PRINT_LEVEL_2 (act.timestamp);
        DEBUG_PRINT_LEVEL_2 ("Force mode: ");
        DEBUG_PRINT_LEVEL_2 (act.forceMode ? "true" : "false");
        DEBUG_PRINT_LEVEL_2 ("Actuator ");
        DEBUG_PRINT_LEVEL_2 (act.index);
        DEBUG_PRINT_LEVEL_2 (": active=");
        DEBUG_PRINT_LEVEL_2 (act.active ? "true" : "false");
        DEBUG_PRINT_LEVEL_2 (", mode=");
        DEBUG_PRINT_LEVEL_2 (act.mode);
        DEBUG_PRINT_LEVEL_2 (", position=");
        DEBUG_PRINT_LEVEL_2 (act.position);
        DEBUG_PRINT_LEVEL_2 (", maxDuration=");
        DEBUG_PRINT_LEVEL_2 (act.maxDuration);
      }
    }
   /**
    *   Begin private section of the class
    *
    *   **/
  private:
    Stream &inStream;
    StatusReportData report;
    String internalBuffer = "";
    // Parses the JSON report from the given String and populates 'report'.
    // Returns true if parsing was successful; false otherwise.
    bool parseReport(const String &jsonReport, StatusReportData &report) {
        StaticJsonDocument<MAX_PAYLOAD_SIZE> doc; // Max size respected for full parsing
        DeserializationError error = deserializeJson(doc, jsonReport);
        if (error) {
            DEBUG_PRINT_LEVEL_2("Failed to parse status report: ");
            DEBUG_PRINT_LEVEL_2(error.f_str());
            return false;
        }

        // Get the JSON array of actuators
        JsonArray actuatorsArray = doc["actuators"].as<JsonArray>();
        if (actuatorsArray.isNull()) {
            DEBUG_PRINT_LEVEL_2("No actuators array found in the report.");
            return false;
        }

        // Process each entry in the actuators array
        for (JsonObject actuator : actuatorsArray) {
            // Extract the "index" field and validate its range
            int idx = actuator["index"] | -1;
            if (idx < 0 || idx >= StatusReportData::MAX_ACTUATORS) {
                DEBUG_PRINT_LEVEL_2("Actuator index out of range or not present.");
                continue; // Skip invalid or out-of-range actuator entries
            }

            ActuatorData &act = report.actuators[idx]; // Reference the specific actuator's data

            // DEBUG: Log the received index for clarity
            DEBUG_PRINT_LEVEL_2("Updating actuator at index: " + String(idx));

            // **Only update values if present in the JSON object**:
            if (actuator.containsKey("timestamp")) {
                act.timestamp = actuator["timestamp"];
            }
            if (actuator.containsKey("forceMode")) {
                act.forceMode = actuator["forceMode"];
            }
            if (actuator.containsKey("active")) {
                act.active = actuator["active"];
            }
            if (actuator.containsKey("mode")) {
                act.mode = actuator["mode"].as<String>();
            }
            if (actuator.containsKey("position")) {
                act.position = actuator["position"];
            }
            if (actuator.containsKey("maxDuration")) {
                act.maxDuration = actuator["maxDuration"];
            }
            if (actuator.containsKey("actuatorName")) {
                act.name = actuator["actuatorName"].as<String>();
            }
            DEBUG_PRINT_LEVEL_1 ("Actuator " + String(idx) + " updated.");
            // Optionally, update the count of received actuators (if this is relevant)
            report.actuatorCount++;
        }

        return true;
    }


    /**
    bool parseReport(const String &jsonReport, StatusReportData &report) {
      StaticJsonDocument<MAX_PAYLOAD_SIZE> doc;
      DeserializationError error = deserializeJson(doc, jsonReport);
      if (error) {
        DEBUG_PRINT_LEVEL_2 ("Failed to parse status report: ");
        DEBUG_PRINT_LEVEL_2 (error.f_str());
        return false;
      }
      JsonArray actuatorsArray = doc["actuators"].as<JsonArray>();
      // Loop through the actuators, ensuring we do not exceed the maximum allowed.
      for (JsonObject actuator : actuatorsArray) {
        // create an alias of act for actuators["index"]
        int idx = actuator["index"] | 0;
        if(idx < 0 || idx >= StatusReportData::MAX_ACTUATORS) {
          // Optionally handle error or skip this actuator entry.
          DEBUG_PRINT_LEVEL_2  ("Actuator index out of range");
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
    }**/

    bool payloadIsOversized(const String &payload) {
      return payload.length() > MAX_PAYLOAD_SIZE;
    }

    String accumulateSerialInput(Stream &stream) {
        // Use appendStreamToBuffer to extract incoming data into the internal buffer
        String completeMessage = appendStreamToBuffer(stream);
        // Process and clean buffer, ensuring valid start marker detection or handle payload issues

            if (completeMessage == "") {
              // no message found in stream.
              DEBUG_PRINT_LEVEL_2 ("No complete message found in stream.");
              return "";
            } else {
                // Extract the complete message
                // duplicate action internalBuffer.remove(0, endPos + endMarker.length());
              DEBUG_PRINT_LEVEL_2  ("Found complete message in stream.  Contents:");
              DEBUG_PRINT_LEVEL_2 (completeMessage);
              DEBUG_PRINT_LEVEL_2  ("End complete message.\n");
                return completeMessage;  // Return the valid JSON message
            }

        // If no complete message, continue buffering
        return "";
    }




  bool purgeUntilStartMarker(String &buffer) {
    int markerPos = buffer.indexOf(startMarker);
    if (markerPos == -1) {
        // No start marker in the buffer, keep it empty
        DEBUG_PRINT_LEVEL_2 ("[DEBUG] No start marker found in buffer.");
        buffer = "";
        return false;
    }
    // call helper function to strip the hex header and store it in global payloadHeader.
    markerPos = stripEmHex (buffer, markerPos);
  // Purge everything before the start marker
    DEBUG_PRINT_LEVEL_2 ("[DEBUG] Purging buffer up to startMarker position: ");
    DEBUG_PRINT_LEVEL_2 (markerPos);

    buffer.remove(0, markerPos);
    return true;
}

      // Helper function to strip the hex header and update the marker position
      int stripEmHex(String & buffer, int markerPos) {
        // Assuming the hex-encoded portion is at the beginning of the buffer and ends before the start marker.
        // The implementation strips the hex portion and recalculates the position of the start marker.

        int hexEndPos = buffer.indexOf(startMarker, 0); // Find where the hex header ends
        if (hexEndPos != -1 && hexEndPos < markerPos) {
          // Remove the hex-encoded portion and adjust the marker position accordingly
          buffer.substring(0, hexEndPos).toCharArray(payloadHeader, sizeof(payloadHeader));
          buffer.remove(0, hexEndPos);
          markerPos -= hexEndPos; // Adjust marker position
        }

        return markerPos;
      }

String extractCompleteMessage(String &buffer) {
    int startIdx = buffer.indexOf(startMarker);
    int endIdx = buffer.indexOf(endMarker, startIdx);

    if (startIdx != -1 && endIdx != -1) {
        // DEBUG: Log complete message extraction
        DEBUG_PRINT_LEVEL_1("Extracting complete message from buffer.");

        // Extract complete JSON message
        String completeMessage = buffer.substring(startIdx, endIdx + endMarker.length());

        // Remove processed portion of the buffer
        buffer = buffer.substring(endIdx + endMarker.length());

        return completeMessage; // Return valid JSON string
    }

    // If no complete message, return an empty string
    return "";
}

/** replacing during trouble-shooting
    String extractCompleteMessage(String &buffer) {
       // Find the start marker.
       startPos = buffer.indexOf(startMarker);
       if (startPos == -1) return "";
       // Look for the end marker after the start marker.
       endPos = buffer.indexOf(endMarker, startPos);
       if (endPos == -1) return "";

       // The complete message includes the end marker.
       int messageLength = (endPos + endMarker.length()) - startPos;
       return buffer.substring(startPos, startPos + messageLength);
    }
**/
    String appendStreamToBuffer(Stream &stream) {
        // Read new data from the stream and append it to the internal buffer
      DEBUG_PRINT_LEVEL_2 ("appendStreamToBuffer:\n");
        char c;
        static String internalBuffer;          // Declare the static buffer which will persist between calls
        DEBUG_PRINT_LEVEL_1(String("Internal buffer so far: ") + internalBuffer);
        while (stream.available()) {
            c = stream.read();  // Read one character
            internalBuffer += c;                   // Append to the buffer
        }
      DEBUG_PRINT_LEVEL_1(internalBuffer);
        return processInternalBuffer();
    }
      /**
        while (internalBuffer.length() > 0) {
          // Look for the endMarker in the current buffer
          //int endPos = internalBuffer.indexOf(endMarker);
          // if (endPos != -1) { // endMarker found
          // Now confirm we have a valid startMarker before the endMarker
          DEBUG_PRINT_LEVEL_2 ("appendStreamToBuffer: pre-purgeUntilStartMarker:");
          DEBUG_PRINT_LEVEL_2 (internalBuffer);
          DEBUG_PRINT_LEVEL_2  ("End buffer contents.\n");
          if (purgeUntilStartMarker(internalBuffer)) {
            // Found a complete message delimited by startMarker and endMarker
            String completeMessage = internalBuffer;
            DEBUG_PRINT_LEVEL_2  ("appendStreamToBuffer: post-purgeUntilStartMarker:");
            DEBUG_PRINT_LEVEL_2 (internalBuffer);
            DEBUG_PRINT_LEVEL_2  ("End buffer contents.\n");

            DEBUG_PRINT_LEVEL_2 ("Found complete message in buffer.  Old buffer contents:");
            DEBUG_PRINT_LEVEL_2 (internalBuffer);
            // Remove the processed data from the buffer (including the endMarker)
            internalBuffer.remove(0, endPos + endMarker.length());
            DEBUG_PRINT_LEVEL_2  ("  New buffer contents:");
            DEBUG_PRINT_LEVEL_2 (internalBuffer);
            DEBUG_PRINT_LEVEL_2  ("End buffer contents.\n");
            return completeMessage;  // Return the complete extracted message
          } else {
            //// If there's no valid startMarker, purge data.
            //internalBuffer = "";
          }
          //}

          // If the buffer exceeds the maximum allowed size, clear it to avoid memory issues
          if (internalBuffer.length() > MAX_PAYLOAD_SIZE) {
            DEBUG_PRINT_LEVEL_2 ("Buffer exceeded max allowed size. Clearing buffer.");
            internalBuffer = "";  // Reset buffer completely
          }
        } **/

      String processInternalBuffer() {
            String completeMessage = "["; // Start a JSON array
            bool isFirstEntry = true;     // Track if it's the first JSON message in the array

            while (internalBuffer.length() > 0) {
                DEBUG_PRINT_LEVEL_2("Processing internalBuffer...");
                DEBUG_PRINT_LEVEL_2(internalBuffer);

                // Remove leading garbage or hex sequence, ensuring valid startMarker remains
                if (!purgeUntilStartMarker(internalBuffer)) {
                    // If no valid startMarker is found, clear the buffer and exit
                    DEBUG_PRINT_LEVEL_2("Start marker not found. Purging invalid data.");
                    internalBuffer = "";
                    break;
                }

                // Locate the end marker
                int endPos = internalBuffer.indexOf(endMarker);
                if (endPos != -1) {
                    // Extract the message between the startMarker and endMarker
                    int startPos = internalBuffer.indexOf(startMarker);
                    if (startPos != -1 && startPos < endPos) {
                        // Extract message
                        String message = internalBuffer.substring(startPos, endPos + endMarker.length());

                        // Append extracted message to the JSON array (add a comma for all but the first)
                        if (!isFirstEntry) {
                            completeMessage += ",";
                        }
                        completeMessage += message;
                        isFirstEntry = false; // Toggle flag after appending the first entry

                        // Remove processed data from the buffer
                        internalBuffer.remove(0, endPos + endMarker.length());
                        DEBUG_PRINT_LEVEL_2("Extracted valid message:");
                        DEBUG_PRINT_LEVEL_2(message);
                    } else {
                        // Invalid structure: no valid startMarker before endMarker, purging buffer
                        DEBUG_PRINT_LEVEL_2("Invalid message structure. Purging buffer.");
                        internalBuffer = "";
                        break;
                    }
                } else {
                    // No endMarker found, stop processing (wait for more data to arrive)
                    DEBUG_PRINT_LEVEL_2("End marker not found. Waiting for more data.");
                    break;
                }

                // Prevent buffer overflow by resetting if it grows too large
                if (internalBuffer.length() > MAX_PAYLOAD_SIZE) {
                    DEBUG_PRINT_LEVEL_2("Buffer exceeded max allowed size. Clearing buffer to avoid overflow.");
                    internalBuffer = "";
                }
            }

            // Close the JSON array if any messages were added
            completeMessage += "]";

            // If no messages were appended, return an empty array
            if (completeMessage == "[]") {
                DEBUG_PRINT_LEVEL_2("No valid messages found in internalBuffer.");
                completeMessage = "";
            }

            return completeMessage;
        }


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


  };
} // namespace ActuatorsController