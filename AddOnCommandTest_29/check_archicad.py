import requests
import json
import time

def check_alive():
    ports = [19723, 65050, 65110]
    for port in ports:
        url = f"http://localhost:{port}/v1/project"
        try:
            response = requests.get(url, timeout=2)
            if response.status_code == 200:
                print(f"Archicad API is ALIVE on port {port}: {response.json()}")
                return port
            else:
                print(f"Port {port} responded with status {response.status_code}")
        except Exception:
            pass
    return None

def check_addon(port):
    url = f"http://localhost:{port}/v1/addons/ZoneHelper/Ping"
    payload = {}
    try:
        response = requests.post(url, json=payload, timeout=5)
        if response.status_code == 200:
            print(f"Add-on Ping RESPONSE: {response.json()}")
            return True
        else:
            print(f"Add-on Ping FAILED with status {response.status_code}: {response.text}")
    except Exception as e:
        print(f"Add-on Ping not reachable: {e}")
    return False

if __name__ == "__main__":
    print("Checking Archicad status...")
    port = check_alive()
    if port:
        print(f"Now checking Add-on status on port {port}...")
        check_addon(port)
    else:
        print("Archicad API is not active yet.")
