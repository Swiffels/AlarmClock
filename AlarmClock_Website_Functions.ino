String GetTimeProcessed(int Time) {

  int hours = Time / 3600;
  int minutes = (Time % 3600) / 60;
  String other = "";

  if(!militaryTime && hours > 12){

    hours -= 12;
    other = "PM";
    
  }else if(!militaryTime){

    other = "AM";
    
  }

  String stringHours = hours < 10? "0" + String(hours): String(hours);
  String stringMinutes = minutes < 10? "0" + String(minutes): String(minutes);

  return stringHours + ":" + stringMinutes + " " + other;
}

int GetTimeUnprocessed(String Time){

  String hours = Time.substring(0,2);
  String minutes = Time.substring(3,5);

  int intHours = hours.toInt() * 3600;
  int intMinutes = minutes.toInt() * 60;

  if(!militaryTime){

    String other = Time.substring(6,8);

    return other == "AM"? intHours + intMinutes: intHours + intMinutes + 43200;
  }

  return intHours + intMinutes;
}

String nextAlarmTime() {

  if(nextAlarmIndex == 0)
    return "No Alarms Currently Set";

    //if (alarms[i].alarmTime > currentTime && ((nextAlarm.alarmDays / (int)pow(10, currentDay - 1)) % 10)) {

  int soonestTime = -1;

  for(int i = currentDay; i <= 7 + currentDay; i++){

    int dayIndex = i > 7? i - 7: i;
  
    for(int j = 0; j < nextAlarmIndex; j++){

      if(i == currentDay){

        if(alarms[j].alarmTime > currentTime && ((alarms[j].alarmDays / (int)pow(10, currentDay - 1)) % 10) && (soonestTime == -1 || soonestTime > alarms[j].alarmTime)){

          soonestTime = alarms[j].alarmTime;

        }
          
      }else{

        if(((alarms[j].alarmDays / (int)pow(10, dayIndex - 1)) % 10) && (soonestTime == -1 || soonestTime > alarms[j].alarmTime)){

          soonestTime = alarms[j].alarmTime;
        
        }
        
      }
      
    }

    if(soonestTime != -1){

      String dayIndexString;
    
      switch (dayIndex)
      {
        case 1:
          dayIndexString = "Monday";
        case 2:
          dayIndexString = "Tuesday";
        case 3:
          dayIndexString = "Wednesday";
        case 4:
          dayIndexString = "Thursday";
        case 5:
          dayIndexString = "Friday";
        case 6:
          dayIndexString = "Saturday";
        case 7:
          dayIndexString = "Sunday";
        default:
          dayIndexString = "";
      }
      
      return dayIndexString + "At" + GetTimeProcessed(soonestTime);
    }
  }

}
