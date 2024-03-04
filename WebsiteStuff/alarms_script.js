document.addEventListener('DOMContentLoaded', function() {

    const container = document.getElementById('alarmoverlay');
    const popup = document.getElementById('popup');
    const popContent = document.getElementById('popupContent');
    const closeEditPopupButton = document.getElementById('closeEditPopup');
    const saveEditPopupButton = document.getElementById('saveEditPopup');
    const modalOverlay = document.getElementById('modalOverlay');

    var data = {
        "alarms": [
            {
                "time": 54060,
                "amount": 5,
                "days": 1,
                "duration": 1800,
                "between": 10
            },
            {
                "time": 54120,
                "amount": 5,
                "days": 1100010,
                "duration": 1800,
                "between": 10
            }
        ]
    };

    var settings = {

    };

    const alarmOverlayMainDiv = document.createElement('div');
    alarmOverlayMainDiv.className = 'alarmoverlaymaindiv';
    container.appendChild(alarmOverlayMainDiv);
        
    data.alarms.forEach((alarm, index) => {
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

            fetch('/removeAlarm', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: 'time=' + encodeURIComponent(alarm.time),
            })
            .then(response => {
                if (response.ok) {
                     console.log('Success:', response);
                     window.location.reload();
                }else{
                    throw new Error('Network response was not ok.');
                }
            })
            .catch(error => alert(error));

        });
        alarmItemButtons.appendChild(removeButton);
    
    });

    //Popup close button code
    closeEditPopupButton.addEventListener('click', function(){

        modalOverlay.style.display = 'none';
        popup.style.display = 'none';

    });

    //Popup save button code
    saveEditPopupButton.addEventListener('click', function(){

        popup.style.display = 'none';

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

});
