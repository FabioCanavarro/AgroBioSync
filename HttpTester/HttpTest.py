import requests
import random
import time

url = "https://agrobiosync.netlify.app/api/sensor-data"

for _ in range(109):
    temps = random.randint(20, 30)
    hum = random.randint(50, 60)
    mois = random.randint(40, 50)
    airtemp = random.randint(20, 30)
    payload = {
        "SoilTemp": temps,
        "SoilMoisture": mois,
        "AirTemp": airtemp,
        "Humidity": hum,
    }
    headers = {"Content-Type": "application/json"}

    response = requests.post(url, headers=headers, json=payload)
    print(f"Status Code: {response.status_code}")
    print(f"Response: {response.text}")
    time.sleep(0.4)
