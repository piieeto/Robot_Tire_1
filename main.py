import time
from serial_interface import SerialInterface
from gamepad_control import get_joystick_command

def console_mode(esp):
	print("Konsole-Modus aktiviert. Mit STRG+C beenden.")
	try:
		while True:
			cmd = input("Befehl eingeben: ").strip().upper()
			esp.send_command(cmd)
			time.sleep(0.05) # Kurz warten auf Antwort
			
			response = esp.read_response()
			if response:
				print("ESP32 Antwort: ", response)

	except KeyboardInterrupt:
		print("\nonsole-Modus beendet.")

def gamepad_mode(esp):
	print("Gamepad-Modus aktiviert. Mit STRG+C beenden.")
	try:
		while True:
			cmd = get_joystick_command()
			if cmd:
				esp.send_command(cmd)
				time.sleep(0.05)
				response = esp.read_response()
				if response:
					print("ESP32 Antwort: ", response)
			time.sleep(0.05)
	except KeyboardInterrupt:
		print("\nonsole-Modus beendet.")

def main():
	esp = SerialInterface()
	
	try:
		while True:
			print("\n--- Steuerungsmodus wÃ¤hlen ---")
			print("1: Konsoleingabe")
			print("2: Gamepad-Steuerung")
			print("q: Beenden")
			auswahl = input("Modus auswÃ¤hlen: ").strip().lower()
            
			if auswahl == "1":
				console_mode(esp)
			elif auswahl == "2":
				gamepad_mode(esp)
			elif auswahl == "q":
				break
			else:
				print("UngÃ¼ltige Auswahl.")

	finally:
		esp.close()

if __name__ == "__main__":
	main()
