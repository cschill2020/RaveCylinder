#include "WLED.h"
#include <filesystem>
#include <fstream>

/*
 * Methods to handle saving and loading presets to/from the filesystem
 */

using std::filesystem::path;

namespace ravecylinder {

static volatile byte presetToApply = 0;
static volatile byte callModeToApply = 0;
static volatile byte presetToSave = 0;
static volatile int8_t saveLedmap = -1;
static char quickLoad[9];
static char saveName[33];
static bool includeBri = true, segBounds = true, selectedOnly = false,
            playlistSave = false;
;

path getFileName(bool persist = true) {
  return persist ? "presets.json" : "tmp.json";
}

static void doSaveState() {
  bool persist = (presetToSave < 251);
  path filename = getFileName(persist);

  // if (!requestJSONBufferLock(10)) return; // will set fileDoc

  initPresetsFile(); // just in case if someone deleted presets.json using /edit
  json sObj = json::object(); // = doc.to<JsonObject>();

  DEBUG_PRINTLN(F("Serialize current state"));
  if (playlistSave) {
    serializePlaylist(&sObj);
    if (includeBri)
      sObj["on"] = true;
  } else {
    serializeState(&sObj, true, includeBri, segBounds, selectedOnly);
  }
  sObj["n"] = saveName;
  if (quickLoad[0])
    sObj[F("ql")] = quickLoad;
  if (saveLedmap >= 0)
    sObj[F("ledmap")] = saveLedmap;
  /*
    #ifdef WLED_DEBUG
      DEBUG_PRINTLN(F("Serialized preset"));
      serializeJson(doc,Serial);
      DEBUG_PRINTLN();
    #endif
  */
  //   #if defined(ARDUINO_ARCH_ESP32)
  //   if (!persist) {
  //     if (tmpRAMbuffer!=nullptr) free(tmpRAMbuffer);
  //     size_t len = measureJson(*fileDoc) + 1;
  //     DEBUG_PRINTLN(len);
  //     // if possible use SPI RAM on ESP32
  //     #if defined(BOARD_HAS_PSRAM) && defined(WLED_USE_PSRAM)
  //     if (psramFound())
  //       tmpRAMbuffer = (char*) ps_malloc(len);
  //     else
  //     #endif
  //       tmpRAMbuffer = (char*) malloc(len);
  //     if (tmpRAMbuffer!=nullptr) {
  //       serializeJson(*fileDoc, tmpRAMbuffer, len);
  //     } else {
  //       writeObjectToFileUsingId(filename, presetToSave, fileDoc);
  //     }
  //   } else
  //   #endif
  writeObjectToFileUsingId(filename, presetToSave, sObj);

  // if (persist) presetsModifiedTime = toki.second(); //unix time
  // releaseJSONBufferLock();
  // updateFSInfo();

  // clean up
  saveLedmap = -1;
  presetToSave = 0;
  saveName[0] = '\0';
  quickLoad[0] = '\0';
  playlistSave = false;
}

// bool getPresetName(byte index, String& name)
// {
//   if (!requestJSONBufferLock(9)) return false;
//   bool presetExists = false;
//   if (readObjectFromFileUsingId(getFileName(), index, &doc))
//   {
//     JsonObject fdo = doc.as<JsonObject>();
//     if (fdo["n"]) {
//       name = (const char*)(fdo["n"]);
//       presetExists = true;
//     }
//   }
//   releaseJSONBufferLock();
//   return presetExists;
// }

void initPresetsFile() {
  if (std::filesystem::exists(getFileName()))
    return;

  //   StaticJsonDocument<64> doc;
  //   JsonObject sObj = doc.to<JsonObject>();
  json doc;
  json sObj = doc["0"];
  std::ofstream f(getFileName());
  // File f = WLED_FS.open(getFileName(), "w");
  if (!f) {
    errorFlag = ERR_FS_GENERAL;
    return;
  }
  // serializeJson(doc, f);
  f << doc;
  f.close();
}

bool applyPreset(byte index, byte callMode) {
  DEBUG_PRINT(F("Request to apply preset: "));
  DEBUG_PRINTLN(index);
  presetToApply = index;
  callModeToApply = callMode;
  return true;
}

// // apply preset or fallback to a effect and palette if it doesn't exist
// void applyPresetWithFallback(uint8_t index, uint8_t callMode, uint8_t
// effectID, uint8_t paletteID)
// {
//   applyPreset(index, callMode);
//   //these two will be overwritten if preset exists in handlePresets()
//   effectCurrent = effectID;
//   effectPalette = paletteID;
// }

void handlePresets() {
  if (presetToSave) {
    doSaveState();
    return;
  }
  if (presetToApply == 0 /*|| fileDoc*/)
    return; // no preset waiting to apply, or JSON buffer is already allocated,
            // return to loop until free

  bool changePreset = false;
  uint8_t tmpPreset = presetToApply; // store temporary since deserializeState()
                                     // may call applyPreset()
  uint8_t tmpMode = callModeToApply;

  json full;
  path filename = getFileName(tmpPreset < 255);

  // allocate buffer
  // if (!requestJSONBufferLock(9)) return;  // will also assign fileDoc

  presetToApply = 0; // clear request for preset
  callModeToApply = 0;

  // DEBUG_PRINT(F("Applying preset: "));
  // DEBUG_PRINTLN(tmpPreset);

  bool read = readObjectFromFile(filename, &full);
  char objKey[10];
  snprintf(objKey, 10, "%d", tmpPreset);
  json fdo;
  if (read && full.contains(objKey)) {
    fdo = full[objKey];
  }

  // HTTP API commands
  //  const char* httpwin = fdo["win"];
  //  if (httpwin) {
  //    String apireq = "win"; // reduce flash string usage
  //    apireq += F("&IN&"); // internal call
  //    apireq += httpwin;
  //    handleSet(nullptr, apireq, false); // may call applyPreset() via PL=
  //    setValuesFromFirstSelectedSeg(); // fills legacy values
  //    changePreset = true;
  //  } else {
  if (!fdo["seg"].is_null() || !fdo["on"].is_null() || !fdo["bri"].is_null() ||
      !fdo["nl"].is_null() || !fdo["ps"].is_null() ||
      !fdo[F("playlist")].is_null())
    changePreset = true;
  // if (!(tmpMode == CALL_MODE_BUTTON_PRESET && fdo["ps"].is<const char *>() &&
  // strchr(fdo["ps"].as<const char *>(),'~') != strrchr(fdo["ps"].as<const char
  // *>(),'~'))) fdo.erase("ps"); // remove load request for presets to prevent
  // recursive crash (if not called by button and contains preset cycling string
  // "1~5~")
  deserializeState(
      fdo, CALL_MODE_NO_NOTIFY,
      tmpPreset); // may change presetToApply by calling applyPreset()
                  //  }
  if (!errorFlag && tmpPreset < 255 && changePreset)
    currentPreset = tmpPreset;

  // releaseJSONBufferLock(); // will also clear fileDoc
  // if (changePreset) notify(tmpMode); // force UDP notification
  stateUpdated(tmpMode); // was colorUpdated() if anything breaks
  // updateInterfaces(tmpMode);
}
// TODO: This code may be needed in handlePresets
// }

// called from handleSet(PS=) [network callback (fileDoc==nullptr), IR
// (irrational), deserializeState, UDP] and deserializeState() [network callback
// (filedoc!=nullptr)]
void savePreset(byte index, const char *pname, json &sObj) {
  if (index == 0 || (index > 250 && index < 255))
    return;
  if (pname)
    strlcpy(saveName, pname, 33);
  else {
    if (sObj["n"].is_string())
      strlcpy(saveName, std::string(sObj["n"]).c_str(), 33);
    else
      snprintf(saveName, 33, PSTR("Preset %d"), index);
  }

  // DEBUG_PRINT(F("Saving preset (")); DEBUG_PRINT(index); DEBUG_PRINT(F(")
  // ")); DEBUG_PRINTLN(saveName);

  presetToSave = index;
  playlistSave = false;
  if (sObj[F("ql")].is_string())
    strlcpy(
        quickLoad, std::string(sObj[F("ql")]).c_str(),
        9); // client limits QL to 2 chars, buffer for 8 bytes to allow unicode

  if (sObj.size() == 0 ||
      sObj["o"].is_null()) { // no "o" means not a playlis or custom API call,
                             // saving of state is async(not immediately)
    includeBri = bool(sObj["ib"]) || sObj.size() == 0 ||
                 index == 255; // temporary preset needs brightness
    segBounds = bool(sObj["sb"]) || sObj.size() == 0 ||
                index == 255; // temporary preset needs bounds
    selectedOnly = bool(sObj[F("sc")]);
    saveLedmap = sObj.value(F("ledmap"), -1);
  } else {
    // this is a playlist or API call
    if (sObj[F("playlist")].is_null()) {
      // we will save API call immediately (often causes presets.json
      // corruption)
      presetToSave = 0;
      if (index > 250 /*|| !fileDoc*/)
        return; // cannot save API calls to temporary preset(255)
                // sObj.remove("o");
      sObj.erase("v");
      sObj.erase("time");
      sObj.erase(F("error"));
      sObj.erase(F("psave"));
      if (sObj["n"].is_null())
        sObj["n"] = saveName;
      initPresetsFile(); // just in case if someone deleted presets.json using
                         // /edit
      writeObjectToFileUsingId(getFileName(index < 255), index, sObj);
      presetsModifiedTime =
          std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count(); // unix time
      // updateFSInfo();
    } else {
      // store playlist
      // WARNING: playlist will be loaded in json.cpp after this call and will
      // have repeat counter increased by 1
      includeBri = true; // !sObj["on"].isNull();
      playlistSave = true;
    }
  }
}

void deletePreset(byte index) {
  json empty;
  writeObjectToFileUsingId(getFileName(), index, empty);
  // presetsModifiedTime = toki.second(); //unix time
  // updateFSInfo();
}
} // namespace ravecylinder