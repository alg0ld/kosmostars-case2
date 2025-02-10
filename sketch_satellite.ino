#include <AffineCipher.h> // Библиотека для шифрования
#include <DHT.h> // Библиотека для работы с DHT11
#include <SPI.h> // Библиотека для работы с шиной SPI 
#include <nRF24L01.h> // Файл конфигурации для библиотеки RF24 
#include <RF24.h> // Библиотека для работы с модулем NRF24L01 

#define PIN_CE 9 // Номер пина Arduino, к которому подключен вывод CE радиомодуля 
#define PIN_CSN 10 // Номер пина Arduino, к которому подключен вывод CSN радиомодуля RF24 
#define PIN_HMD A0 // Пин для модуля влажности почвы
#define PIN_DHT 2 // Пин для модуля DHT
#define PIN_PS A1 // Пин для модуля освещенности

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN
AffineCipher cipher(10, 7, 3); // Инициализация шифра с параметрами m, a, b

DHT dht11(PIN_DHT, DHT11); // Инициализация датчика DHT11

const byte address[6] = "00001";

void setup() { 
  Serial.begin(9600); // Инициализация серийного порта для отладки 
  radio.begin(); // Инициализация радиомодуля NRF24L01
  dht11.begin(); // Инициализация датчика DHT11
  radio.setChannel(5); // Обмен данными будет вестись на пятом канале (2,405 ГГц) 
  radio.setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек 
  radio.setPALevel(RF24_PA_HIGH); // Выбираем высокую мощность передатчика (-6dBm)
  radio.openWritingPipe(address); // Открываем трубу с уникальным ID
  radio.stopListening(); // Устанавливаем в режим передачи
} 

void loop() { 
  delay(2000); // Задержка в 2 секунды
  
  // Чтение данных с датчиков
  float HMA = dht11.readHumidity();
  float temp = dht11.readTemperature();
  int HMD = analogRead(PIN_HMD);
  int PS = analogRead(PIN_PS);

  // Вывод значений для отладки
  Serial.print("HMA: "); Serial.println(HMA);
  Serial.print("temp: "); Serial.println(temp);
  Serial.print("HMD: "); Serial.println(HMD);
  Serial.print("PS: "); Serial.println(PS);
  
  // Шифрование данных
  String HMAc = cipher.encryptString(String(HMA));
  String tempc = cipher.encryptString(String(temp));
  String HMDc = cipher.encryptString(String(HMD));
  String PSc = cipher.encryptString(String(PS));

  // Проверка на ошибки чтения
  if (HMA < 0 || temp < 0 || HMD < 0 || PS < 0) {
    const char text[] = "Error of reading values";
    radio.write(&text, sizeof(text));
    delay(1000);
  } else {
    // Создание массива для передачи
    char data[100]; // Увеличьте размер массива, если необходимо
    sprintf(data, "%s,%s,%s,%s", HMAc.c_str(), tempc.c_str(), HMDc.c_str(), PSc.c_str());
    
    // Отправка данных
    radio.write(&data, sizeof(data));
  }
}
