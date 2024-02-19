void deleteSettingsFile() {

  Serial.println("Deleting Settings File");

  File file = SPIFFS.open("/settings.json", "w");

  file.close();

}

void handleSetSettings() {

  Serial.println("Writing Settings File");

  if (alarmOn == true) {

    server.send(409, "text/plain", "Turn Off Active Alarm Before Changing Settings");
    return;

  }

  String militaryTimeString = server.arg("militaryTime");
  militaryTime = militaryTimeString.toInt();

  deviceName = server.arg("deviceName");

  File file = SPIFFS.open("/settings.json", "w");

  if (!file) {

    Serial.println("Set Settings Function Was Unable To Open File");
    return;

  }

  StaticJsonDocument<128> doc;

  doc["mili"] = militaryTime;
  doc["name"] = deviceName;

  if (serializeJson(doc, file) == 0) {

    Serial.println("Failed To Serialize Settings");

  }

  file.close();

  String message = " Settings Are Now Set, Military Time: " + militaryTimeString + ", Device Name: " + deviceName;

  server.send(200, "text/plain", message);
  
}

void loadSettings() {

  Serial.println("Loading Settings");

  File file = SPIFFS.open("/settings.json", "r");

  if (!file) {

    Serial.println("Settings Were Unable To Open");
    return;

  }

  StaticJsonDocument<128> doc;

  DeserializationError error = deserializeJson(doc, file);

  if (error) {

    Serial.println("Deserialize Settings File Failed");

  } else {

    militaryTime = doc["mili"];
    deviceName = doc["name"].as<String>();

  }

  file.close();

  if(deviceName.length() == 0 || deviceName == NULL){

    deviceName = "AlarmClockPrototype";
    
  }

}
