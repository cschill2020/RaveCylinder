#include "WLED.h"
#include "WLED_FX.h"
#include "WLED_fcn_declare.h"
#include <httpserver.hpp>

namespace ravecylinder {
json serializeModeNames() {
  char lineBuffer[256];
  json response = json::array();
  for (size_t i = 0; i < strip().getModeCount(); i++) {
    strncpy(lineBuffer, strip().getModeData(i),
            sizeof(lineBuffer) / sizeof(char) - 1);
    lineBuffer[sizeof(lineBuffer) / sizeof(char) - 1] =
        '\0'; // terminate string
    if (lineBuffer[0] != 0) {
      char *dataPtr = strchr(lineBuffer, '@');
      if (dataPtr)
        *dataPtr = 0; // terminate mode data after name
      response.insert(response.end(), lineBuffer);
    }
  }
  return response;
}

json serializeModeData() {
  json response = json::array();
  char lineBuffer[256];
  for (size_t i = 0; i < strip().getModeCount(); i++) {
    strncpy(lineBuffer, strip().getModeData(i),
            sizeof(lineBuffer) / sizeof(char) - 1);
    lineBuffer[sizeof(lineBuffer) / sizeof(char) - 1] =
        '\0'; // terminate string
    if (lineBuffer[0] != 0) {
      char *dataPtr = strchr(lineBuffer, '@');
      if (dataPtr) {
        response.insert(response.end(), dataPtr + 1);
      } else
        response.insert(response.end(), "");
    }
  }
  return response;
}

void serializeSegment(json *root, Segment &seg, byte id, bool forPreset,
                      bool segmentBounds) {
  (*root)["id"] = id;
  if (segmentBounds) {
    (*root)["start"] = seg.start;
    (*root)["stop"] = seg.stop;
#ifndef WLED_DISABLE_2D
    if (strip().isMatrix) {
      (*root)[F("startY")] = seg.startY;
      (*root)[F("stopY")] = seg.stopY;
    }
#endif
  }
  if (!forPreset)
    (*root)["len"] = seg.stop - seg.start;
  (*root)["grp"] = seg.grouping;
  (*root)[F("spc")] = seg.spacing;
  (*root)[F("of")] = seg.offset;
  (*root)["on"] = bool(seg.on);
  (*root)["frz"] = bool(seg.freeze);
  byte segbri = seg.opacity;
  (*root)["bri"] = (segbri) ? segbri : 255;
  (*root)["cct"] = seg.cct;
  (*root)[F("set")] = bool(seg.set);

  if (seg.name != nullptr)
    (*root)["n"] = reinterpret_cast<const char *>(
        seg.name); // not good practice, but decreases required JSON buffer
  else if (forPreset)
    (*root)["n"] = "";

  // to conserve RAM we will serialize the col array manually
  // this will reduce RAM footprint from ~300 bytes to 84 bytes per segment
  char colstr[70];
  colstr[0] = '[';
  colstr[1] = '\0'; // max len 68 (5 chan, all 255)
  const char *format =
      strip().hasWhiteChannel() ? PSTR("[%u,%u,%u,%u]") : PSTR("[%u,%u,%u]");
  for (size_t i = 0; i < 3; i++) {
    byte segcol[4];
    byte *c = segcol;
    segcol[0] = R(seg.colors[i]);
    segcol[1] = G(seg.colors[i]);
    segcol[2] = B(seg.colors[i]);
    segcol[3] = W(seg.colors[i]);
    char tmpcol[22];
    sprintf_P(tmpcol, format, (unsigned)c[0], (unsigned)c[1], (unsigned)c[2],
              (unsigned)c[3]);
    strcat(colstr, i < 2 ? strcat(tmpcol, ",") : tmpcol);
  }
  strcat(colstr, "]");
  (*root)["col"] = json::parse(colstr);

  (*root)["fx"] = seg.mode;
  (*root)["sx"] = seg.speed;
  (*root)["ix"] = seg.intensity;
  (*root)["pal"] = seg.palette;
  (*root)["c1"] = seg.custom1;
  (*root)["c2"] = seg.custom2;
  (*root)["c3"] = uint8_t(seg.custom3);
  (*root)["sel"] = seg.isSelected();
  (*root)["rev"] = bool(seg.reverse);
  (*root)["mi"] = bool(seg.mirror);
#ifndef WLED_DISABLE_2D
  if (strip().isMatrix) {
    (*root)["rY"] = bool(seg.reverse_y);
    (*root)["mY"] = bool(seg.mirror_y);
    (*root)[F("tp")] = bool(seg.transpose);
  }
#endif
  (*root)["o1"] = bool(seg.check1);
  (*root)["o2"] = bool(seg.check2);
  (*root)["o3"] = bool(seg.check3);
  (*root)["si"] = uint8_t(seg.soundSim);
  (*root)["m12"] = uint8_t(seg.map1D2D);
}

void serializeState(json *root, bool forPreset = false, bool includeBri = true,
                    bool segmentBounds = true,
                    bool selectedSegmentsOnly = false) {
  if (includeBri) {
    (*root)["on"] = (bri > 0);
    (*root)["bri"] = briLast;
    (*root)[F("transition")] = transitionDelay / 100; // in 100ms
  }

  if (!forPreset) {
    if (errorFlag) {
      (*root)[F("error")] = errorFlag;
      errorFlag = ERR_NONE;
    } // prevent error message to persist on screen

    (*root)["ps"] = (currentPreset > 0) ? currentPreset : -1;
    (*root)[F("pl")] = currentPlaylist;

    // usermods.addToJsonState((*root));

    json &nl = (*root)["nl"];
    nl["on"] = nightlightActive;
    nl["dur"] = nightlightDelayMins;
    nl["mode"] = nightlightMode;
    nl[F("tbri")] = nightlightTargetBri;
    if (nightlightActive) {
      nl[F("rem")] = (nightlightDelayMs - (millis() - nightlightStartTime)) /
                     1000; // seconds remaining
    } else {
      nl[F("rem")] = -1;
    }

    json &udpn = (*root)["udpn"];
    udpn["send"] = notifyDirect;
    // udpn["recv"] = receiveNotifications;
    // udpn["sgrp"] = syncGroups;
    // udpn["rgrp"] = receiveGroups;

    // (*root)[F("lor")] = realtimeOverride;
  }

  (*root)[F("mainseg")] = strip().getMainSegmentId();

  json &seg = (*root)["seg"];
  seg = json::array();
  for (size_t s = 0; s < strip().getMaxSegments(); s++) {
    if (s >= strip().getSegmentsNum()) {
      if (forPreset && segmentBounds &&
          !selectedSegmentsOnly) { // disable segments not part of preset
        json seg0 = json::object();
        seg0["stop"] = 0;
        seg.push_back(seg0);
        continue;
      } else
        break;
    }
    Segment &sg = strip().getSegment(s);
    if (forPreset && selectedSegmentsOnly && !sg.isSelected())
      continue;
    if (sg.isActive()) {
      json seg0 = json::object();
      serializeSegment(&seg0, sg, s, forPreset, segmentBounds);
      seg.push_back(seg0);
    } else if (forPreset &&
               segmentBounds) { // disable segments not part of preset
      json seg0 = json::object();
      seg0["stop"] = 0;
      seg.push_back(seg0);
    }
  }
}

// helper to get int value with in/decrementing support via ~ syntax
void parseNumber(const char *str, byte *val, byte minv, byte maxv) {
  if (str == nullptr || str[0] == '\0')
    return;
  if (str[0] == 'r') {
    *val = random8(minv, maxv ? maxv : 255);
    return;
  } // maxv for random cannot be 0
  bool wrap = false;
  if (str[0] == 'w' && strlen(str) > 1) {
    str++;
    wrap = true;
  }
  if (str[0] == '~') {
    int out = atoi(str + 1);
    if (out == 0) {
      if (str[1] == '0')
        return;
      if (str[1] == '-') {
        *val = (int)(*val - 1) < (int)minv
                   ? maxv
                   : std::min((int)maxv, (*val - 1)); //-1, wrap around
      } else {
        *val = (int)(*val + 1) > (int)maxv
                   ? minv
                   : std::max((int)minv, (*val + 1)); //+1, wrap around
      }
    } else {
      if (wrap && *val == maxv && out > 0)
        out = minv;
      else if (wrap && *val == minv && out < 0)
        out = maxv;
      else {
        out += *val;
        if (out > maxv)
          out = maxv;
        if (out < minv)
          out = minv;
      }
      *val = out;
    }
    return;
  } else if (minv == maxv && minv == 0) { // limits "unset" i.e. both 0
    byte p1 = atoi(str);
    const char *str2 =
        strchr(str, '~'); // min/max range (for preset cycle, e.g. "1~5~")
    if (str2) {
      byte p2 = atoi(++str2); // skip ~
      if (p2 > 0) {
        while (isdigit(*(++str2)))
          ; // skip digits
        parseNumber(str2, val, p1, p2);
        return;
      }
    }
  }
  *val = atoi(str);
}

bool getVal(json &elem, byte *val, byte vmin = 0, byte vmax = 255) {
  if (elem.is_number_integer()) {
    if (elem < 0)
      return false; // ignore e.g. {"ps":-1}

    *val = elem;
    return true;
  } else if (elem.is_string()) {
    std::string str_elem = elem;
    const char *str = str_elem.c_str();
    size_t len = strnlen(str, 12);
    if (len == 0 || len > 10)
      return false;
    parseNumber(str, val, vmin, vmax);
    return true;
  }
  return false; // key does not exist
}

bool deserializeSegment(json &elem, byte it, byte presetId) {
  byte id = elem.value("id", it);
  if (id >= strip().getMaxSegments())
    return false;

  bool newSeg = false;
  int stop = elem.value("stop", -1);

  // append segment
  if (id >= strip().getSegmentsNum()) {
    if (stop <= 0)
      return false; // ignore empty/inactive segments
    strip().appendSegment(Segment(0, strip().getLengthTotal()));
    id = strip().getSegmentsNum() - 1; // segments are added at the end of list
    newSeg = true;
  }

  // DEBUG_PRINTLN("-- JSON deserialize segment.");
  Segment &seg = strip().getSegment(id);
  // DEBUG_PRINTF("--  Original segment: %p\n", &seg);
  Segment prev = seg; // make a backup so we can tell if something changed
  // DEBUG_PRINTF("--  Duplicate segment: %p\n", &prev);

  uint16_t start = elem.value("start", seg.start);
  if (stop < 0) {
    if (elem.contains("len")) {
      int len = elem["len"];
      stop = (len > 0) ? start + len : seg.stop;
    }
  }
  // 2D segments
  uint16_t startY = elem.value("startY", seg.startY);
  uint16_t stopY = elem.value("stopY", seg.stopY);

  // repeat, multiplies segment until all LEDs are used, or max segments reached
  bool repeat = elem.value("rpt", false);
  if (repeat && stop > 0) {
    elem.erase("id");  // remove for recursive call
    elem.erase("rpt"); // remove for recursive call
    elem.erase("n");   // remove for recursive call
    uint16_t len = stop - start;
    for (size_t i = id + 1; i < strip().getMaxSegments(); i++) {
      start = start + len;
      if (start >= strip().getLengthTotal())
        break;
      // TODO: add support for 2D
      elem["start"] = start;
      elem["stop"] = start + len;
      elem["rev"] = !elem["rev"]; // alternate reverse on even/odd segments
      deserializeSegment(elem, i, presetId); // recursive call with new id
    }
    return true;
  }

  if (elem.contains("n")) {
    // name field exists
    if (seg.name) { // clear old name
      delete[] seg.name;
      seg.name = nullptr;
    }

    const std::string name_str = elem["n"].template get<std::string>();
    const char *name = name_str.c_str();
    size_t len = 0;
    if (name != nullptr)
      len = strlen(name);
    if (len > 0) {
      if (len > WLED_MAX_SEGNAME_LEN)
        len = WLED_MAX_SEGNAME_LEN;
      seg.name = new char[len + 1];
      if (seg.name)
        strlcpy(seg.name, name, WLED_MAX_SEGNAME_LEN + 1);
    } else {
      // but is empty (already deleted above)
      elem.erase("n");
    }
  } else if (start != seg.start || stop != seg.stop) {
    // clearing or setting segment without name field
    if (seg.name) {
      delete[] seg.name;
      seg.name = nullptr;
    }
  }

  uint16_t grp = elem.value("grp", seg.grouping);
  uint16_t spc = elem.value("spc", seg.spacing);
  uint16_t of = seg.offset;
  uint8_t soundSim = elem.value("si", uint8_t(seg.soundSim));
  uint8_t map1D2D = elem.value("m12", uint8_t(seg.map1D2D));

  if ((spc > 0 && spc != seg.spacing) || seg.map1D2D != map1D2D)
    seg.fill(BLACK); // clear spacing gaps

  seg.map1D2D = constrain(map1D2D, 0, 7);
  seg.soundSim = constrain(soundSim, 0, 1);

  uint8_t set = elem.value("set", uint8_t(seg.set));
  seg.set = constrain(set, 0, 3);

  uint16_t len = 1;
  if (stop > start)
    len = stop - start;
  int offset = elem.value("of", INT32_MAX);
  if (offset != INT32_MAX) {
    int offsetAbs = abs(offset);
    if (offsetAbs > len - 1)
      offsetAbs %= len;
    if (offset < 0)
      offsetAbs = len - offsetAbs;
    of = offsetAbs;
  }
  if (stop > start && of > len - 1)
    of = len - 1;

  // update segment (delete if necessary)
  // do not call seg.setUp() here, as it may cause a crash due to concurrent
  // access if the segment is currently drawing effects WS2812FX handles
  // queueing of the change
  strip().setSegment(id, start, stop, grp, spc, of, startY, stopY);
  if (newSeg)
    seg.refreshLightCapabilities(); // fix for #3403

  if (seg.reset && seg.stop == 0) {
    if (id == strip().getMainSegmentId())
      strip().setMainSegmentId(0); // fix for #3403
    return true; // segment was deleted & is marked for reset, no need to change
                 // anything else
  }

  byte segbri = seg.opacity;
  if (getVal(elem["bri"], &segbri)) {
    if (segbri > 0)
      seg.setOpacity(segbri);
    seg.setOption(SEG_OPTION_ON, segbri); // use transition
  }

  bool on = elem.value("on", bool(seg.on));
  // if (elem["on"].is<const char*>() && elem["on"].as<const char*>()[0] == 't')
  // on = !on;
  seg.setOption(SEG_OPTION_ON, on); // use transition
  bool frz = elem.value("frz", bool(seg.freeze));
  // if (elem["frz"].is<const char*>() && elem["frz"].as<const char*>()[0] ==
  // 't') frz = !seg.freeze;
  seg.freeze = frz;

  seg.setCCT(elem.value("cct", seg.cct));

  json &colarr = elem["col"];
  if (!colarr.is_null()) {
    if (seg.getLightCapabilities() & 3) {
      // segment has RGB or White
      for (size_t i = 0; i < 3; i++) {
        int rgbw[] = {0, 0, 0, 0};
        bool colValid = false;
        json &colX = colarr[i];
        if (colX.is_null()) {
          byte brgbw[] = {0, 0, 0, 0};
          std::string hexCol_str = colarr[i].template get<std::string>();
          const char *hexCol = hexCol_str.c_str();
          if (hexCol ==
              nullptr) { // Kelvin color temperature (or invalid), e.g 2400
            int kelvin =
                colarr[i].is_null() ? -1 : colarr[i].template get<int>();
            if (kelvin < 0)
              continue;
            if (kelvin == 0)
              seg.setColor(i, 0);
            if (kelvin > 0)
              colorKtoRGB(kelvin, brgbw);
            colValid = true;
          } else { // HEX string, e.g. "FFAA00"
            colValid = colorFromHexString(brgbw, hexCol);
          }
          for (size_t c = 0; c < 4; c++)
            rgbw[c] = brgbw[c];
        } else { // Array of ints (RGB or RGBW color), e.g. [255,160,0]
          byte sz = colX.size();
          if (sz == 0)
            continue; // do nothing on empty array
          int cntr = 0;
          for (auto &elem : colX) {
            rgbw[cntr] = elem.template get<uint8_t>();
            cntr++;
          }
          colValid = true;
        }

        if (!colValid)
          continue;

        seg.setColor(i, RGBW32(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
        if (seg.mode == FX_MODE_STATIC)
          strip().trigger(); // instant refresh
      }
    } else {
      // non RGB & non White segment (usually On/Off bus)
      seg.setColor(0, ULTRAWHITE);
      seg.setColor(1, BLACK);
    }
  }

// lx parser
#ifdef WLED_ENABLE_LOXONE
  int lx = elem[F("lx")] | -1;
  if (lx > 0) {
    parseLxJson(lx, id, false);
  }
  int ly = elem[F("ly")] | -1;
  if (ly > 0) {
    parseLxJson(ly, id, true);
  }
#endif

#ifndef WLED_DISABLE_2D
  bool reverse = seg.reverse;
  bool mirror = seg.mirror;
#endif
  seg.selected = elem.value("sel", bool(seg.selected));
  seg.reverse = elem.value("rev", bool(seg.reverse));
  seg.mirror = elem.value("mi", bool(seg.mirror));
#ifndef WLED_DISABLE_2D
  bool reverse_y = seg.reverse_y;
  bool mirror_y = seg.mirror_y;
  seg.reverse_y = elem.value("rY", bool(seg.reverse_y));
  seg.mirror_y = elem.value("mY", bool(seg.mirror_y));
  seg.transpose = elem.value("tp", bool(seg.transpose));
  if (seg.is2D() && seg.map1D2D == M12_pArc &&
      (reverse != seg.reverse || reverse_y != seg.reverse_y ||
       mirror != seg.mirror || mirror_y != seg.mirror_y))
    seg.fill(BLACK); // clear entire segment (in case of Arc 1D to 2D expansion)
#endif

  byte fx = seg.mode;
  if (getVal(elem["fx"], &fx, 0,
             strip().getModeCount())) { // load effect ('r' random, '~' inc/dec,
                                        // 0-255 exact value)
    // if (!presetId && currentPlaylist>=0) unloadPlaylist();
    if (fx != seg.mode) {
      seg.setMode(fx, elem.value("fxdef", false));
    }
  }

  // getVal also supports inc/decrementing and random
  getVal(elem["sx"], &seg.speed);
  getVal(elem["ix"], &seg.intensity);

  uint8_t pal = seg.palette;
  if (seg.getLightCapabilities() &
      1) { // ignore palette for White and On/Off segments
    if (getVal(elem["pal"], &pal))
      seg.setPalette(pal);
  }

  getVal(elem["c1"], &seg.custom1);
  getVal(elem["c2"], &seg.custom2);
  uint8_t cust3 = seg.custom3;
  getVal(elem["c3"], &cust3); // we can't pass reference to bitfield
  seg.custom3 = constrain(cust3, 0, 31);

  seg.check1 = elem.value("o1", bool(seg.check1));
  seg.check2 = elem.value("o2", bool(seg.check2));
  seg.check3 = elem.value("o3", bool(seg.check3));

  // json& iarr = elem[F("i")]; //set individual LEDs
  // if (!iarr.isNull()) {
  //   uint8_t oldMap1D2D = seg.map1D2D;
  //   seg.map1D2D = M12_Pixels; // no mapping

  //   // set brightness immediately and disable transition
  //   jsonTransitionOnce = true;
  //   seg.stopTransition();
  //   strip.setTransition(0);
  //   strip.setBrightness(scaledBri(bri), true);

  //   // freeze and init to black
  //   if (!seg.freeze) {
  //     seg.freeze = true;
  //     seg.fill(BLACK);
  //   }

  //   uint16_t start = 0, stop = 0;
  //   byte set = 0; //0 nothing set, 1 start set, 2 range set

  //   for (size_t i = 0; i < iarr.size(); i++) {
  //     if(iarr[i].is<JsonInteger>()) {
  //       if (!set) {
  //         start = abs(iarr[i].as<int>());
  //         set++;
  //       } else {
  //         stop = abs(iarr[i].as<int>());
  //         set++;
  //       }
  //     } else { //color
  //       uint8_t rgbw[] = {0,0,0,0};
  //       JsonArray icol = iarr[i];
  //       if (!icol.isNull()) { //array, e.g. [255,0,0]
  //         byte sz = icol.size();
  //         if (sz > 0 && sz < 5) copyArray(icol, rgbw);
  //       } else { //hex string, e.g. "FF0000"
  //         byte brgbw[] = {0,0,0,0};
  //         const char* hexCol = iarr[i];
  //         if (colorFromHexString(brgbw, hexCol)) {
  //           for (size_t c = 0; c < 4; c++) rgbw[c] = brgbw[c];
  //         }
  //       }

  //       if (set < 2 || stop <= start) stop = start + 1;
  //       uint32_t c = gamma32(RGBW32(rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
  //       while (start < stop) seg.setPixelColor(start++, c);
  //       set = 0;
  //     }
  //   }
  //  //seg.map1D2D = oldMap1D2D; // restore mapping
  //  strip().trigger(); // force segment update
  // }
  // send UDP/WS if segment options changed (except selection; will also
  // deselect current preset)
  if (seg.differs(prev) & 0x7F)
    stateChanged = true;
  return true;
}

// deserializes WLED state (fileDoc points to doc object if called from web
// server) presetId is non-0 if called from handlePreset()
bool deserializeState(json &root, byte callMode = CALL_MODE_DIRECT_CHANGE,
                      byte presetId = 0) {
  bool stateResponse = root.value("v", false);

  // #if defined(WLED_DEBUG) && defined(WLED_DEBUG_HOST)
  // netDebugEnabled = root[F("debug")] | netDebugEnabled;
  // #endif

  bool onBefore = bri;
  getVal(root["bri"], &bri);

  // bool on = root["on"] | (bri > 0);
  // if (!on != !bri) toggleOnOff();

  // if (root["on"].is<const char*>() && root["on"].as<const char*>()[0] == 't')
  // {
  //   if (onBefore || !bri) toggleOnOff(); // do not toggle off again if just
  //   turned on by bri (makes e.g. "{"on":"t","bri":32}" work)
  // }

  // if (bri && !onBefore) { // unfreeze all segments when turning on
  //   for (size_t s=0; s < strip.getSegmentsNum(); s++) {
  //     strip.getSegment(s).freeze = false;
  //   }
  //   if (realtimeMode && !realtimeOverride && useMainSegmentOnly) { // keep
  //   live segment frozen if live
  //     strip.getMainSegment().freeze = true;
  //   }
  // }

  int tr = -1;
  // if (!presetId ||
  //     currentPlaylist <
  //         0) { // do not apply transition time from preset if playlist
  //         active,
  //              // as it would override playlist transition times
  //   tr = int(root[F("transition")]) | -1;
  //   if (tr >= 0) {
  //     transitionDelay = tr * 100;
  //     if (fadeTransition)
  //       strip().setTransition(transitionDelay);
  //   }
  // }

  // temporary transition (applies only once)
  tr = root.value("tt", -1);
  if (tr >= 0) {
    jsonTransitionOnce = true;
    if (fadeTransition)
      strip().setTransition(tr * 100);
  }

  tr = root.value("tb", -1);
  if (tr >= 0)
    strip().timebase = tr - millis();

  json &nl = root["nl"];
  if (!nl.is_null()) {
    nightlightActive = nl.value("on", nightlightActive);
    nightlightDelayMins = nl.value("dur", nightlightDelayMins);
    nightlightMode = nl.value("mode", nightlightMode);
    nightlightTargetBri = nl.value("tbri", nightlightTargetBri);
  }

  json &udpn = root["udpn"];
  if (!udpn.is_null()) {
    notifyDirect = udpn.value("send", notifyDirect);
    // syncGroups           = udpn["sgrp"] | syncGroups;
    // receiveNotifications = udpn["recv"] | receiveNotifications;
    // receiveGroups        = udpn["rgrp"] | receiveGroups;
    // if ((bool)udpn[F("nn")]) callMode = CALL_MODE_NO_NOTIFY; //send no
    // notification just for this request
  }

  // unsigned long timein = root.value("time", UINT32_MAX); // backup time
  // source if NTP not synced if (timein != UINT32_MAX) {
  //   setTimeFromAPI(timein);
  //   if (presetsModifiedTime == 0) presetsModifiedTime = timein;
  // }

  // if (root[F("psave")].isNull()) doReboot = root[F("rb")] | doReboot;

  // do not allow changing main segment while in realtime mode (may get odd
  // results else)
  /*if (!realtimeMode)*/ strip().setMainSegmentId(root.value(
      "mainseg",
      strip().getMainSegmentId())); // must be before realtimeLock() if "live"

  // realtimeOverride = root[F("lor")] | realtimeOverride;
  // if (realtimeOverride > 2) realtimeOverride = REALTIME_OVERRIDE_ALWAYS;
  // if (realtimeMode && useMainSegmentOnly) {
  //   strip.getMainSegment().freeze = !realtimeOverride;
  // }

  // if (root.containsKey("live")) {
  //   if (root["live"].as<bool>()) {
  //     jsonTransitionOnce = true;
  //     strip.setTransition(0);
  //     realtimeLock(65000);
  //   } else {
  //     exitRealtime();
  //   }
  // }

  int it = 0;
  json &segVar = root["seg"];
  if (segVar.is_object()) {
    int id = segVar.value("id", -1);
    // if "seg" is not an array and ID not specified, apply to all
    // selected/checked segments
    if (id < 0) {
      // apply all selected segments
      // bool didSet = false;
      for (size_t s = 0; s < strip().getSegmentsNum(); s++) {
        Segment &sg = strip().getSegment(s);
        if (sg.isSelected()) {
          deserializeSegment(segVar, s, presetId);
          // didSet = true;
        }
      }
      // TODO: not sure if it is good idea to change first active but unselected
      // segment if (!didSet) deserializeSegment(segVar,
      // strip.getMainSegmentId(), presetId);
    } else {
      deserializeSegment(
          segVar, id, presetId); // apply only the segment with the specified ID
    }
  } else {
    size_t deleted = 0;
    for (json &elem : segVar) {
      if (deserializeSegment(elem, it++, presetId) && !elem["stop"].is_null() &&
          elem["stop"] == 0)
        deleted++;
    }
    if (strip().getSegmentsNum() > 3 &&
        deleted >= strip().getSegmentsNum() / 2U)
      strip().purgeSegments(); // batch deleting more than half segments
  }

  // usermods.readFromJsonState(root);

  // loadLedmap = root[F("ledmap")] | loadLedmap;

  // byte ps = root[F("psave")];
  // if (ps > 0 && ps < 251) savePreset(ps, nullptr, root);

  // ps = root[F("pdel")]; //deletion
  // if (ps > 0 && ps < 251) deletePreset(ps);

  // HTTP API commands (must be handled before "ps")
  // const char* httpwin = root["win"];
  // if (httpwin) {
  //  String apireq = "win"; apireq += '&'; // reduce flash string usage
  //  apireq += httpwin;
  //  handleSet(nullptr, apireq, false);    // may set stateChanged
  //}

  // // applying preset (2 cases: a) API call includes all preset values ("pd"),
  // b) API only specifies preset ID ("ps")) byte presetToRestore = 0;
  // // a) already applied preset content (requires "seg" or "win" but will
  // ignore the rest) if (!root["pd"].isNull() && stateChanged) {
  //   currentPreset = root[F("pd")] | currentPreset;
  //   if (root["win"].isNull()) presetCycCurr = currentPreset; // otherwise it
  //   was set in handleSet() [set.cpp] presetToRestore = currentPreset; //
  //   stateUpdated() will clear the preset, so we need to restore it after
  //   //unloadPlaylist(); // applying a preset unloads the playlist, may be
  //   needed here too?
  // } else if (!root["ps"].isNull()) {
  //   ps = presetCycCurr;
  //   if (root["win"].isNull() && getVal(root["ps"], &ps, 0, 0) && ps > 0 && ps
  //   < 251 && ps != currentPreset) {
  //     // b) preset ID only or preset that does not change state (use embedded
  //     cycling limits if they exist in getVal()) presetCycCurr = ps;
  //     unloadPlaylist();          // applying a preset unloads the playlist
  //     applyPreset(ps, callMode); // async load from file system (only preset
  //     ID was specified) return stateResponse;
  //   }
  // }

  // JsonObject playlist = root[F("playlist")];
  // if (!playlist.isNull() && loadPlaylist(playlist, presetId)) {
  //   //do not notify here, because the first playlist entry will do
  //   if (root["on"].isNull()) callMode = CALL_MODE_NO_NOTIFY;
  //   else callMode = CALL_MODE_DIRECT_CHANGE;  // possible bugfix for playlist
  //   only containing HTTP API preset FX=~
  // }

  if (root.contains(F("rmcpal")) && root[F("rmcpal")].template get<bool>()) {
    if (strip().customPalettes.size()) {
      char fileName[32];
      sprintf_P(fileName, PSTR("/palette%d.json"),
                strip().customPalettes.size() - 1);
      // if (WLED_FS.exists(fileName)) WLED_FS.remove(fileName);
      strip().loadCustomPalettes();
    }
  }

  // stateUpdated(callMode);
  // if (presetToRestore) currentPreset = presetToRestore;
  return stateResponse;
}

std::shared_ptr<httpserver::http_response>
postJson(const httpserver::http_request &request) {
  bool verboseResponse = false;
  bool isConfig = false;

  // if (!requestJSONBufferLock(14)) return;
  // DeserializationError error = deserializeJson(doc,
  // (uint8_t*)(request->_tempObject)); JsonObject root = doc.as<JsonObject>();
  json root = json::parse(request.get_content());
  if (root.is_null()) {
    // releaseJSONBufferLock();
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response("{\"error\":9}", 400,
                                        "application/json"));
  }
  // if (root.containsKey("pin"))
  //   checkSettingsPIN(root["pin"].as<const char *>());

  const auto &path = request.get_path();
  isConfig = (path.find("cfg") != path.npos);
  if (!isConfig) {
    /*
    #ifdef WLED_DEBUG
      DEBUG_PRINTLN(F("Serialized HTTP"));
      serializeJson(root,Serial);
      DEBUG_PRINTLN();
    #endif
    */
    verboseResponse = deserializeState(root);
  } /*else {
    if (!correctPIN && strlen(settingsPIN) > 0) {
      request->send(401, "application/json", F("{\"error\":1}")); // ERR_DENIED
      releaseJSONBufferLock();
      return;
    }
    verboseResponse =
        deserializeConfig(root); // use verboseResponse to determine whether cfg
                                 // change should be saved immediately
  }
  releaseJSONBufferLock();*/

  if (verboseResponse) {
    if (!isConfig) {
      // lastInterfaceUpdate = millis(); // prevent WS update until cooldown
      // interfaceUpdateCallMode = CALL_MODE_WS_SEND; // schedule WS update
      std::cout << "request: " << request << std::endl;
      return serveJson(request); // if JSON contains "v"
      // return;
    } /*else {
      doSerializeConfig = true; // serializeConfig(); //Save new settings to FS
    }*/
  }
  // request->send(200, "application/json", F("{\"success\":true}"));

  return std::shared_ptr<httpserver::http_response>(
      new httpserver::string_response("{\"success\":true}", 200,
                                      "application/json"));
}

std::shared_ptr<httpserver::http_response>
serveJson(const httpserver::http_request &request) {
  if (request.get_path().find("palette") != std::string::npos) {
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(JSON_palette_names, 200,
                                        "application/json"));
  } else if (request.get_path().find("effects") != std::string::npos) {
    json modeNames = serializeModeNames();
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(modeNames.dump(), 200,
                                        "application/json"));
  } else if (request.get_path().find("fxdata") != std::string::npos) {
    json modeData = serializeModeData();
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(modeData.dump(), 200,
                                        "application/json"));
  } else if (request.get_path().find("si") != std::string::npos) {
    json si;
    json &state = si["state"];
    serializeState(&state);
    json &info = si["info"];
    // Not sure if this is needed yet.
    // serializeInfo(&info);
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(si.dump(), 200, "application/json"));
  }
  return std::shared_ptr<httpserver::http_response>(
      new httpserver::string_response("{\"error\":3}", 503,
                                      "application/json"));
}

} // namespace ravecylinder