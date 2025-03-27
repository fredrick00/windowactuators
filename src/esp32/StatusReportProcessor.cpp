
//
// Created by fredr on 3/26/2025.
//
#include "esp32/StatusReportProcessor.h"
#include "esp32/esp32Config.h"
using namespace ActuatorsController;


    bool StatusReportProcessor::process(Stream &dataStream) {
      if (inStream.available()) {
        String jsonReport = accumulateSerialInput(inStream);
        #undef CURRENT_LOG_LEVEL
        #define CURRENT_LOG_LEVEL 3
        SET_BUG_LOG ("have report sized: ");
        SET_BUG_LOG  (jsonReport.length());
        SET_BUG_LOG  (" for stream with contents:\n");
        SET_BUG_LOG (jsonReport);
        SET_BUG_LOG ("\nEnd Report\n");
        DEBUG_PRINT();

        // Check that the complete JSON was received before parsing
        if (jsonReport.length() == 0) {
          // debug output
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
          SET_BUG_LOG  ("no full report");
          DEBUG_PRINT();
          return false;
        }
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 3
        SET_BUG_LOG ("have full report sized: ");
        SET_BUG_LOG  (jsonReport.length());
        SET_BUG_LOG  (" expected: ");
        SET_BUG_LOG  (payloadHeader);
        SET_BUG_LOG (jsonReport);
        SET_BUG_LOG  ("end report\n");
        DEBUG_PRINT();
        return parseReport(jsonReport, report);
      }
      return false;
    }


    // getReport simply returns a constant reference to the internal
    // report data structure.
    const StatusReportData& StatusReportProcessor::getReport() const {
      return report;
    }


    void StatusReportProcessor::printReport(const StatusReportData &report) {
      for (uint8_t i = 0; i < report.actuatorCount; i++) {
        const ActuatorData &act = report.actuators[i];
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
        SET_BUG_LOG ("Timestamp: ");
        SET_BUG_LOG (act.timestamp);
        SET_BUG_LOG (", Force mode: ");
        SET_BUG_LOG (act.forceMode ? "true" : "false");
        SET_BUG_LOG (", Actuator ");
        SET_BUG_LOG (act.index);
        SET_BUG_LOG (", active=");
        SET_BUG_LOG (act.active ? "true" : "false");
        SET_BUG_LOG (", mode=");
        SET_BUG_LOG (act.mode);
        SET_BUG_LOG (", position=");
        SET_BUG_LOG (act.position);
        SET_BUG_LOG (", maxDuration=");
        SET_BUG_LOG (act.maxDuration);
        DEBUG_PRINT();
      }
    }


    // Parses the JSON report from the given String and populates 'report'.
    // Returns true if parsing was successful; false otherwise.
    bool StatusReportProcessor::parseReport(const String &jsonReport, StatusReportData &reportToParse) {
        StaticJsonDocument<MAX_PAYLOAD_SIZE> doc; // Max size respected for full parsing
        DeserializationError error = deserializeJson(doc, jsonReport);
        if (error) {
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
            SET_BUG_LOG("Failed to parse status report: ");
            SET_BUG_LOG(error.f_str());
            DEBUG_PRINT();
            return false;
        }

        // Get the JSON array of actuators
        JsonArray actuatorsArray = doc["actuators"].as<JsonArray>();
        if (actuatorsArray.isNull()) {
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
            SET_BUG_LOG("No actuators array found in the report.");
            DEBUG_PRINT();
            return false;
        }

        // Process each entry in the actuators array
        for (JsonObject actuator : actuatorsArray) {
            // Extract the "index" field and validate its range
            int idx = actuator["index"] | -1;
            if (idx < 0 || idx >= StatusReportData::MAX_ACTUATORS) {
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
                SET_BUG_LOG("Actuator index out of range or not present.");
                DEBUG_PRINT();
                continue; // Skip invalid or out-of-range actuator entries
            }

            ActuatorData &act = reportToParse.actuators[idx]; // Reference the specific actuator's data

            // DEBUG: Log the received index for clarity
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 2
            SET_BUG_LOG("Updating actuator at index: " + String(idx));
            DEBUG_PRINT();

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
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 2
            SET_BUG_LOG ("Actuator " + String(idx) + " updated.");
            DEBUG_PRINT();
            // Optionally, update the count of received actuators (if this is relevant)
            reportToParse.actuatorCount++;
        }

        return true;
    }

    String StatusReportProcessor::accumulateSerialInput(Stream &stream) {
        // Use appendStreamToBuffer to extract incoming data into the internal buffer
        String completeMessage = appendStreamToBuffer(stream);
        // Process and clean buffer, ensuring valid start marker detection or handle payload issues

            if (completeMessage == "") {
              // no message found in stream.
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
              SET_BUG_LOG ("No complete message found in stream.");
              DEBUG_PRINT();
              return "";
            } else {
                // Extract the complete message
                // duplicate action internalBuffer.remove(0, endPos + endMarker.length());
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 2
              SET_BUG_LOG  ("Found complete message in stream.  Contents:");
              SET_BUG_LOG (completeMessage);
              SET_BUG_LOG  ("End complete message.\n");
              DEBUG_PRINT();

                return completeMessage;  // Return the valid JSON message
            }

        // If no complete message, continue buffering
        return "";
    }

    void StatusReportProcessor::stripInvalidEntries(String &message) {
      // Check if the completeMessage contains valid data
     if (message.isEmpty() || message[0] != arrayStart || message[message.length() - 1] != arrayEnd) {
                  #undef CURRENT_LOG_LEVEL
                  #define CURRENT_LOG_LEVEL 1
         SET_BUG_LOG("[ERROR] No valid 'actuators' array found. Invalid JSON dumped:\n");
         SET_BUG_LOG(message + "\n");
         DEBUG_PRINT();
         message = ""; // Clear invalid message
         return;
     }
                  #undef CURRENT_LOG_LEVEL
                  #define CURRENT_LOG_LEVEL 3
        SET_BUG_LOG("[INFO] Valid 'actuators' array found. Processing...\n");
        SET_BUG_LOG(message + "\n");
        SET_BUG_LOG("End valid actuators message contents.\n");
        DEBUG_PRINT();
      // Split the array into individual entries (fields are delimited by "},{")
      String processedArray = "";
      int entryStart = 0;
      int entryEnd = message.indexOf("},{", entryStart); // Locate the delimiter

      while (entryEnd != -1) {
          // Extract the current entry (including the braces)
          String entry = message.substring(entryStart, entryEnd + 1);

          // Count the number of fields in the entry
          int fieldCount = 0;
          int fieldPosition = entry.indexOf(":");

          while (fieldPosition != -1) {
              fieldCount++;
              fieldPosition = entry.indexOf(":", fieldPosition + 1);
          }

          // If the field count matches, keep the entry; otherwise, log the invalid entry
          if (fieldCount == JSON_FIELD_COUNT) {
              if (!processedArray.isEmpty()) {
                  processedArray += ","; // Add a comma delimiter for valid entries
              }
              processedArray += entry;
          } else {
              // Log the invalid entry
                  #undef CURRENT_LOG_LEVEL
                  #define CURRENT_LOG_LEVEL 1
              SET_BUG_LOG("[ERROR] Invalid entry found (Field count mismatch). Dumping entry:\n");
              SET_BUG_LOG(entry + "\n");
              DEBUG_PRINT();
          }

          // Move to the next entry
          entryStart = entryEnd + 3; // Skip past "},{"
          entryEnd = message.indexOf("},{", entryStart);
      }

      // Process the last entry in the array (if any)
      String lastEntry = message.substring(entryStart);
        if (!lastEntry.isEmpty()) {
          // Count fields in the last entry
          int fieldCount = 0;
          int fieldPosition = lastEntry.indexOf(":");

          while (fieldPosition != -1) {
              fieldCount++;
              fieldPosition = lastEntry.indexOf(":", fieldPosition + 1);
          }

          // Keep or discard the last entry based on field count
          if (fieldCount == JSON_FIELD_COUNT) {
              if (!processedArray.isEmpty()) {
                  processedArray += ",";
              }
              processedArray += lastEntry;
          } else {
              // Log the invalid entry
                  #undef CURRENT_LOG_LEVEL
                  #define CURRENT_LOG_LEVEL 1
              SET_BUG_LOG("[ERROR] Invalid entry found (Field count mismatch). Dumping entry:\n");
              SET_BUG_LOG(lastEntry + "\n");
              DEBUG_PRINT();
          }
      }

      // Replace the actuators array in the original message with the processed array
      message = message.substring(0, arrayStart + 1) +
                        processedArray +
                        message.substring(arrayEnd);
  }


  bool StatusReportProcessor::purgeUntilStartMarker(String &buffer) {
    int markerPos = buffer.indexOf(startMarker);
    if (markerPos == -1) {
        // No start marker in the buffer, keep it empty
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
        SET_BUG_LOG ("[DEBUG] No start marker found in buffer.");
        buffer = "";
            DEBUG_PRINT();
        return false;
    }
    // call helper function to strip the hex header and store it in global payloadHeader.
    markerPos = stripEmHex (buffer, markerPos);
  // Purge everything before the start marker
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 3
    SET_BUG_LOG ("[DEBUG] Purging buffer up to startMarker position: ");
    SET_BUG_LOG (markerPos);
            DEBUG_PRINT();

    buffer.remove(0, markerPos);
    return true;
}

      // Helper function to strip the hex header and update the marker position
      int StatusReportProcessor::stripEmHex(String & buffer, int markerPos) {
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

    String StatusReportProcessor::appendStreamToBuffer(Stream &stream, StatusReportData &reportData) {
        // Read new data from the stream and append it to the internal buffer
        char c;
        static String internalBuffer;          // Declare the static buffer which will persist between calls
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 2
        SET_BUG_LOG(String("\n\n***Processing New Stream ***\n\nInternal buffer so far: ") + internalBuffer + "\n**End Internal Buffer**\n");
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 4
        SET_BUG_LOG(String("***Original Incoming Stream***\n"));
        while (stream.available()) {
            c = stream.read();  // Read one character
            internalBuffer += c;                   // Append to the buffer
#undef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL 4
            SET_BUG_LOG(String(c));
            if (payloadIsOversized(internalBuffer)) {
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
                SET_BUG_LOG("Internal buffer exceeded max allowed size. of " + String(MAX_PAYLOAD_SIZE) + "Clearing buffer to avoid overflow.");
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 4
                SET_BUG_LOG("\nBuffer contents:\n"+ internalBuffer + "\nEnd Buffer Contents\n");
                DEBUG_PRINT();
                internalBuffer = "";
                break;
            }
        }
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 4
        SET_BUG_LOG(String("\n***End Incoming Stream***\n"));
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 3
                    SET_BUG_LOG(String("*Internal Buffer After Processing*\n") + internalBuffer + "\n");
        SET_BUG_LOG(internalBuffer);
                    SET_BUG_LOG(String("\n*End Internal Buffer After Processing*\n"));
        DEBUG_PRINT();
        return processInternalBuffer();
    }

      String StatusReportProcessor::processInternalBuffer(StatusReportData &reportData) {
            String completeMessage = "["; // Start a JSON array
            bool isFirstEntry = true;     // Track if it's the first JSON message in the array

            while (internalBuffer.length() > 0) {
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 3
                SET_BUG_LOG("Processing internalBuffer...");
                SET_BUG_LOG(internalBuffer);
                DEBUG_PRINT();

                // Remove leading garbage or hex sequence, ensuring valid startMarker remains
                if (!purgeUntilStartMarker(internalBuffer)) {
                    // If no valid startMarker is found, clear the buffer and exit
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
                    SET_BUG_LOG("Start marker not found. Purging invalid data.");
                    DEBUG_PRINT();
                    internalBuffer = "";
                    return "";
                }

                // Locate the end marker
                int endPos = internalBuffer.indexOf(endMarker);
                if (endPos != -1) {
                    // Extract the message between the startMarker and endMarker
                    int startPos = internalBuffer.indexOf(startMarker);
                    if (startPos != -1 && startPos < endPos) {
                        // Extract message
                        String message = internalBuffer.substring(startPos, endPos + endMarker.length());
                        stripInvalidEntries(message);
                        // Append extracted message to the JSON array (add a comma for all but the first)
                        if (!isFirstEntry) {
                            completeMessage += ",";
                        }
                        completeMessage += message;
                        isFirstEntry = false; // Toggle flag after appending the first entry

                        // Remove processed data from the buffer
                        internalBuffer.remove(0, endPos + endMarker.length());
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 2
                        SET_BUG_LOG("Extracted valid message:");
                        DEBUG_PRINT();
                        SET_BUG_LOG(message);
                    } else {
                        // Invalid structure: no valid startMarker before endMarker, purging buffer
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
                        SET_BUG_LOG("Invalid message structure. Purging buffer.");
                        DEBUG_PRINT();
                        internalBuffer = "";
                        return "";
                    }
                } else {
                    // No endMarker found, stop processing (wait for more data to arrive)
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
                    SET_BUG_LOG("End marker not found. Waiting for more data.");
                    DEBUG_PRINT();
                    return "";
                }

                // Prevent buffer overflow by resetting if it grows too large
                if (internalBuffer.length() > MAX_PAYLOAD_SIZE) {
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
                    SET_BUG_LOG("Buffer exceeded max allowed size. Clearing buffer to avoid overflow.");
                    DEBUG_PRINT();
                    internalBuffer = "";
                    return "";
                }
            }

            // Close the JSON array if any messages were added
            completeMessage += "]";

            // If no messages were appended, return an empty array
            if (completeMessage == "[]") {
                #undef CURRENT_LOG_LEVEL
                #define CURRENT_LOG_LEVEL 1
                SET_BUG_LOG("No valid messages found in internalBuffer.");
                DEBUG_PRINT();
                completeMessage = "";
                return "";
            }
          return completeMessage;
        }


    inline bool StatusReportProcessor::payloadIsOversized(String &buffer) {
      return buffer.length() > MAX_PAYLOAD_SIZE;
    }

    // Extract the valid JSON portion from a raw payload that may include additional framing data.
    inline String StatusReportProcessor::processPayload(const char* payload, size_t length) {
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
