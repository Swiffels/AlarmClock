document.addEventListener('DOMContentLoaded', function() {

    const container = document.getElementById('alarmoverlay');
    const popup = document.getElementById('popup');
    const closeEditPopupButton = document.getElementById('closeEditPopup');
    const saveEditPopupButton = document.getElementById('saveEditPopup');
    const modalOverlay = document.getElementById('modalOverlay');
    const addAlarmButton = document.getElementById('addAlarmButton');

    const alarmOverlayMainDiv = document.createElement('div');
    alarmOverlayMainDiv.className = 'alarmoverlaymaindiv';
    container.appendChild(alarmOverlayMainDiv);

    var alarmId = '';

    fetch('/sendAlarmData')
        .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
            return response.json(); // Parse the response body as JSON
        })
        .then(fetchedData => {
            console.log(fetchedData);
            fetchedData.alarms.forEach((alarm, index) => {
                const alarmItem = document.createElement('div');
                alarmItem.className = 'alarmitem';
                alarmOverlayMainDiv.appendChild(alarmItem);
        
                const alarmOverlayInfo = document.createElement('div');
                alarmOverlayInfo.className = 'alarmoverlayinfo';
                alarmItem.appendChild(alarmOverlayInfo);
        
                const alarmOverlayHeaders = document.createElement('div');
                alarmOverlayHeaders.className = 'alarmoverlayheaders';
                alarmOverlayInfo.appendChild(alarmOverlayHeaders);
        
                const timeH2 = document.createElement('h2');
                timeH2.textContent = 'Alarm Time';
                alarmOverlayHeaders.appendChild(timeH2);
        
                const timeP = document.createElement('p');
                timeP.textContent = new Date((alarm.time + 25200) * 1000).toLocaleTimeString('en-US', {hour: '2-digit', minute:'2-digit'});
                alarmOverlayHeaders.appendChild(timeP);
        
                const alarmOverlayTimes = document.createElement('div');
                alarmOverlayTimes.className = 'alarmoverlaytimes';
                alarmOverlayInfo.appendChild(alarmOverlayTimes);
        
                const dayH2 = document.createElement('h2');
                dayH2.textContent = 'Days';
                alarmOverlayTimes.appendChild(dayH2);
        
                const dayP = document.createElement('p');
        
                var days = '';
        
                Math.trunc(alarm.days / 1) % 10? days += 'M': null;
                Math.trunc(alarm.days / 10) % 10? days += 'T': null;
                Math.trunc(alarm.days / 100) % 10? days += 'W': null;
                Math.trunc(alarm.days / 1000) % 10? days += 'R': null;
                Math.trunc(alarm.days / 10000) % 10? days += 'F': null;
                Math.trunc(alarm.days / 100000) % 10? days += 'S': null;
                Math.trunc(alarm.days / 1000000) % 10? days += 'U': null;
        
                days == ''? days += '\u200B': null;
        
                dayP.textContent = days;
                alarmOverlayTimes.appendChild(dayP);
        
                const alarmItemButtons = document.createElement('div');
                alarmItemButtons.className = 'alarmitembuttons';
                alarmItem.appendChild(alarmItemButtons);
        
                //Popup edit button code
                const editButton = document.createElement('button');
                editButton.textContent = 'Edit';
                editButton.addEventListener('click', function(){

                    alarmId = index;
        
                    document.getElementById('alarmTimeTime').value = new Date((alarm.time) * 1000).toISOString().slice(11, 16);
                    
                    document.getElementById('daysOfWeekMonday').checked = Math.trunc(alarm.days / 1) % 10;
                    document.getElementById('daysOfWeekTuesday').checked = Math.trunc(alarm.days / 10) % 10;
                    document.getElementById('daysOfWeekWednesday').checked = Math.trunc(alarm.days / 100) % 10;
                    document.getElementById('daysOfWeekThursday').checked = Math.trunc(alarm.days / 1000) % 10;
                    document.getElementById('daysOfWeekFriday').checked = Math.trunc(alarm.days / 10000) % 10;
                    document.getElementById('daysOfWeekSaturday').checked = Math.trunc(alarm.days / 100000) % 10;
                    document.getElementById('daysOfWeekSunday').checked = Math.trunc(alarm.days / 1000000) % 10;
        
                    document.getElementById('alarmAmountNumber').value = alarm.amount;
                    document.getElementById('maxDurationNumber').value = alarm.duration;
                    document.getElementById('timeBetweenNumber').value = alarm.between;
                    
                    popup.style.display = 'block';
                    modalOverlay.style.display = 'block';
        
                });
                alarmItemButtons.appendChild(editButton);
        
                //Popup remove button code
                const removeButton = document.createElement('button');
                removeButton.textContent = 'Remove';
                removeButton.addEventListener('click', function(){
        
                    fetch('/deleteAlarm', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/x-www-form-urlencoded',
                        },
                        body: 'alarmTime=' + encodeURIComponent(alarm.time),
                    })
                    .then(response => {
                        response.text().then(text => {
                            if (response.ok) {
                                console.log('Success:', text);
                                window.location.reload();
                            } else {
                                alert("Error: " + text);
                            }
                        });
                    })
                    .catch(error => {
                        alert("Network error: " + error.message);
                    });
        
                });
                alarmItemButtons.appendChild(removeButton);
            
            });
        })
        .catch(error => {
            console.error('There was a problem with the fetch operation:', error);
        });

    //Popup close button code
    closeEditPopupButton.addEventListener('click', function(){

        alarmId = '';

        modalOverlay.style.display = 'none';
        popup.style.display = 'none';

    });

    //Popup save button code
    saveEditPopupButton.addEventListener('click', function(){

        console.log(document.getElementById('alarmTimeTime').value);

        if(document.getElementById('alarmTimeTime').value != '' && getSelectedDays() != '' && document.getElementById('alarmAmountNumber').value != '' && document.getElementById('maxDurationNumber').value != '' && document.getElementById('timeBetweenNumber').value != ''){

            fetch('/setAlarm', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body:
                    'alarmId=' + encodeURIComponent(alarmId) +
                    '&alarmTime=' + encodeURIComponent(getAlarmTime()) +
                    '&alarmDays=' + encodeURIComponent(getSelectedDays()) +
                    '&alarmAmount=' + encodeURIComponent(document.getElementById('alarmAmountNumber').value) +
                    '&maxDuration=' + encodeURIComponent(document.getElementById('maxDurationNumber').value) +
                    '&timeBetween=' + encodeURIComponent(document.getElementById('timeBetweenNumber').value),
            })
            .then(response => {
                response.text().then(text => {
                    if (response.ok) {
                        console.log('Success:', text);
                        window.location.reload();
                    } else {
                        alert("Error: " + text);
                    }
                });
            })
            .catch(error => {
                alert("Network error: " + error.message);
            });
        
        }else{

            alert("Please fill out all info before saving");

        }

    });

    //Add new alarm button code
    addAlarmButton.addEventListener('click', function(){

        document.getElementById('alarmTimeTime').value = '';
                    
        document.getElementById('daysOfWeekMonday').checked = false;
        document.getElementById('daysOfWeekTuesday').checked = false;
        document.getElementById('daysOfWeekWednesday').checked = false;
        document.getElementById('daysOfWeekThursday').checked = false;
        document.getElementById('daysOfWeekFriday').checked = false;
        document.getElementById('daysOfWeekSaturday').checked = false;
        document.getElementById('daysOfWeekSunday').checked = false;

        document.getElementById('alarmAmountNumber').value = 3;
        document.getElementById('maxDurationNumber').value = 3600;
        document.getElementById('timeBetweenNumber').value = 30;

        modalOverlay.style.display = 'block';
        popup.style.display = 'block';

    });

    //Checking Text Values Min and Max When Changed
    document.getElementById('alarmAmountNumber').addEventListener('change', function(){
        var amount = this.value;
        if(amount < 1 || amount > 2048){
            alert("Please enter an amount between 1 and 2048.");
            this.value = '';
        }
    });
    document.getElementById('maxDurationNumber').addEventListener('change', function(){
        var amount = this.value;
        if(amount < 1 || amount > 2048){
            alert("Please enter an amount between 1 and 86400.");
            this.value = '';
        }
    });
    document.getElementById('timeBetweenNumber').addEventListener('change', function(){
        var amount = this.value;
        if(amount < 1 || amount > 2048){
            alert("Please enter an amount between 1 and 86400.");
            this.value = '';
        }
    });

    function getSelectedDays(){

        const checkboxes = document.querySelectorAll('input[name="daysOfWeek"]:checked');
        var days = '';

        checkboxes.forEach((checkbox) => {
            days += checkbox.value;
        });

        return days;
    }

    function getAlarmTime() {

        const alarmTimeValue = document.getElementById('alarmTimeTime').value;
    
        const hour = parseInt(alarmTimeValue.slice(0, 2), 10);
        const minute = parseInt(alarmTimeValue.slice(3, 5), 10);
    
        return (hour * 3600) + (minute * 60);
    }

});
