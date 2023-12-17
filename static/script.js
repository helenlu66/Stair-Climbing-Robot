esp32_cam_ip = "";
esp32_ip = ""


function goForward() {
    console.log("Forward button clicked");
    humanDetected = checkHumanDetected();
    if (!humanDetected) {
        // Create JSON data with the "command" field
        const jsonData = { command: "forward" };

        // Send a POST request to a specified endpoint (replace with your desired endpoint)
        fetch(esp32_ip, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(jsonData),
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .catch(error => {
            console.error('There was a problem with the forward operation:', error);
        });
    }
}

function stop() {
    console.log("Stop button clicked");
}

function goBack() {
    console.log("Backward button clicked");
}

function checkHumanDetected() {

    fetch(esp32_cam_ip, { method: 'GET' })
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            //return response.json();
        })
        .then(data => {
            console.log(data);
            if (data && data.humanDetected === true) {
                showPopup(msg); 
                return true;
            } 
            return false;
        })
        .catch(error => {
            console.error('There was a problem with the fetch operation:', error);
        });

}

function showPopup(message) {
    const popup = document.createElement('div');
    popup.className = 'popup';

    const closeButton = document.createElement('span');
    closeButton.className = 'popup-close';
    closeButton.innerHTML = '&times;';
    closeButton.addEventListener('click', () => {
        document.body.removeChild(popup);
    });
    popup.appendChild(closeButton);

    const messageElement = document.createElement('p');
    messageElement.textContent = message;
    popup.appendChild(messageElement);

    const okButton = document.createElement('button');
    okButton.className = 'popup-button';
    okButton.textContent = 'OK';
    okButton.addEventListener('click', () => {
        document.body.removeChild(popup);
    });
    popup.appendChild(okButton);

    document.body.appendChild(popup);

    popup.style.display = 'block';
}
