import serial
import time

class SerialInterface:
    def __init__(self, port="/dev/ttyUSB0", baudrate=115200, timeout=1):
        # Verbindung Ã¶ffnen
        self.ser = serial.Serial(port, baudrate, timeout=timeout)
        time.sleep(2) # Warten, bis ESP32b ereit ist
    
    def send_command(self, command: str):
        """Sendet einen String-Befehl (z.?B. 'F100') und gibt die Antwort zurÃ¼ck"""
        if not command.endswith("\n"):
            command += "\n" # ESP erwartet Zeilenende
        self.ser.write(command.encode())
    
    def read_response(self):
        """Liest eine Antwortzeile vom ESP32"""
        if self.ser.in_waiting > 0:
            return self.ser.readline().decode().strip()
        return None
    
    def close(self):
        self.ser.close()
