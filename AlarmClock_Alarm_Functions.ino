void timeManager() {

  if ((millis() - lastTime) > 1000) {
    lastTime = millis();
    currentTime++;
    timeSinceUpdated++;

    if(currentTime % 60 == 0){

      Serial.println(GetTimeProcessed(currentTime));
      
    }

    if (alarmOn == false && (WiFi.status() != WL_CONNECTED || wifiConnected == false)) {

      //digitalWrite(LED_PIN, (digitalRead(LED_PIN) == HIGH ? LOW : HIGH));

    }

    if (currentTime >= 86400) {

      currentTime = 0;

      currentDay++;

      if (currentDay > 7) {

        currentDay = 0;

      }

      if (alarmOn == false && alarmWaiting == false) {

        alarmManager();

      }

    }

    if (timeSinceUpdated >= timeBetweenEachUpdate) {
      timeClient.update();
      currentTime = currentTimezone.toLocal(timeClient.getEpochTime()) % 86400;
      currentDay = ((currentTimezone.toLocal(timeClient.getEpochTime()) / 86400) + 3) % 7 + 1;
      timeSinceUpdated = 0;
    }
  }

}

void alarmManager() {

  Serial.println("Running Alarm Manager");

  nextAlarm.alarmTime = -1;

  for (int i = 0; i < nextAlarmIndex; i++) {

    if (alarms[i].alarmTime > currentTime && ((alarms[i].alarmDays / (int)pow(10, currentDay - 1)) % 10)) {

      if (nextAlarm.alarmTime == -1) {

        nextAlarm = alarms[i];

      }

      if (alarms[i].alarmTime < nextAlarm.alarmTime) {

        nextAlarm = alarms[i];

      }

    }

  }

}

void alarmSequence() {

  if (nextAlarm.alarmTime == currentTime && alarmTimeOn == 0) {

    alarmOn = true;
    alarmTimeOn = millis();

    Serial.println("Alarm Started");

  }
  
  if (alarmOn || alarmWaiting) {

    if ((millis() - alarmTimeOn) > (nextAlarm.alarmMaxDuration * 1000)) {

      resetAlarmVariables();

      Serial.println("Alarm Reset Due To Max Duration");

      alarmManager();

    }

    if (alarmOn) {

      //digitalWrite(BUZZER_PIN, HIGH);
      //digitalWrite(LED_PIN, HIGH);

      if (/*digitalRead(BUTTON_PIN) == LOW || */alarmStop) {

        //digitalWrite(BUZZER_PIN, LOW);
        //digitalWrite(LED_PIN, LOW);
        alarmStop = false;

        alarmOn = false;
        currentAlarmAmount++;

        if (currentAlarmAmount >= nextAlarm.alarmAmount) {

          resetAlarmVariables();

          Serial.println("Alarm Off From Alarm Count");

          alarmManager();

        } else {

          alarmWaiting = true;

          Serial.println("Alarm Off, Now Waiting");

        }

      }

    }

    if (alarmWaiting && alarmTimeWaiting == 0) {

      alarmTimeWaiting = millis();

      Serial.println("Alarm Waiting Started Counting Time Between");

    }

    if (alarmWaiting && ((millis() - alarmTimeWaiting) > (nextAlarm.timeBetween * 1000))) {

      alarmWaiting = false;
      alarmOn = true;

      Serial.println("Alarm Re-enabled");

      alarmTimeWaiting = 0;

    }

  }

}

void resetAlarmVariables() {

  Serial.println("Reset All Alarm Variables");

  alarmWaiting = false;
  alarmOn = false;

  currentAlarmAmount = 0;
  alarmTimeWaiting = 0;
  alarmTimeOn = 0;

}


void handleSetAlarm() {

  Serial.println("Setting New Alarm");

  String alarmTimeString = server.arg("alarmTime");
  String alarmDaysString = server.arg("alarmDays");
  String alarmAmount = server.arg("alarmAmount");
  String alarmMaxDuration = server.arg("maxDuration");
  String timeBetween = server.arg("timeBetween");

  int alarmTime = alarmTimeString.toInt();
  int alarmDays = 0;

  String message = "Alarm Set with time: " + alarmTimeString + " days: " + alarmDaysString + " amount: " + alarmAmount + " max duration: " + alarmMaxDuration + " time between: " + timeBetween;

  if (nextAlarmIndex >= MAX_ALARMS) {

    server.send(429, "text/plain", "Too Many Alarms Set");
    return;
  }

  if (alarmOn == true) {

    server.send(409, "text/plain", "Turn Off Active Alarm Before Adding A New Alarm");
    return;
  }

  for (int i = 0; i < nextAlarmIndex; i++) {

    if (alarmTime == alarms[i].alarmTime) {

      server.send(409, "text/plain", "There Is Already A Alarm Set For This Time");
      return;

    }

  }

  for (int i = 0; i < alarmDaysString.length(); i++) {

    switch (alarmDaysString[i]) {
      case 'M':
        alarmDays += 1;
        break;
      case 'T':
        alarmDays += 10;
        break;
      case 'W':
        alarmDays += 100;
        break;
      case 'R':
        alarmDays += 1000;
        break;
      case 'F':
        alarmDays += 10000;
        break;
      case 'S':
        alarmDays += 100000;
        break;
      case 'U':
        alarmDays += 1000000;
        break;
    }

  }

  alarms[nextAlarmIndex].id = nextAlarmIndex;
  alarms[nextAlarmIndex].alarmTime = alarmTime;
  alarms[nextAlarmIndex].alarmDays = alarmDays;
  alarms[nextAlarmIndex].alarmAmount = alarmAmount.toInt();
  alarms[nextAlarmIndex].alarmMaxDuration = alarmMaxDuration.toInt();
  alarms[nextAlarmIndex].timeBetween = timeBetween.toInt();

  nextAlarmIndex++;

  server.send(200, "text/plain", message);  // Send a response back to the client

  saveAlarms();

  alarmManager();

}

void handleDeleteAlarm() {

  Serial.println("Deleting Old Alarm");

  String alarmTimeString = server.arg("alarmTime");
  bool alarmDeleted = false;

  if (alarmOn == true) {

    server.send(409, "text/plain", "Turn Off Active Alarm Before Deleting An Alarm");
    return;
  }

  int alarmTime = alarmTimeString.toInt();

  for (int i = 0; i < nextAlarmIndex; i++) {

    if (alarms[i].alarmTime == alarmTime) {

      if (nextAlarmIndex > 1) {

        for (int j = i; j < nextAlarmIndex - 1; j++) {

          alarms[j].id = j;
          alarms[j].alarmTime = alarms[j + 1].alarmTime;
          alarms[j].alarmDays = alarms[j + 1].alarmDays;
          alarms[j].alarmAmount = alarms[j + 1].alarmAmount;
          alarms[j].alarmMaxDuration = alarms[j + 1].alarmMaxDuration;
          alarms[j].timeBetween = alarms[j + 1].timeBetween;

        }
      }

      alarmDeleted = true;
      nextAlarmIndex--;

      server.send(200, "text/plain", "Alarm Deleted");

    }

  }

  if (!alarmDeleted) {

    server.send(404, "text/plain", "No Alarm With This Time Found");

  }

  saveAlarms();

  alarmManager();

}

void saveAlarms() {

  Serial.println("Saving Alarms");

  File file = SPIFFS.open("/alarms.json", "w");
  if (!file) {

    Serial.println("Save File Function Was Unable To Open File");
    return;

  }

  StaticJsonDocument<1024> doc;

  JsonArray alarmArray = doc.createNestedArray("alarms");

  for (int i = 0; i < nextAlarmIndex; i++) {

    JsonObject alarmObject = alarmArray.createNestedObject();

    alarmObject["time"] = alarms[i].alarmTime;
    alarmObject["days"] = alarms[i].alarmDays;
    alarmObject["amount"] = alarms[i].alarmAmount;
    alarmObject["duration"] = alarms[i].alarmMaxDuration;
    alarmObject["between"] = alarms[i].timeBetween;

  }

  if (serializeJson(doc, file) == 0) {

    Serial.println("Failed To Write For Save Function");

  }

  file.close();

}

void loadAlarms() {

  Serial.println("Loading Alarms");

  File file = SPIFFS.open("/alarms.json", "r");

  if (!file) {

    Serial.println("Load Alarm Function Was Unable To Open File");
    return;

  }

  StaticJsonDocument<1024> doc;

  DeserializationError error = deserializeJson(doc, file);

  if (error) {

    Serial.println("Loading Alarms Function Was Unable To Deserialize");

  } else {

    JsonArray array = doc["alarms"];

    for (int i = 0; i < array.size(); i++) {

      JsonObject alarmObj = array[i];

      alarms[i].id = i;
      alarms[i].alarmTime = alarmObj["time"];
      alarms[i].alarmDays = alarmObj["days"];
      alarms[i].alarmAmount = alarmObj["amount"];
      alarms[i].alarmMaxDuration = alarmObj["duration"];
      alarms[i].timeBetween = alarmObj["between"];

    }

    nextAlarmIndex = array.size();

  }

  file.close();

}

void handleSendAlarmData() {

  Serial.println("Sending Alarm Data");

  File file = SPIFFS.open("/alarms.json", "r");
  if (!file) {

    server.send(404, "text/plain", "Send Alarm Info File Couldn't Open Or Does Not Exist");
    return;
  }

  server.streamFile(file, "application/json");

  file.close();

}

void handleDeleteAllAlarms() {

  Serial.println("Deleting All Alarm Data");

  if (alarmOn == true) {

    server.send(409, "text/plain", "Please Turn Off Active Alarm Before Deleting All Alarms");
    return;
  }

  nextAlarmIndex = 0;

  File file = SPIFFS.open("/alarms.json", "w");
  if (!file) {

    server.send(404, "text/plain", "Alarm Save File Unable To Open");
    return;
  }

  file.close();

  server.send(200, "text/plain", "All Alarms Deleted");

  nextAlarm.alarmTime = -1;

}

void handleSendAlarmTime() {

  Serial.println("Sending Alarm Time");

  String message = "The current time is: " + currentTime;

  server.send(200, "text/plain", message);

}

void stopAlarm() {

  Serial.println("Stopping Alarm");

  alarmStop = true;
  server.send(200, "text/plain", "Sent");

}
