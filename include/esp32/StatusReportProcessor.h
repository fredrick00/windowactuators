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


  class StatusReportProcessor {
  public:
    // Constructor: takes a reference to an input Stream (e.g., Serial2)
    StatusReportProcessor(Stream &inputStream) : inStream(inputStream), report () {}
    // Process incoming JSON data. If a complete report is read, fills in 'report'
    // and returns true; otherwise returns false.
    // Predeclarations
    const StatusReportData& getReport() const;
    static void printReport(const StatusReportData &report);
    bool process(Stream &dataStream);


  /**
    *   Begin private section of the class
    *
    *   **/
  private:
    Stream &inStream;
    StatusReportData report;
    // predeclarations
    String internalBuffer = "";
    String accumulateSerialInput(Stream &stream);
    String appendStreamToBuffer(Stream &stream, StatusReportData &reportData);
    void clearBuffer();
    String getBufferContents() const { return internalBuffer; }
    bool parseReport(const String &jsonReport, StatusReportData &reportToParse);
    bool payloadIsOversized(String &buffer);
    String processPayload(const char* payload, size_t length);
    String processInternalBuffer(StatusReportData &reportData);
    bool purgeUntilStartMarker(String &buffer);
    int stripEmHex(String & buffer, int markerPos);
    void stripInvalidEntries(String &message);

  };
} // namespace ActuatorsController