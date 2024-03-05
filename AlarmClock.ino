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
<html lang=en>
<head>
<meta charset=UTF-8>
<meta name=viewport content="width=device-width, initial-scale=1.0">
<title>AlarmClock - Status</title>
<link href=https://unpkg.com/boxicons@2.1.4/css/boxicons.min.css rel=stylesheet>
<link href=css.css type=text/css rel=stylesheet>
<link rel=preconnect href=https://fonts.googleapis.com>
<link rel=preconnect href=https://fonts.gstatic.com crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Open+Sans:wght@300;400;600&display=swap" rel=stylesheet>
</head>
<body>
<div class=sidebar>
<div class=top>
<div class=logo>
<i class='bx bx-alarm'></i>
<span class=header>AlarmClock</span>
</div>
<i class="bx bx-menu" id=btn></i>
</div>
<ul>
<li>
<a href=/>
<i class='bx bx-info-circle'></i>
<span class=nav-item>Overview</span>
</a>
</li>
<li>
<a href=/alarms>
<i class='bx bx-alarm-add'></i>
<span class=nav-item>Alarms</span>
</a>
</li>
<li>
<a href=#>
<i class='bx bx-cog'></i>
<span class=nav-item>Settings</span>
</a>
</li>
<li>
<a href=#>
<i class='bx bx-message-dots'></i>
<span class=nav-item>Logs</span>
</a>
</li>
<li>
<a href=#>
<i class='bx bx-help-circle'></i>
<span class=nav-item>About</span>
</a>
</li>
</ul>
</div>
<div class=main-content>
<div class=top>
<div class=timeoverview>
<h2>Current Time</h2>
<div class=timecounter>
%%CURRENTTIME%%
</div>
</div>
<div class=wifioverview>
<h2>Wifi</h2>
<p>%%WIFINETWORK%%</p>
</div>
</div>
<div class=nextalarmtime>
<div class=nextalarmtimetext>
<h2>Next Alarm</h2>
<p>%%NEXTALARMTIME%%</p>
</div>
</div>
</div>
</body>
<script>let btn=document.querySelector("#btn");let sidebar=document.querySelector(".sidebar");btn.onclick=function(){sidebar.classList.toggle("active")};</script>
</html>
)rawliteral";

const char *css = R"rawliteral(
input[type="number"]::-webkit-inner-spin-button,input[type="number"]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}input[type="number"]{-moz-appearance:textfield}*{font-family:'Open Sans',sans-serif;margin:0;padding:0;box-sizing:border-box}.sidebar{position:absolute;top:0;left:0;height:100vh;width:80px;background-color:#4d4d4d;padding:.4rem .8rem;transition:all .5s ease;z-index:100}.sidebar.active{width:200px}.sidebar #btn{position:absolute;color:#f5f5f5;top:.47rem;left:50%;font-size:1.5rem;line-height:80px;transform:translateX(-50%);cursor:pointer}.sidebar.active #btn{left:90%}.sidebar .top .logo{color:#f5f5f5;display:flex;height:80px;width:100%;align-items:center;pointer-events:none;opacity:0;font-size:1.3rem;font:800;margin-right:5px;margin-left:12px;border-radius:.8rem}.sidebar.active .top .logo{opacity:1}.sidebar ul li{position:relative;list-style-type:none;height:50px;width:90%;margin:.8rem auto;line-height:50px}.sidebar ul li a{color:#f5f5f5;display:flex;align-items:center;text-decoration:none;border-radius:.8rem}.sidebar ul li a:hover{background-color:#808080;color:#212121}.sidebar ul li a i{min-width:50px;text-align:center;height:50px;border-radius:12px;line-height:50px}.sidebar .nav-item{opacity:0}.sidebar.active .nav-item{opacity:1}.main-content{position:relative;display:flex;flex-direction:column;background-color:#c4c4c4;min-height:100vh;top:0;left:80px;transition:all .5s ease;width:calc(100% - 80px);padding-left:0}.main-content .top,.main-content .nextalarmtime,.main-content .alarmoverlay{display:flex;padding:15px 0 0 0;justify-content:space-around;font:600;text-align:center}.main-content .top>div,.main-content .nextalarmtime>div{background-color:#a1a1a1;height:30vh;width:40%;display:flex;flex-direction:column;justify-content:center}.main-content .nextalarmtime>div{height:25vh;width:90%;display:flex;flex-direction:column;justify-content:center}.main-content>div>div>h2{font-size:3vw}.main-content>div>div>div{font-size:5vw}.main-content>div>div>p{font-size:5vw}.alarmitem{display:flex;justify-content:space-between;align-items:center;background-color:#a1a1a1;border:1.5px solid #000;min-height:100px;max-height:120px}.main-content .alarmoverlay>div>div{height:15vh;width:90%;display:flex;flex-direction:row;justify-content:space-evenly}.alarmoverlayinfo{display:grid;grid-template-columns:1fr 1fr;width:60%;align-items:center;justify-items:center}.alarmoverlayinfo>div{display:flex;justify-content:center;flex-direction:column}.alarmoverlayheaders>h2,.alarmoverlaytimes>h2{font-size:2.75vw}.alarmoverlaytimes>p,.alarmoverlayheaders>p{font-size:2.25vw}.alarmitembuttons{padding:0 0 3.75vh 0}.alarmoverlay{display:flex;flex-direction:column;justify-content:center;align-items:center}.alarmoverlaymaindiv{width:100%;display:flex;flex-direction:column;justify-content:center;align-items:center}.alarmitembuttons>button{min-width:8.5vw;height:3.75vw;font-size:2vw;margin:0 10px 0 10px;padding:0;border-radius:.5vw}.popup{position:fixed;top:50%;left:50%;transform:translate(-40%,-50%);background-color:#b6b6b6;border-radius:7.5px;border:1px solid #a5a5a5;padding:20px;z-index:100;display:none}.popupForm>div{display:flex}.modalOverlay{display:none;position:fixed;width:100%;height:100%;z-index:99;background-color:rgba(0,0,0,0.4)}.addAlarmButton{align-self:center;min-width:10vw;height:3.25vw;font-size:1.5vw;margin:2.5vh 0 0 10px;padding:0;border-radius:.5vw}.addAlarmButton,.alarmitembuttons>button{background-color:#e6e6e6;border:#212121 1.5px solid}.addAlarmButton:hover,.alarmitembuttons>button:hover{background-color:#d6d6d6;border:#212121 1.75px solid}.addAlarmButton:active,.alarmitembuttons>button:active{background-color:#c1c1c1;border:#212121 1.75px solid}
)rawliteral";

const char *alarms_html = R"rawliteral(
<!DOCTYPE html>
<html lang=en>
<head>
<meta charset=UTF-8>
<meta name=viewport content="width=device-width, initial-scale=1.0">
<title>AlarmClock - Status</title>
<script src=alarms_script.js></script>
<link href=https://unpkg.com/boxicons@2.1.4/css/boxicons.min.css rel=stylesheet>
<link href=css.css type=text/css rel=stylesheet>
<link rel=preconnect href=https://fonts.googleapis.com>
<link rel=preconnect href=https://fonts.gstatic.com crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Open+Sans:wght@300;400;600&display=swap" rel=stylesheet>
</head>
<body>
<div class=sidebar>
<div class=top>
<div class=logo>
<i class='bx bx-alarm'></i>
<span class=header>AlarmClock</span>
</div>
<i class="bx bx-menu" id=btn></i>
</div>
<ul>
<li>
<a href=/>
<i class='bx bx-info-circle'></i>
<span class=nav-item>Overview</span>
</a>
</li>
<li>
<a href=/alarms>
<i class='bx bx-alarm-add'></i>
<span class=nav-item>Alarms</span>
</a>
</li>
<li>
<a href=#>
<i class='bx bx-cog'></i>
<span class=nav-item>Settings</span>
</a>
</li>
<li>
<a href=#>
<i class='bx bx-message-dots'></i>
<span class=nav-item>Logs</span>
</a>
</li>
<li>
<a href=#>
<i class='bx bx-help-circle'></i>
<span class=nav-item>About</span>
</a>
</li>
</ul>
</div>
<div id=modalOverlay class=modalOverlay></div>
<div class=main-content>
<div id=alarmoverlay class=alarmoverlay>
</div>
<button class=addAlarmButton id=addAlarmButton>Add Alarm</button>
<div class=popup id=popup>
<form class=popupContent id=popupContent>
<div class=popupForm>
<div>
<p>Alarm Time</p>
<input type=time name=alarmTime id=alarmTimeTime>
</div>
<div>
<fieldset>
<legend>Select Days of the Week</legend>
<label><input type=checkbox name=daysOfWeek id=daysOfWeekMonday value=M> Monday</label>
<label><input type=checkbox name=daysOfWeek id=daysOfWeekTuesday value=T> Tuesday</label>
<label><input type=checkbox name=daysOfWeek id=daysOfWeekWednesday value=W> Wednesday</label>
<label><input type=checkbox name=daysOfWeek id=daysOfWeekThursday value=H> Thursday</label>
<label><input type=checkbox name=daysOfWeek id=daysOfWeekFriday value=F> Friday</label>
<label><input type=checkbox name=daysOfWeek id=daysOfWeekSaturday value=S> Saturday</label>
<label><input type=checkbox name=daysOfWeek id=daysOfWeekSunday value=U> Sunday</label>
</fieldset>
</div>
<div>
<label>Alarm Amount:</label>
<input type=number name=alarmAmount id=alarmAmountNumber min=1 max=2048 value=3 style=text-align:right>
</div>
<div>
<label>Max Duration:</label>
<input type=number name=maxDuration id=maxDurationNumber min=1 max=86400 value=3600 style=text-align:right>
</div>
<div>
<label>Time Between:</label>
<input type=number name=timeBetween id=timeBetweenNumber min=0 max=86400 value=30 style=text-align:right>
</div>
</div>
<div class=popupbuttonselection>
<input type=button value=Save id=saveEditPopup>
<button id=closeEditPopup type=button>Close</button>
</div>
</form>
</div>
</div>
</body>
<script>let btn=document.querySelector("#btn");let sidebar=document.querySelector(".sidebar");btn.onclick=function(){sidebar.classList.toggle("active")};</script>
</html>
)rawliteral";

const char *alarms_script = R"rawliteral(
document.addEventListener("DOMContentLoaded",function(){const e=document.getElementById("alarmoverlay"),h=document.getElementById("popup"),t=document.getElementById("closeEditPopup"),n=document.getElementById("saveEditPopup"),E=document.getElementById("modalOverlay"),a=document.getElementById("addAlarmButton"),p=document.createElement("div");p.className="alarmoverlaymaindiv",e.appendChild(p);var v="";function d(){const e=document.querySelectorAll('input[name="daysOfWeek"]:checked');var t="";return e.forEach(e=>{t+=e.value}),t}fetch("/sendAlarmData").then(e=>{if(!e.ok)throw new Error("Network response was not ok");return e.json()}).then(e=>{console.log(e),e.alarms.forEach((e,t)=>{const n=document.createElement("div");n.className="alarmitem",p.appendChild(n);const a=document.createElement("div");a.className="alarmoverlayinfo",n.appendChild(a);const d=document.createElement("div");d.className="alarmoverlayheaders",a.appendChild(d);const m=document.createElement("h2");m.textContent="Alarm Time",d.appendChild(m);const o=document.createElement("p");o.textContent=new Date(1e3*(e.time+25200)).toLocaleTimeString("en-US",{hour:"2-digit",minute:"2-digit"}),d.appendChild(o);const c=document.createElement("div");c.className="alarmoverlaytimes",a.appendChild(c);const l=document.createElement("h2");l.textContent="Days",c.appendChild(l);const r=document.createElement("p");var u="";Math.trunc(+e.days)%10&&(u+="M"),Math.trunc(e.days/10)%10&&(u+="T"),Math.trunc(e.days/100)%10&&(u+="W"),Math.trunc(e.days/1e3)%10&&(u+="R"),Math.trunc(e.days/1e4)%10&&(u+="F"),Math.trunc(e.days/1e5)%10&&(u+="S"),Math.trunc(e.days/1e6)%10&&(u+="U"),""==u&&(u+="​"),r.textContent=u,c.appendChild(r);const s=document.createElement("div");s.className="alarmitembuttons",n.appendChild(s);const i=document.createElement("button");i.textContent="Edit",i.addEventListener("click",function(){v=t,document.getElementById("alarmTimeTime").value=new Date(1e3*e.time).toISOString().slice(11,16),document.getElementById("daysOfWeekMonday").checked=Math.trunc(+e.days)%10,document.getElementById("daysOfWeekTuesday").checked=Math.trunc(e.days/10)%10,document.getElementById("daysOfWeekWednesday").checked=Math.trunc(e.days/100)%10,document.getElementById("daysOfWeekThursday").checked=Math.trunc(e.days/1e3)%10,document.getElementById("daysOfWeekFriday").checked=Math.trunc(e.days/1e4)%10,document.getElementById("daysOfWeekSaturday").checked=Math.trunc(e.days/1e5)%10,document.getElementById("daysOfWeekSunday").checked=Math.trunc(e.days/1e6)%10,document.getElementById("alarmAmountNumber").value=e.amount,document.getElementById("maxDurationNumber").value=e.duration,document.getElementById("timeBetweenNumber").value=e.between,h.style.display="block",E.style.display="block"}),s.appendChild(i);const y=document.createElement("button");y.textContent="Remove",y.addEventListener("click",function(){fetch("/deleteAlarm",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:"alarmTime="+encodeURIComponent(e.time)}).then(t=>{t.text().then(e=>{t.ok?(console.log("Success:",e),window.location.reload()):alert("Error: "+e)})}).catch(e=>{alert("Network error: "+e.message)})}),s.appendChild(y)})}).catch(e=>{console.error("There was a problem with the fetch operation:",e)}),t.addEventListener("click",function(){v="",E.style.display="none",h.style.display="none"}),n.addEventListener("click",function(){console.log(document.getElementById("alarmTimeTime").value),""!=document.getElementById("alarmTimeTime").value&&""!=d()&&""!=document.getElementById("alarmAmountNumber").value&&""!=document.getElementById("maxDurationNumber").value&&""!=document.getElementById("timeBetweenNumber").value?fetch("/setAlarm",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:"alarmId="+encodeURIComponent(v)+"&alarmTime="+encodeURIComponent(function(){const e=document.getElementById("alarmTimeTime").value,t=parseInt(e.slice(0,2),10),n=parseInt(e.slice(3,5),10);return 3600*t+60*n}())+"&alarmDays="+encodeURIComponent(d())+"&alarmAmount="+encodeURIComponent(document.getElementById("alarmAmountNumber").value)+"&maxDuration="+encodeURIComponent(document.getElementById("maxDurationNumber").value)+"&timeBetween="+encodeURIComponent(document.getElementById("timeBetweenNumber").value)}).then(t=>{t.text().then(e=>{t.ok?(console.log("Success:",e),window.location.reload()):alert("Error: "+e)})}).catch(e=>{alert("Network error: "+e.message)}):alert("Please fill out all info before saving")}),a.addEventListener("click",function(){document.getElementById("alarmTimeTime").value="",document.getElementById("daysOfWeekMonday").checked=!1,document.getElementById("daysOfWeekTuesday").checked=!1,document.getElementById("daysOfWeekWednesday").checked=!1,document.getElementById("daysOfWeekThursday").checked=!1,document.getElementById("daysOfWeekFriday").checked=!1,document.getElementById("daysOfWeekSaturday").checked=!1,document.getElementById("daysOfWeekSunday").checked=!1,document.getElementById("alarmAmountNumber").value=3,document.getElementById("maxDurationNumber").value=3600,document.getElementById("timeBetweenNumber").value=30,E.style.display="block",h.style.display="block"}),document.getElementById("alarmAmountNumber").addEventListener("change",function(){var e=this.value;(e<1||2048<e)&&(alert("Please enter an amount between 1 and 2048."),this.value="")}),document.getElementById("maxDurationNumber").addEventListener("change",function(){var e=this.value;(e<1||2048<e)&&(alert("Please enter an amount between 1 and 86400."),this.value="")}),document.getElementById("timeBetweenNumber").addEventListener("change",function(){var e=this.value;(e<1||2048<e)&&(alert("Please enter an amount between 1 and 86400."),this.value="")})});
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
    server.on("/setAlarm", HTTP_POST, handleSetAlarm);
    server.on("/deleteAlarm", HTTP_POST, handleDeleteAlarm);
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
    server.on("/alarms", [](){
      String htmlContent = alarms_html;
      server.send(200, "text/html", htmlContent);
      htmlContent = "";
    });
    server.on("/alarms_script.js", []() {
      server.send(200, "text/javascript", alarms_script);
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
