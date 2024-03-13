document.addEventListener('DOMContentLoaded', function() {

    const logboxconsole = document.getElementById("logboxconsole");

    updateLogBox();

    function updateLogBox() {

        fetch('/sendLogs')
            .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
                return response.json();
            }) 
            .then(data => {
                data.forEach(log => {
                    const logboxentry = document.createElement('div');
                    logboxentry.className = "logboxentry";
                    logboxconsole.appendChild(logboxentry);

                    const logboxtimestamp = document.createElement('span');
                    logboxtimestamp.className = "logboxtimestamp";
                    const date = new Date(log.time * 1000);
                    const pad = (num) => num.toString().padStart(2, '0');
                    logboxtimestamp.textContent = `${pad(date.getMonth() + 1)}.${pad(date.getDate())} ${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
                    logboxentry.appendChild(logboxtimestamp);

                    const logboxlevel = document.createElement('span');
                    logboxlevel.className = "logboxlevel";
                    if(log.level == 3){
                        logboxlevel.textContent = "ERROR";
                        logboxlevel.style.color = "#b11d12";
                    }else if(log.level == 2){
                        logboxlevel.textContent = "WARN";
                        logboxlevel.style.color = "#ba9723";
                    }else{
                        logboxlevel.textContent = "INFO";
                    }
                    logboxentry.appendChild(logboxlevel);

                    const logboxmessage = document.createElement('span');
                    logboxmessage.className = "logboxmessage";
                    logboxmessage.textContent = log.message;
                    logboxentry.appendChild(logboxmessage);

                    logboxconsole.scrollTop = logboxconsole.scrollHeight;
                });
            })
            .catch(error => {
                console.error('There was a problem with the fetch operation:', error);
            });

    }

});