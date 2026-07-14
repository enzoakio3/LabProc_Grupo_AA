import time
import RPi.GPIO as GPIO

SERVO_PIN = 18
BUZZER_PIN = 4

BUTTON_UP = 16
BUTTON_DOWN = 26

GPIO.setmode(GPIO.BCM)

GPIO.setup(SERVO_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

GPIO.setup(BUTTON_UP, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(BUTTON_DOWN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

servo = GPIO.PWM(SERVO_PIN, 50)
buzzer = GPIO.PWM(BUZZER_PIN, 262)

servo.start(0)
buzzer.start(0)

bpm = 60

SERVO_LEFT = 5.5
SERVO_RIGHT = 9.5

position = SERVO_LEFT

next_beat = time.monotonic()


def beat(position):

    servo.ChangeDutyCycle(position)

    buzzer.ChangeDutyCycle(50)

    time.sleep(0.08)

    buzzer.ChangeDutyCycle(0)

    time.sleep(0.12)

    servo.ChangeDutyCycle(0)


try:

    while True:

        if GPIO.input(BUTTON_UP) == GPIO.LOW:

            bpm = min(bpm + 5, 180)

            print(f"BPM = {bpm}")

            time.sleep(0.2)

        if GPIO.input(BUTTON_DOWN) == GPIO.LOW:

            bpm = max(bpm - 5, 30)

            print(f"BPM = {bpm}")

            time.sleep(0.2)

        period = 60.0 / bpm

        now = time.monotonic()

        if now >= next_beat:

            beat(position)

            if position == SERVO_LEFT:
                position = SERVO_RIGHT
            else:
                position = SERVO_LEFT

            next_beat += period

except KeyboardInterrupt:

    pass

finally:

    servo.stop()
    buzzer.stop()

    GPIO.cleanup()