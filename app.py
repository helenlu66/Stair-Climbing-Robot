import requests
from flask import Flask, render_template, request, jsonify, Response
from flask_cors import CORS  # Import CORS

esp32_url = "http://10.247.137.97"
esp32_cam_url = "http://10.247.137.87/picture"

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

# Add a new endpoint to handle the forwarded request
@app.route('/esp32', methods=['POST'])
def forward_esp32_endpoint():
    # Forward the request to esp32
    # You may need to install the 'requests' library: pip install requests
    response = requests.post(url=esp32_url, json=request.json)

    # Return the response from the external server to the client
    return Response(response.content, status=response.status_code, content_type=response.headers['content-type'])

# Add a new endpoint to handle the forwarded request
@app.route('/esp32_cam', methods=['GET'])
def forward_esp32_cam_endpoint():
    # Forward the request to esp32
    # You may need to install the 'requests' library: pip install requests
    response = requests.get(url=esp32_cam_url)
    response = requests.get(url=esp32_cam_url)
    # Return the response from the external server to the client
    return Response(response.content, status=response.status_code, content_type=response.headers['content-type'])

if __name__ == '__main__':
    app.run(debug=True)
