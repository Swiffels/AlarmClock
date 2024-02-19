#include <ArduinoJson.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WifiUdp.h>
#include <NTPClient.h>
#include <Timezone.h>

#define MAX_ALARMS 8

//#define BUZZER_PIN 2
//#define BUTTON_PIN 1
//#define LED_PIN 3

String ssid = "";
String password = "";

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org");

struct Alarm {

  int id;
  int alarmTime;
  int alarmDays;
  int alarmAmount;
  int alarmMaxDuration;
  int timeBetween;

};

Alarm alarms[MAX_ALARMS];
int nextAlarmIndex = 0;

int currentTime;
int currentDay;
unsigned long lastTime = 0;
int timeSinceUpdated = 0;
int timeBetweenEachUpdate = 3600;

Alarm nextAlarm;

int currentAlarmAmount = 0;

bool alarmOn = false;
bool alarmWaiting = false;

bool wifiConnected = true;
int wifiTimeWaiting = 0;

String deviceName = "AlarmClockPrototype";

int timeDelay = 0;
TimeChangeRule utcRule = { "UTC", Last, Sun, Mar, 1, 0 };
Timezone currentTimezone(utcRule, utcRule);

unsigned long alarmTimeOn = 0;
unsigned long alarmTimeWaiting = 0;

bool alarmStop = false;

bool militaryTime = 0;

int udpPort = 80;

ESP8266WebServer server(udpPort);  // Server on port 80

// ++++++++++++ Function Declarations ++++++++++++ //

void wifiCheck();
void handleDeleteWifi();
void handleSetWifi();
void loadWifi();

void handleSetRegion();
void loadRegion();
void switchRegion();

void timeManager();
void alarmManager();
void alarmSequence();
void resetAlarmVariables();
void handleSetAlarm();
void handleDeleteAlarm();
void saveAlarms();
void loadAlarms();
void handleSendAlarmData();
void handleDeleteAllAlarms();
void handleSendAlarmTime();
void stopAlarm();

String GetTimeProcessed(int Time);
int GetTimeUnprocessed(String Time);
String nextAlarmTime();

void handleSetSettings();
void deleteSettingsFile();
void loadSettings();

// +++++++++++++++++++++++++++++++++++++++++++++++ //

// ++++++++++++ Website Declarations +++++++++++++ //

const char *overview_html = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AlarmClock - Status</title>

    <link href='https://unpkg.com/boxicons@2.1.4/css/boxicons.min.css' rel='stylesheet'>
    <link href="css.css" type="text/css" rel="stylesheet">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Open+Sans:wght@300;400;600&display=swap" rel="stylesheet">

</head>
<body>
    
    <div class="sidebar">
        <div class="top">
            <div class="logo">
                <i class='bx bx-alarm'></i>
                <span class="header">AlarmClock</span>
            </div>
            <i class="bx bx-menu" id="btn"></i>
        </div>
        <ul>
            <li>
                <a href="#">
                    <i class='bx bx-info-circle' ></i>
                    <span class="nav-item">Overview</span>
                </a>
            </li>
            <li>
                <a href="#">
                    <i class='bx bx-alarm-add' ></i>
                    <span class="nav-item">Alarms</span>
                </a>
            </li>
            <li>
                <a href="#">
                    <i class='bx bx-cog' ></i>
                    <span class="nav-item">Settings</span>
                </a>
            </li>
            <li>
                <a href="#">
                    <i class='bx bx-message-dots' ></i>
                    <span class="nav-item">Logs</span>
                </a>
            </li>
            <li>
                <a href="#">
                    <i class='bx bx-help-circle'></i>
                    <span class="nav-item">About</span>
                </a>
            </li>
        </ul>

    </div>

    <div class="main-content">
        <div class="top">
            <div class="timeoverview">
                <h2>Current Time</h2>
                <div class="timecounter">
                    %%CURRENTTIME%%
                </div>
            </div>
            <div class="wifioverview">
                <h2>Wifi</h2>
                <p>%%WIFINETWORK%%</p>
            </div>
        </div>
        <div class="nextalarmtime">
            <div class="nextalarmtimetext">
                <h2>Next Alarm</h2>
                <p>%%NEXTALARMTIME%%</p>
            </div>
        </div>
    </div>

</body>

<script>

    let btn = document.querySelector("#btn");
    let sidebar = document.querySelector(".sidebar");

    btn.onclick = function () {
        sidebar.classList.toggle("active");
    }

</script>

</html>

)rawliteral";

const char *css = R"rawliteral(

*{
    font-family: 'Open Sans', sans-serif;
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

.sidebar{
    position: absolute;
    top: 0;
    left: 0;
    height: 100vh;
    width: 80px;
    background-color: #4d4d4d;
    padding: .4rem .8rem;
    transition: all 0.5s ease;
    z-index: 100;
}

.sidebar.active{
    width: 200px;
}

.sidebar #btn{
    position: absolute;
    color: #f5f5f5;
    top: .47rem;
    left: 50%;
    font-size: 1.5rem;
    line-height: 80px;
    transform: translateX(-50%);
    cursor: pointer;
}

.sidebar.active #btn{
    left: 90%;
}

.sidebar .top .logo{
    color: #f5f5f5;
    display: flex;
    height: 80px;
    width: 100%;
    align-items: center;
    pointer-events: none;
    opacity: 0;
    font-size: 1.3rem;
    font: 800;
    margin-right: 5px;
    margin-left: 12px;
    border-radius: 0.8rem;
}

.sidebar.active .top .logo{
    opacity: 1;
}

.sidebar ul li{
    position: relative;
    list-style-type: none;
    height: 50px;
    width: 90%;
    margin: .8rem auto;
    line-height: 50px;
}

.sidebar ul li a{
    color: #f5f5f5;
    display: flex;
    align-items: center;
    text-decoration: none;
    border-radius: 0.8rem;
}

.sidebar ul li a:hover{
    background-color: #808080;
    color: #212121;
}

.sidebar ul li a i{
    min-width: 50px;
    text-align: center;
    height: 50px;
    border-radius: 12px;
    line-height: 50px;
}

.sidebar .nav-item{
    opacity: 0;
}

.sidebar.active .nav-item{
    opacity: 1;
}

.main-content{
    position: relative;
    background-color: #c4c4c4;
    min-height: 100vh;
    top: 0;
    left: 80px;
    transition: all 0.5s ease;
    width: calc(100% - 80px);
    padding-left: 0px;
}

.main-content .top, .main-content .nextalarmtime{
    display: flex;
    padding: 15px 0px 0px 0px;
    justify-content: space-around;
    font: 600;
    text-align: center;
}

.main-content .top > div, .main-content .nextalarmtime > div{
    background-color: #a1a1a1;
    height: 30vh;
    width: 40%;
    display: flex;
    flex-direction: column;
    justify-content: center;
}

.main-content .nextalarmtime > div{
    background-color: #a1a1a1;
    height: 15vh;
    width: 90%;
    display: flex;
    flex-direction: column;
    justify-content: center;
}

.main-content > div > div > h2{
    font-size: 3vw;
}

.main-content > div > div > div{
    font-size: 5vw;
}

.main-content > div > div > p{
    font-size: 5vw;
}

)rawliteral";

// +++++++++++++++++++++++++++++++++++++++++++++++ //

void setup() {

  Serial.begin(115200);

  Serial.println("Starting Up");

  SPIFFS.begin();

  WiFi.persistent(false);

  WiFi.mode(WIFI_STA);

  //pinMode(BUZZER_PIN, OUTPUT);
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  //pinMode(LED_PIN, OUTPUT);

  loadWifi();
  loadRegion();
  loadSettings();

  WiFi.hostname(deviceName);

  WiFi.begin(ssid.c_str(), password.c_str());

  wifiTimeWaiting = 0;
  wifiConnected = true;

  while (WiFi.status() != WL_CONNECTED) {

    delay(1000);
    Serial.println("Connecting to WiFi...");

    wifiTimeWaiting++;

    if (wifiTimeWaiting > 10 || (ssid == "" && password == "")) {

      Serial.println("Wifi Not Connecting, Going Into Wifi Setup");

      wifiConnected = false;

      WiFi.disconnect();

      delay(1000);

      WiFi.mode(WIFI_AP);

      const char *ssid = "AlarmClock Setup";
      const char *password = "AlarmClock";

      IPAddress local_IP(192, 168, 0, 100);
      IPAddress gateway(192, 168, 0, 1);
      IPAddress subnet(255, 255, 255, 0);

      if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {

        Serial.println("Wifi Settings Not Set Correctly");

      }

      delay(1000);

      WiFi.softAP(ssid, password);

      if (!MDNS.begin("AlarmClockSetup")) { // Start the mDNS responder

        Serial.println("Error Setting Up MDNS Responder!");

      } else {

        Serial.println("mDNS Responder Started");
        MDNS.addService("http", "tcp", 80);

      }

      server.on("/setWifi", HTTP_GET, handleSetWifi);

      server.begin();

      Serial.print("IP Address: ");
      Serial.println(WiFi.softAPIP());

      break;

    }

  }

  if (wifiConnected == true) {

    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    if (!MDNS.begin(deviceName.c_str())) { // Start the mDNS responder

      Serial.println("Error Setting Up MDNS Responder!");

    } else {

      Serial.println("mDNS Responder Started");
      MDNS.addService("http", "tcp", 80);

    }

    // Define endpoint handlers
    server.on("/setAlarm", HTTP_GET, handleSetAlarm);
    server.on("/deleteAlarm", HTTP_GET, handleDeleteAlarm);
    server.on("/sendAlarmData", HTTP_GET, handleSendAlarmData);
    server.on("/deleteAllAlarms", HTTP_GET, handleDeleteAllAlarms);
    server.on("/stopAlarm", HTTP_GET, stopAlarm);
    server.on("/setRegion", HTTP_GET, handleSetRegion);
    server.on("/setWifi", HTTP_GET, handleSetWifi);
    server.on("/deleteWifi", HTTP_GET, handleDeleteWifi);
    server.on("/setSettings", HTTP_GET, handleSetSettings);
    
    server.on("/", [](){
      String htmlContent = overview_html;
      htmlContent.replace("%%CURRENTTIME%%", GetTimeProcessed(currentTime));
      htmlContent.replace("%%WIFINETWORK%%", ssid);
      htmlContent.replace("%%NEXTALARMTIME%%", nextAlarmTime());
      server.send(200, "text/html", htmlContent);
      htmlContent = "";
    });
    server.on("/css.css", []() {
      server.send(200, "text/css", css);
    });
    

    server.begin();  // Start the server

    timeClient.begin();
    timeClient.update();
    currentTime = currentTimezone.toLocal(timeClient.getEpochTime()) % 86400;
    currentDay = ((currentTimezone.toLocal(timeClient.getEpochTime()) / 86400) + 3) % 7 + 1;

    Serial.print("The Current Day Is: ");
    Serial.print(currentDay);
    Serial.print(" And The Current Time Is: ");
    Serial.println(currentTime);

    loadAlarms();

    alarmManager();

  }
}

void loop() {

  if (wifiConnected == false) {

    server.handleClient();  // Listen for incoming clients

    MDNS.update();

    timeManager();

  } else {

    server.handleClient();

    MDNS.update();

    timeManager();

    alarmSequence();

    wifiCheck();

  }

}
