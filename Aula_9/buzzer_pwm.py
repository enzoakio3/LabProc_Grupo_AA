import RPi.GPIO as GPIO
import time

BUZZER_PIN = 4

GPIO.setmode(GPIO.BCM)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

buzzer = GPIO.PWM(BUZZER_PIN, 440)

try:

    buzzer.start(50)

    notas = [
        (65, "Do"),
        (73, "Mi"),
        (98, "Sol"),
        (110, "La"),
        (130, "Do agudo"),
        (220, "La agudo")
    ]

    for frequencia, nome in notas:

        print(f"{nome} ({frequencia} Hz)")

        buzzer.ChangeFrequency(frequencia)

        time.sleep(2)

finally:

    buzzer.stop()
    GPIO.cleanup()

    print("Teste finalizado.")