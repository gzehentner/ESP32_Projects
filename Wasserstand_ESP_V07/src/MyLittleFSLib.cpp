/**********************************************
 This file holds some simple functions to handle LittleFS
 I took this from Arduino libraries LittleFS examples
 ************************************************* */

// #ifndef MY_LITTLE_FS_LIB
// #define MY_LITTLE_FS_LIB
// #endif


#include <Arduino.h>
#include <LittleFS.h>

void readFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) { Serial.write(file.read()); }
  file.close();
}

void writeFile(const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void appendFile(const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (LittleFS.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (LittleFS.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

ssize_t getFileSize(const char *path)
{
  if (!LittleFS.begin())
  {
    // mount failed
    return -1;
  }
  if (!LittleFS.exists(path))
  {
    // file does not exist
    return -1;
  }
  File f = LittleFS.open(path, "r");
  if (!f)
    return -1;
  ssize_t sz = (ssize_t)f.size(); // bytes
  f.close();
  return sz;
}

void appendErrorFile(const char *message, String currentDate, String formattedTime)
{
  String errMessage = "";
  errMessage = currentDate;
  errMessage += " - ";
  errMessage += formattedTime;
  errMessage += " - ";
  errMessage += message;
  errMessage += "\n";
  appendFile("/error.log", errMessage.c_str());
}
//void listDir(const char *dirname) {
//  Serial.printf("Listing directory: %s\n", dirname);

//   Dir root = LittleFS.openDir(dirname);

//   while (root.next()) {
//     File file = root.openFile("r");
//     Serial.print("  FILE: ");
//     Serial.print(root.fileName());
//     Serial.print("  SIZE: ");
//     Serial.print(file.size());
//     time_t cr = file.getCreationTime();
//     time_t lw = file.getLastWrite();
//     file.close();
//     struct tm *tmstruct = localtime(&cr);
//     Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
//     tmstruct = localtime(&lw);
//     Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
//   }
// }
