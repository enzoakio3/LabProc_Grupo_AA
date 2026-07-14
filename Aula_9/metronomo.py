import RPi.GPIO as GPIO
import time

BUZZER_PIN = 27

GPIO.setmode(GPIO.BCM)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

buzzer = GPIO.PWM(BUZZER_PIN, 440)

try:

    buzzer.start(50)

    notas = [
        (262, "Do"),
        (330, "Mi"),
        (392, "Sol"),
        (440, "La"),
        (523, "Do agudo"),
        (880, "La agudo")
    ]

    for frequencia, nome in notas:

        print(f"{nome} ({frequencia} Hz)")

        buzzer.ChangeFrequency(frequencia)

        time.sleep(2)

finally:

    buzzer.stop()
    GPIO.cleanup()

    print("Teste finalizado.")