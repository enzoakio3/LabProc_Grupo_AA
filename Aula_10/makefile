CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

WIRINGPI = -lwiringPi
WIRINGPI_DEV = -lwiringPiDev

KEYPAD_FILES = Keypad.cpp Key.cpp

all: \
	teste_lcd \
	teste_teclado \
	teste_teclado_lcd \
	teste_buzzer \
	teste_sensor \
	teste_atuador \
	teste_senha

teste_lcd: teste_lcd.cpp
	$(CXX) $(CXXFLAGS) teste_lcd.cpp \
	-o teste_lcd \
	$(WIRINGPI) $(WIRINGPI_DEV)

teste_teclado: teste_teclado.cpp $(KEYPAD_FILES)
	$(CXX) $(CXXFLAGS) teste_teclado.cpp \
	$(KEYPAD_FILES) \
	-o teste_teclado \
	$(WIRINGPI)

teste_teclado_lcd: teste_teclado_lcd.cpp $(KEYPAD_FILES)
	$(CXX) $(CXXFLAGS) teste_teclado_lcd.cpp \
	$(KEYPAD_FILES) \
	-o teste_teclado_lcd \
	$(WIRINGPI) $(WIRINGPI_DEV)

teste_buzzer: teste_buzzer.cpp
	$(CXX) $(CXXFLAGS) teste_buzzer.cpp \
	-o teste_buzzer \
	$(WIRINGPI)

teste_sensor: teste_sensor.cpp
	$(CXX) $(CXXFLAGS) teste_sensor.cpp \
	-o teste_sensor \
	$(WIRINGPI)

teste_atuador: teste_atuador.cpp
	$(CXX) $(CXXFLAGS) teste_atuador.cpp \
	-o teste_atuador \
	$(WIRINGPI)

teste_senha: teste_senha.cpp $(KEYPAD_FILES)
	$(CXX) $(CXXFLAGS) teste_senha.cpp \
	$(KEYPAD_FILES) \
	-o teste_senha \
	$(WIRINGPI)

clean:
	rm -f \
	teste_lcd \
	teste_teclado \
	teste_teclado_lcd \
	teste_buzzer \
	teste_sensor \
	teste_atuador \
	teste_senha

.PHONY: all clean