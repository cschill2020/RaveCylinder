#include "WLED.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace ravecylinder {
bool writeJsonToFile(const fs::path& file, json& content) {
          std::fstream f;
    f.open(file, std::ios::out);
    f << content;
    f.close();
    return true;
}

bool writeObjectToFileUsingId(const fs::path& file, uint16_t id,
                              json& content) {
  char objKey[10];
  snprintf(objKey, 10, "%d", id);
  return writeObjectToFile(file, objKey, content);
}

bool writeObjectToFile(const fs::path& file, const char* key, json& content)
{
    std::fstream f;
    json jf;
    if (fs::exists(file)) {
        f.open(file, std::ios::in);
        jf = json::parse(f);
        f.close();
    }    
    
    std::cout<<"json read file: "<<file<<" : "<<jf<<std::endl;
    if (jf.contains(key)) {
        jf.erase(key);
    } 
    if (!content.empty()) {
      jf[key] = content;
    }
    f.open(file, std::ios::out);
    f << jf;
    f.close();
    return true;
//     std::ifstream ifs(file);
//     json jf = json::parse(ifs);
//     ifs.close();
// //   uint32_t s = 0; //timing
// //   #ifdef WLED_DEBUG_FS
// //     DEBUGFS_PRINTF("Write to %s with key %s >>>\n", file, (key==nullptr)?"nullptr":key);
// //     serializeJson(*content, Serial); DEBUGFS_PRINTLN();
// //     s = millis();
// //   #endif

//   size_t pos = 0;
//   f = WLED_FS.open(file, "r+");
//   if (!f && !WLED_FS.exists(file)) f = WLED_FS.open(file, "w+");
//   if (!f) {
//     DEBUGFS_PRINTLN(F("Failed to open!"));
//     return false;
//   }

//   if (!bufferedFind(key)) //key does not exist in file
//   {
//     return appendObjectToFile(key, content, s);
//   }

//   //an object with this key already exists, replace or delete it
//   pos = f.position();
//   //measure out end of old object
//   bufferedFindObjectEnd();
//   size_t pos2 = f.position();

//   uint32_t oldLen = pos2 - pos;
//   DEBUGFS_PRINTF("Old obj len %d\n", oldLen);

//   //Three cases:
//   //1. The new content is null, overwrite old obj with spaces
//   //2. The new content is smaller than the old, overwrite and fill diff with spaces
//   //3. The new content is larger than the old, but smaller than old + trailing spaces, overwrite with new
//   //4. The new content is larger than old + trailing spaces, delete old and append

//   size_t contentLen = 0;
//   if (!content->isNull()) contentLen = measureJson(*content);

//   if (contentLen && contentLen <= oldLen) { //replace and fill diff with spaces
//     DEBUGFS_PRINTLN(F("replace"));
//     f.seek(pos);
//     serializeJson(*content, f);
//     writeSpace(pos2 - f.position());
//   } else if (contentLen && bufferedFindSpace(contentLen - oldLen, false)) { //enough leading spaces to replace
//     DEBUGFS_PRINTLN(F("replace (trailing)"));
//     f.seek(pos);
//     serializeJson(*content, f);
//   } else {
//     DEBUGFS_PRINTLN(F("delete"));
//     pos -= strlen(key);
//     if (pos > 3) pos--; //also delete leading comma if not first object
//     f.seek(pos);
//     writeSpace(pos2 - pos);
//     if (contentLen) return appendObjectToFile(key, content, s, contentLen);
//   }

//   doCloseFile = true;
//   DEBUGFS_PRINTF("Replaced/deleted, took %d ms\n", millis() - s);
//  return true;
}

// bool readObjectFromFileUsingId(const char* file, uint16_t id, json* dest)
// {
//   char objKey[10];
//   sprintf(objKey, "\"%d\":", id);
//   return readObjectFromFile(file, objKey, dest);
// }

// //if the key is a nullptr, deserialize entire object
// bool readObjectFromFile(const char *file, const char *key, json *dest) {
//   f = WLED_FS.open(file, "r");
//   if (!f)
//     return false;

//   if (key != nullptr && !bufferedFind(key)) // key does not exist in file
//   {
//     f.close();
//     dest->clear();
//     DEBUGFS_PRINTLN(F("Obj not found."));
//     return false;
//   }
//   deserializeJson(*dest, f);

//   f.close();
//   DEBUGFS_PRINTF("Read, took %d ms\n", millis() - s);
//   return true;
// }

bool readObjectFromFile(const fs::path& file, json *dest) {
  std::fstream f;
  if (fs::exists(file)) {
    f.open(file, std::ios::in);
    *dest = json::parse(f);
    f.close();
    return true;
  }

  f.close();
  return false;
}
} // namespace ravecylinder