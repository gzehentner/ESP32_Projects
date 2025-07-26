// define function to print debug messages
#include <debugPrint.h>
#include <TelnetStream.h> // for Telnet output

//*******************************************************************************
void debugPrintln(String msg) {
  // if (debugLevel < globalDebugLevel) {
  //   return; // Do not print if debug level is lower than the set level
  // }
  Serial.println(msg);
  // if (enableTelnet) {
    TelnetStream.println(msg);
  // }
}
void debugPrint(String msg  ) {
  // if (debugLevel < globalDebugLevel) {
    // return; // Do not print if debug level is lower than the set level
  // }
  Serial.print(msg);
  // if (enableTelnet) {
    TelnetStream.print(msg);
  // }
}
//*******************************************************************************
//*******************************************************************************
void debugPrintln(int msg ) {
  // if (debugLevel < globalDebugLevel) {
    // return; // Do not print if debug level is lower than the set level
  // }
  Serial.println(msg);
  // if (enableTelnet) {
    TelnetStream.println(msg);
  // }
}
void debugPrint(int msg ) {
  // if (debugLevel < globalDebugLevel) {
    // return; // Do not print if debug level is lower than the set level
  // }
  Serial.print(String(msg));
  // if (enableTelnet) {
    TelnetStream.print(String(msg));
  // }
}
//*******************************************************************************
//*******************************************************************************
// void debugPrintln(String msg, int enableTelnet , int debugLevel ) {
//   if (debugLevel < globalDebugLevel) {
//     return; // Do not print if debug level is lower than the set level
//   }
//   Serial.println(msg);
//   if (enableTelnet) {
//     TelnetStream.println(msg);
//   }
// }
// void debugPrint(String msg, int enableTelnet,  int debugLevel ) {
//   if (debugLevel < globalDebugLevel) {
//     return; // Do not print if debug level is lower than the set level
//   }
//   Serial.print(msg);
//   if (enableTelnet) {
//     TelnetStream.print(msg);
//   }
// }
//*******************************************************************************
//*******************************************************************************
// void debugPrintln(int msg, int enableTelnet, int debugLevel) {
//   if (debugLevel < globalDebugLevel) {
//     return; // Do not print if debug level is lower than the set level
//   }
//   Serial.println(msg);
//   if (enableTelnet) {
//     TelnetStream.println(msg);
//   }
// }
// void debugPrint(int msg, int enableTelnet , int debugLevel ) {
//   if (debugLevel < globalDebugLevel) {
//     return; // Do not print if debug level is lower than the set level
//   }
//   Serial.print(String(msg));
//   if (enableTelnet) {
//     TelnetStream.print(String(msg));
//   }
// }

// Helper function to convert esp_reset_reason_t to a string
const char* espResetReasonToString(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_UNKNOWN:      return "Unknown";
    case ESP_RST_POWERON:      return "Power-on";
    case ESP_RST_EXT:          return "External reset";
    case ESP_RST_SW:           return "Software reset";
    case ESP_RST_PANIC:        return "Exception/panic";
    case ESP_RST_INT_WDT:      return "Interrupt watchdog";
    case ESP_RST_TASK_WDT:     return "Task watchdog";
    case ESP_RST_WDT:          return "Other watchdog";
    case ESP_RST_DEEPSLEEP:    return "Deep sleep";
    case ESP_RST_BROWNOUT:     return "Brownout";
    case ESP_RST_SDIO:         return "SDIO reset";
    default:                   return "Other";
  }
}