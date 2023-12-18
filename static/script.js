esp32_endpoint = "esp32";
esp32_cam_endpoint = "esp32_cam"


function goForward() {
    console.log("Forward button clicked");
    sendCommand('forward')
    .then(responseData => {
        console.log('Received response data:', responseData);
        // display a popup if the robot is going backward instead of forward due to IMU angle being too large
        if (responseData['executed action'] === 'backward'){
            showPopup("Angle larger than safety threshold.")
        }
    })
    .catch(error => {
        console.error('Error:', error);
    });
}

function stop() {
    console.log("Stop button clicked");
    sendCommand('stop')
    .then(responseData => {
        console.log('Received response data:', responseData);
    })
    .catch(error => {
        console.error('Error:', error);
    });
}

function goBack() {
    console.log("Backward button clicked");
    sendCommand('backward')
    .then(responseData => {
        console.log('Received response data:', responseData);
    })
    .catch(error => {
        console.error('Error:', error);
    });
}

function sendCommand(command) {
    // Create JSON data with the "command" field
    const jsonData = { command: command };

    // Send a POST request to a specified endpoint (replace with your desired endpoint)
    return fetch(esp32_endpoint, {
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
        console.error('There was a problem with the operation:', error);
    });
}

function takePic() {
    return fetch(esp32_cam_endpoint, { method: 'GET' })
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.blob();
        })
        .then(imageBlob => {
            const imageUrl = URL.createObjectURL(imageBlob);
            showImagePopup(imageUrl);
        })
        .catch(error => {
            console.error('There was a problem with the fetch operation:', error);
        });
}

function showImagePopup(imageUrl) {
    const popup = document.createElement('div');
    popup.className = 'popup';

    const closeButton = document.createElement('span');
    closeButton.className = 'popup-close';
    closeButton.innerHTML = '&times;';
    closeButton.addEventListener('click', () => {
        document.body.removeChild(popup);
    });
    popup.appendChild(closeButton);

    const imageElement = document.createElement('img');
    imageElement.src = imageUrl;
    popup.appendChild(imageElement);

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

function checkHumanDetected() {
    
    return fetch(esp32_cam_endpoint, { method: 'GET' })
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
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
