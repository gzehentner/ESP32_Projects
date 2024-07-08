/**********************************************
 Header file for MyLittleFSLib.h
 ************************************************* */
#ifdef GZE_DEBUG
void readFile  (const char *path   );
void writeFile (const char *path,  const char *message);
void appendFile(const char *path,  const char *message);
void renameFile(const char *path1, const char *path2  );
void deleteFile(const char *path   );
void listDir   (const char *dirname);
#endif