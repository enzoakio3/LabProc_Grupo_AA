import RPi.GPIO as GPIO
import time

LED_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)

pwm = GPIO.PWM(LED_PIN, 500)

try:
    pwm.start(0)

    duty_cycles = [0, 10, 25, 50, 75, 100]

    for duty in duty_cycles:
        print(f"Duty cycle: {duty}%")
        pwm.ChangeDutyCycle(duty)
        time.sleep(3)

finally:
    pwm.stop()
    GPIO.cleanup()