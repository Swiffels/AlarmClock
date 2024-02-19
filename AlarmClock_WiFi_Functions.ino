void wifiCheck() {

  if (currentTime % 20 == 0) {

    if (WiFi.status() != WL_CONNECTED) {

      Serial.println("Connection Has Been Lost! Trying To Reconnect...");

      WiFi.disconnect();

      WiFi.begin(ssid.c_str(), password.c_str());

    }

  }

}

void handleDeleteWifi() {

  Serial.println("Deleting Wifi File");

  if (alarmOn == true) {

    server.send(409, "text/plain", "Turn Off Active Alarm Before Deleting Wifi");
    return;

  }

  File file = SPIFFS.open("/wifiInfo.json", "w");

  if (!file) {

    Serial.println("Wifi Delete Function Was Unable To Open Wifi File");
    return;

  }

  file.close();

  server.send(200, "text/plain", "Wifi Info Deleted");

  ESP.restart();

}

void handleSetWifi() {

  Serial.println("Writing Wifi File");

  if (alarmOn == true) {

    server.send(409, "text/plain", "Turn Off Active Alarm Before Setting Wifi");
    return;

  }

  ssid = server.arg("ssid");
  password = server.arg("password");

  File file = SPIFFS.open("/wifiInfo.json", "w");

  if (!file) {

    Serial.println("Set Wifi Function Was Unable To Open File");
    return;

  }

  StaticJsonDocument<128> doc;

  doc["ssid"] = ssid;
  doc["password"] = password;


  if (serializeJson(doc, file) == 0) {

    Serial.println("Failed To Write For Save Function");

  }

  file.close();

  String message = "Wifi set to ssid: " + ssid + " password: " + password;

  server.send(200, "text/plain", message);

  delay(4000);

  ESP.restart();

}

void loadWifi() {

  Serial.println("Loading Wifi File");

  File file = SPIFFS.open("/wifiInfo.json", "r");

  if (!file) {

    Serial.println("Load Wifi Function Was Unable To Open File");
    return;

  }

  StaticJsonDocument<128> doc;

  DeserializationError error = deserializeJson(doc, file);

  if (error) {

    Serial.println("Loading Wifi Was Couldn't Deserialize File");

  } else {

    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();

  }

  Serial.println("SSID from file: " + ssid);
  Serial.println("Password from file: " + password);

  file.close();

}
