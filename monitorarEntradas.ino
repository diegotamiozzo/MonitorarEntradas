/* Diego Tamiozzo
   21/07/2023
   RTC com Sd Card 
   10 entradas, salvando data e hora Ligado e Desligado
   Salvando em dois arquivos
   Modificado formato de data e hora, leituras no LCD somente
*/

#include <SD.h>
#include <Wire.h>
#include <RTClib.h>

// LCD
#include <LiquidCrystal_I2C.h>
#define I2C_ADDR 0x27  // Endereço I2C do display LCD
#define LCD_COLS 16    // Número de colunas do display LCD
#define LCD_ROWS 2     // Número de linhas do display LCD

const int chipSelect = 53;
const int buttonPin[10] = { 22, 24, 26, 28, 30, 32, 34, 36, 38, 40 };
char daysOfTheWeek[7][12] = { "Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado" };
bool stop[10] = { false };
bool acionado[10] = { false };
DateTime horaAcionamento[10];
DateTime horaFinal[10];
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);
File dataFile;

void setup() {
  pinMode(10, OUTPUT);
  for (int i = 0; i < 10; i++) {
    pinMode(buttonPin[i], INPUT);
  }

  //LCD
  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.clear();
    lcd.setCursor((LCD_COLS - 16) / 2, 0);  // Centraliza "Não foi possível encontrar RTC"
    lcd.print("NAO FOI POSSIVEL");
    lcd.setCursor((LCD_COLS - 14) / 2, 1);
    lcd.print("ENCONTRAR RTC");
    while (1)
      ;
  }

  if (!rtc.isrunning()) {
    lcd.clear();
    lcd.setCursor((LCD_COLS - 13) / 2, 0);  // Centraliza "O RTC NÃO está em execução"
    lcd.print("O RTC NAO ESTA");
    lcd.setCursor((LCD_COLS - 14) / 2, 1);
    lcd.print("EM EXECUCAO");
    while (1)
      ;
  }

  lcd.clear();

  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.setCursor((LCD_COLS - 7) / 2, 0);  // Centraliza "ERRO SD"
    lcd.print("ERRO SD");
    while (1)
      ;
  }

  lcd.setCursor((LCD_COLS - 13) / 2, 0);  // Centraliza "Inicializando"
  lcd.print("INICIALIZANDO");
  lcd.setCursor((LCD_COLS - 9) / 2, 1);  // Centraliza "Status OK"
  lcd.print("STATUS OK");
  delay(3000);

  // Imprime Data e Hora, verificar se estão corretos
  DateTime now = rtc.now();

  lcd.clear();  // Limpa o LCD antes de exibir a nova informação

  lcd.print("DATA: ");
  if (now.day() < 10) {
    lcd.print("0");
  }
  lcd.print(now.day(), DEC);
  lcd.print('/');
  if (now.month() < 10) {
    lcd.print("0");
  }
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.year(), DEC);
  lcd.print(" ");
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);

  lcd.setCursor(0, 1);
  lcd.print("HORA: ");
  if (now.hour() < 10) {
    lcd.print("0");
  }
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  if (now.minute() < 10) {
    lcd.print("0");
  }
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  if (now.second() < 10) {
    lcd.print("0");
  }
  lcd.print(now.second(), DEC);
  delay(3000);
}

void loop() {
  for (int i = 0; i < 10; i++) {
    if (digitalRead(buttonPin[i]) == HIGH && !stop[i]) {
      if (!acionado[i]) {
        DateTime now = rtc.now();
        horaAcionamento[i] = now;  // Armazena a hora de acionamento

        dataFile = SD.open("dados01.txt", FILE_WRITE);  // Abre o arquivo de dados
        if (dataFile) {
          dataFile.print("Entrada ");
          dataFile.print(i + 1);
          dataFile.print(" Acionada,");
          dataFile.print(now.day() < 10 ? "0" : "");
          dataFile.print(now.day(), DEC);
          dataFile.print('/');
          dataFile.print(now.month() < 10 ? "0" : "");
          dataFile.print(now.month(), DEC);
          dataFile.print('/');
          dataFile.print(now.year(), DEC);
          dataFile.print(',');

          dataFile.print(daysOfTheWeek[now.dayOfTheWeek()]);

          // Adiciona a hora inicial
          dataFile.print(',');
          dataFile.print(now.hour() < 10 ? "0" : "");
          dataFile.print(now.hour(), DEC);
          dataFile.print(':');
          dataFile.print(now.minute() < 10 ? "0" : "");
          dataFile.print(now.minute(), DEC);
          dataFile.print(':');
          dataFile.print(now.second() < 10 ? "0" : "");
          dataFile.print(now.second(), DEC);
          dataFile.println();
          dataFile.close();

          lcd.clear();
          lcd.setCursor((LCD_COLS - 12) / 2, 0);  // Centraliza "ENTRADA"
          lcd.print("ENTRADA ");
          lcd.print(i + 1);
          lcd.print(" ON");
          delay(1000);
          lcd.clear();
          lcd.setCursor((LCD_COLS - 12) / 2, 0);  // Centraliza "DADOS SALVOS"
          lcd.print("DADOS SALVOS");
          delay(2000);
          lcd.clear();
          lcd.setCursor((LCD_COLS - 10) / 2, 0);  // Centraliza "MONITORANDO"
          lcd.print("MONITORANDO");
          lcd.setCursor((LCD_COLS - 13) / 2, 1);  // Centraliza "SISTEMA"
          lcd.print("...SISTEMA...");

          acionado[i] = true;  // Define a entrada como acionada
        } else {
          lcd.clear();
          lcd.print("ERRO");
          lcd.setCursor(0, 1);
          lcd.print("para gravação");
        }
      }
    } else if (digitalRead(buttonPin[i]) == LOW) {
      if (acionado[i]) {
        DateTime now = rtc.now();
        horaFinal[i] = now;  // Armazena a hora final

        dataFile = SD.open("dados02.txt", FILE_WRITE);  // Abre o arquivo de dados
        if (dataFile) {
          dataFile.print("Entrada ");
          dataFile.print(i + 1);
          dataFile.print(" Desligada,");
          dataFile.print(now.day() < 10 ? "0" : "");
          dataFile.print(now.day(), DEC);
          dataFile.print('/');
          dataFile.print(now.month() < 10 ? "0" : "");
          dataFile.print(now.month(), DEC);
          dataFile.print('/');
          dataFile.print(now.year(), DEC);
          dataFile.print(',');

          dataFile.print(daysOfTheWeek[now.dayOfTheWeek()]);

          // Adiciona a hora final
          dataFile.print(',');
          dataFile.print(now.hour() < 10 ? "0" : "");
          dataFile.print(now.hour(), DEC);
          dataFile.print(':');
          dataFile.print(now.minute() < 10 ? "0" : "");
          dataFile.print(now.minute(), DEC);
          dataFile.print(':');
          dataFile.print(now.second() < 10 ? "0" : "");
          dataFile.print(now.second(), DEC);
          dataFile.println();
          dataFile.close();

          lcd.clear();
          lcd.setCursor((LCD_COLS - 12) / 2, 0);  // Centraliza "ENTRADA"
          lcd.print("ENTRADA ");
          lcd.print(i + 1);
          lcd.print(" OFF");
          delay(1000);
          lcd.clear();
          lcd.setCursor((LCD_COLS - 12) / 2, 0);  // Centraliza "DADOS SALVOS"
          lcd.print("DADOS SALVOS");
          delay(2000);
          lcd.clear();
          lcd.setCursor((LCD_COLS - 10) / 2, 0);  // Centraliza "MONITORANDO"
          lcd.print("MONITORANDO");
          lcd.setCursor((LCD_COLS - 13) / 2, 1);  // Centraliza "SISTEMA"
          lcd.print("...SISTEMA...");

          acionado[i] = false;  // Redefine a entrada como não acionada
          stop[i] = false;      // Permite acionamento novamente quando o botão for pressionado novamente
        } else {
          lcd.clear();
          lcd.print("ERRO");
          lcd.setCursor(0, 1);
          lcd.print("para gravação");
        }
      }
    }
    stop[i] = (digitalRead(buttonPin[i]) == LOW);
  }
}
