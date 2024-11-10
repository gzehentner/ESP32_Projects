// ===================================================
//
// Utilities to capture photo
//
// ===================================================

#ifndef CapturePhoto_h_
#define CapturePhoto_h_ 

#include <FS.h>

bool checkPhoto( fs::FS &fs );
void capturePhotoSaveSpiffs( void );
void capturePhotoPost( void );
void postImageFile();
void postImageOnly();



#endif