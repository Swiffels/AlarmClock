void handleSetRegion() {

  Serial.println("Setting Region Data");

  if (alarmOn == true) {

    server.send(409, "text/plain", "Turn Off Active Alarm Before Setting Region");
    return;

  }

  String timeDelayString = server.arg("region");
  timeDelay = timeDelayString.toInt();

  switchRegion();

  File file = SPIFFS.open("/regionInfo.json", "w");

  if (!file) {

    Serial.println("Region Set Function Was Unable To Open File");
    return;

  }

  StaticJsonDocument<64> doc;

  doc["Delay"] = timeDelay;

  if (serializeJson(doc, file) == 0) {

    Serial.println("Failed To Write Region Save Function");

  }

  file.close();

  String message = "Region set to a time delay of: " + timeDelayString;

  server.send(200, "text/plain", message);

  timeClient.update();
  currentTime = currentTimezone.toLocal(timeClient.getEpochTime()) % 86400;
  currentDay = ((currentTimezone.toLocal(timeClient.getEpochTime()) / 86400) + 3) % 7 + 1;

  loadAlarms();

  alarmManager();

}

void loadRegion() {

  Serial.println("Loading Region Data");

  File file = SPIFFS.open("/regionInfo.json", "r");

  if (!file) {

    Serial.println("Region Load Function Was Unable To Open File");
    return;

  }

  StaticJsonDocument<64> doc;

  DeserializationError error = deserializeJson(doc, file);

  if (error) {

    Serial.println("Loading Region File Couldn't Deserialize");

  } else {

    timeDelay = doc["Delay"];

  }

  Serial.print("Region From File: ");
  Serial.println(timeDelay);

  file.close();

  switchRegion();


}

void switchRegion() {

  switch (timeDelay) {
    case -4: {
        // US Eastern Time Zone (New York, Detroit)
        TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Daylight time = UTC - 4 hours
        TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Standard time = UTC - 5 hours
        Timezone usEastern(usEDT, usEST);
        currentTimezone = usEastern;
        break;
      }
    case -5: {
        // US Central Time Zone (Chicago, St Louis)
        TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
        TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
        Timezone usCentral(usCDT, usCST);
        currentTimezone = usCentral;
        break;
      }
    case -6: {
        // US Mountain Time Zone (Denver, Phoenix)
        TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};
        TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};
        Timezone usMountain(usMDT, usMST);
        currentTimezone = usMountain;
        break;
      }
    case -7: {
        // US Pacific Time Zone (Los Angeles, San Francisco)
        TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
        TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
        Timezone usPacific(usPDT, usPST);
        currentTimezone = usPacific;
        break;
      }
    case -10: {
        TimeChangeRule hiST = {"HST", First, Sun, Jan, 1, -600}; // Hawaii Standard Time (no DST change)
        Timezone usHawaii(hiST, hiST);
        currentTimezone = usHawaii;
        break;
      }
    case -8: {
        TimeChangeRule akDT = {"AKDT", Second, Sun, Mar, 2, -480}; // Alaska Daylight Time = UTC - 8 hours
        TimeChangeRule akST = {"AKST", First, Sun, Nov, 2, -540};  // Alaska Standard Time = UTC - 9 hours
        Timezone usAlaska(akDT, akST);
        currentTimezone = usAlaska;
        break;
      }
  }
}
