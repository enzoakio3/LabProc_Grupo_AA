import RPi.GPIO as GPIO
import time

BUZZER_PIN = 4

GPIO.setmode(GPIO.BCM)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

buzzer = GPIO.PWM(BUZZER_PIN, 440)

try:

    buzzer.start(50)

    notas = [
        (130, "Do"),
        (164, "Mi"),
        (196, "Sol"),
        (220, "La"),
        (261, "Do agudo"),
        (329, "Mi agudo")
    ]

    for frequencia, nome in notas:

        print(f"{nome} ({frequencia} Hz)")

        buzzer.ChangeFrequency(frequencia)

        time.sleep(2)

finally:

    buzzer.stop()
    GPIO.cleanup()

    print("Teste finalizado.")