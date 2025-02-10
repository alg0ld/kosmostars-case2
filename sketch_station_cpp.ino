#include <AffineCipher.h> // Библиотека для шифрования
#include <SPI.h> // Библиотека для работы с шиной SPI 
#include <nRF24L01.h> // Файл конфигурации для библиотеки RF24 
#include <RF24.h> // Библиотека для работы с модулем NRF24L01 
#include <LiquidCrystalRus.h> // Библиотека для работы с жк-дисплеем с интегрированными русскими символами

// Определение пинов для nRF24L01
#define PIN_CE 9
#define PIN_CSN 10

// Инициализация радиомодуля
RF24 radio(PIN_CE, PIN_CSN);

// Инициализация шифра
AffineCipher cipher(10, 7, 3);

// Адрес канала
const byte address[6] = "00001";

// Инициализация ЖК-дисплея
LiquidCrystalRus lcd(7, 6, 5, 4, 3, 2);

void setup() { 
  Serial.begin(9600); // Инициализация серийного порта для отладки 
  radio.begin(); // Инициализация радиомодуля NRF24L01
  radio.setChannel(5); // Обмен данными будет вестись на пятом канале (2,405 ГГц) 
  radio.setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек 
  radio.setPALevel(RF24_PA_HIGH); // Выбираем высокую мощность передатчика (-6dBm)
  radio.openReadingPipe(0, address); // Открываем трубу для приема
  radio.startListening(); // Устанавливаем в режим приема
  lcd.begin(20, 4);
}

void loop() { 
  lcd.clear();
  char data[100];
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    
    char HMAc[20], tempc[20], HMDc[20], PSc[20];
    if (sscanf(data, "%19[^,],%19[^,],%19[^,],%19[^,]", HMAc, tempc, HMDc, PSc) != 4) {
      lcd.clear();
      lcd.print("Error parsing");
      delay(500);
      return;
    }
    
    // Дешифрование данных
    float HMA = cipher.decryptString(String(HMAc)).toFloat();
    float temp = cipher.decryptString(String(tempc)).toFloat();
    int HMD = cipher.decryptString(String(HMDc)).toInt();
    int PS = cipher.decryptString(String(PSc)).toInt();

    // Очистка дисплея и вывод информации
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("[Влажность воздуха");
    lcd.setCursor(0,1);
    lcd.print("/");
    lcd.setCursor(0,2);
    lcd.print("Air humidity]:");
    lcd.setCursor(0,3);
    lcd.print(String(HMA));
    lcd.print("%");
    delay(1000);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("[Температура");
    lcd.setCursor(0,1);
    lcd.print("/");
    lcd.setCursor(0,2);
    lcd.print("Temperature]:");
    lcd.setCursor(0,3);
    lcd.print(String(temp));
    lcd.print(" C");

    delay(1000);
    
    // Вывод влажности почвы с анализом
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("[Влажность почвы");
    lcd.setCursor(0,1);
    lcd.print("/");
    lcd.setCursor(0,2);
    lcd.print("Soil moisture]:");
    lcd.setCursor(0,3);
    
    String humidityComment;
    if (0 < HMD < 400) {
      humidityComment = "NOT OK";
    }
    else if (HMD < 800) {
      humidityComment = "OK";
    }
    else {
      humidityComment = "NaN";
    }
    
    lcd.print(humidityComment);
    delay(1000);

    // Вывод освещенности
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("[Освещенность");
    lcd.setCursor(0,1);
    lcd.print("/");
    lcd.setCursor(0,2);
    lcd.print("Light]:");
    lcd.setCursor(0,3);
    
    String lightComment;
    if (200 < PS < 300) {
      lightComment = "LOW LIGHT";
    }
    else if (100 < PS < 200) {
      lightComment = "NORMAL LIGHT";
    }
    else if (PS < 100) {
      lightComment = "HIGH LIGHT";
    }
    else {
      lightComment = "Insuf. lighting";
    }
    
    lcd.print(lightComment);
    
    delay(1000); // Задержка перед следующим оzбновлением
  }
  else {
    lcd.print("No connect");
    delay(2000);
  }
}
