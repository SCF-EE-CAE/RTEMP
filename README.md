# RTENV - Versão: v1

Código do módulo de temperatura do projeto.

- _/aux_ contém arquivos auxiliares que foram utilizados para montar o programa
- _/main_ contém o código principal do módulo, que é efeitavemente executado por ele. O código está presente em _main.ino_ e todos os parâmetros de configuação, como nome e senha do WiFi estão no arquivo _config.h_

Algumas bibliotecas são utilizadas no código e podem ser baixadas pela IDE do Arduino. Suas referências podem ser encontradas a seguir

- [Adafruit_MQTT_Client](https://github.com/adafruit/Adafruit_MQTT_Library): Abstração do Cliente MQTT
- [NTPClient](https://github.com/arduino-libraries/NTPClient): Abstração do Cliente NTP, para sincronização do tempo
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library): Para a leitura do sensor de temperatura DS18B20    
- [ArduinoLog](https://github.com/thijse/Arduino-Log/): Framework de geração de log (Conexão Serial)
- [ArduinoOTA](https://www.arduino.cc/reference/en/libraries/arduinoota/): Para possibilitar flash do firmware via conexão WiFi

Para enviar o código para ESP-01, também é necessário adicionar uma URL de Board Manager nas preferências da IDE:

https://arduino.esp8266.com/stable/package_esp8266com_index.json

E selecionar a placa "Generic ESP8266 Module"
