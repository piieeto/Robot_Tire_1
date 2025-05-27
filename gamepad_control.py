from evdev import InputDevice, ecodes, list_devices
import select

# Steuerungs-Konstanten
DEADZONE = 8000            # Totzone fÃ¼r Joystick (verhindert Zittern)
CENTER = 0                 # Mittelwert laut evtest (Stick-Ruhe)
AXIS_LIMIT = 32767         # Maximaler Stick-Ausschlag

# Initialisierung
def find_gamepad():
    devices = [InputDevice(path) for path in list_devices()]
    for dev in devices:
        name = dev.name
        # Nur echte Gamepads, keine IMU-Sensoren
        if any(x in name for x in ['Gamepad', '8BitDo', 'Xbox', 'Sony', 'Pro']) and 'IMU' not in name:
            dev.grab()
            print(f"? Gamepad erkannt: {name} ({dev.path})")
            return dev
    raise RuntimeError("Kein Gamepad gefunden!")

gamepad = find_gamepad()

# Aktuelle Werte
current_y = 0
current_x = 0
last_command = None

def get_joystick_command():
    global current_y, current_x, last_command

    # Neue Events prÃ¼fen (nicht blockierend)
    r, _, _ = select.select([gamepad], [], [], 0.01)
    if r:
        for event in gamepad.read():
            # Y-Achse (vor/zurÃ¼ck)
            if event.type == ecodes.EV_ABS and event.code == ecodes.ABS_Y:
                current_y = event.value
            # X-Achse (links/rechts)
            elif event.type == ecodes.EV_ABS and event.code == ecodes.ABS_X:
                current_x = event.value
            # Taste B / BTN_EAST als Not-Stopp
            elif event.type == ecodes.EV_KEY and event.code == ecodes.BTN_EAST and event.value == 1:
                if last_command != "M0,0":
                    last_command = "M0,0"
                    return "M0,0"

    # Achsen invertieren: Stick oben = positiv, rechts = positiv
    y = -current_y
    x = current_x

    # Totzone prÃ¼fen
    if abs(y) < DEADZONE and abs(x) < DEADZONE:
        if last_command != "M0,0":
            last_command = "M0,0"
            return "M0,0"
        return None

    # Werte normieren (von -32767 bis +32767 ? -255 bis +255)
    norm_y = max(min(y / AXIS_LIMIT, 1.0), -1.0)
    norm_x = max(min(x / AXIS_LIMIT, 1.0), -1.0)

    # Differentialberechnung (MOTOR LINKS & RECHTS)
    left_speed = norm_y + norm_x
    right_speed = norm_y - norm_x

    # Begrenzen auf -1.0 bis +1.0
    left_speed = max(min(left_speed, 1.0), -1.0)
    right_speed = max(min(right_speed, 1.0), -1.0)

    # In PWM umrechnen (int -255 bis +255)
    pwm_left = int(left_speed * 255)
    pwm_right = int(right_speed * 255)

    cmd = f"M{pwm_left},{pwm_right}"

    if cmd != last_command:
        last_command = cmd
        return cmd

    return None
