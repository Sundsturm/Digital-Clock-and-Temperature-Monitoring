#include <avr/io.h>
#include <avr/interrupt.h>

#define PIN1 9
#define PIN2 10
#define PIN3 11
#define PIN4 12
#define PINA 2
#define PINB 3
#define PINC 4
#define PIND 5
#define PINE 6
#define PINF 7
#define PING 8
#define PINUDOT 13 // Dot bawah
#define PINDDOT 15 // Dot atas
#define PINLED 14  // LED pada digital pin 14 atau A0
#define PINBUTTON A2
#define PINLM35 A3

volatile int secondCount = 0;
volatile int minuteCount = 59;
volatile int hourCount = 10;
volatile bool ledState = false;
volatile int halfSecondCounter = 0;
volatile bool showClock = true;  // Mulai dengan mode jam
int digits[4] = {0, 0, 0, 0};

float lastTemperature = -999.0;  // Suhu terakhir yang ditampilkan
int lastHour = -1, lastMinute = -1, lastSecond = -1;  // Waktu terakhir yang ditampilkan
bool temperatureUpdated = false; // Untuk memastikan suhu diperbarui setiap detik

ISR(TIMER1_COMPA_vect) {
  if (halfSecondCounter == 2) {
    halfSecondCounter = 0;
    secondCount++;
    temperatureUpdated = false; // Reset flag pembaruan suhu
    if (secondCount == 60) {
      secondCount = 0;
      minuteCount++;
      if (minuteCount == 60) {
        minuteCount = 0;
        hourCount++;
        if (hourCount == 24) hourCount = 0;
      }
    }
  }
  halfSecondCounter++;
  ledState = !ledState;  // Toggle LED setiap detik
  digitalWrite(PINLED, ledState);
}

void updateDigitsClock() {
  digits[0] = hourCount / 10;
  digits[1] = hourCount % 10;
  digits[2] = minuteCount / 10;
  digits[3] = minuteCount % 10;

  // Hanya cetak waktu jika ada perubahan
  if (hourCount != lastHour || minuteCount != lastMinute || secondCount != lastSecond) {
    Serial.print("Clock: ");
    Serial.print(hourCount);
    Serial.print(":");
    Serial.print(minuteCount);
    Serial.print(":");
    Serial.println(secondCount);

    lastHour = hourCount;
    lastMinute = minuteCount;
    lastSecond = secondCount;
  }
}

void updateDigitsTemp(float temp) {
  int tempInt = int(temp * 100);  // Konversi ke integer dengan 1 desimal
  digits[0] = tempInt / 1000 % 10;
  digits[1] = tempInt / 100 % 10;
  digits[2] = tempInt / 10 % 10;
  digits[3] = tempInt % 10;

  // Hanya cetak suhu jika ada perubahan signifikan dan setiap detik
  if (!temperatureUpdated && abs(temp - lastTemperature) > 0.1) {
    Serial.print("Temperature (Celsius): ");
    Serial.println(temp, 2);  // Cetak suhu dengan 2 desimal
    lastTemperature = temp;
    temperatureUpdated = true; // Tandai bahwa suhu sudah diperbarui untuk detik ini
  }
}

void displayDigit(int digit, int value, bool dotOn = false) {
  // Matikan semua digit
  digitalWrite(PIN1, HIGH);
  digitalWrite(PIN2, HIGH);
  digitalWrite(PIN3, HIGH);
  digitalWrite(PIN4, HIGH);

  // Pilih digit yang akan menyala
  switch (digit) {
    case 1: digitalWrite(PIN1, LOW); break;
    case 2: digitalWrite(PIN2, LOW); break;
    case 3: digitalWrite(PIN3, LOW); break;
    case 4: digitalWrite(PIN4, LOW); break;
  }

  // Tampilkan angka pada digit
  switch (value) {
    case 0: digitalWrite(PINA, HIGH); digitalWrite(PINB, HIGH); digitalWrite(PINC, HIGH); digitalWrite(PIND, HIGH); digitalWrite(PINE, HIGH); digitalWrite(PINF, HIGH); digitalWrite(PING, LOW); break;
    case 1: digitalWrite(PINA, LOW); digitalWrite(PINB, HIGH); digitalWrite(PINC, HIGH); digitalWrite(PIND, LOW); digitalWrite(PINE, LOW); digitalWrite(PINF, LOW); digitalWrite(PING, LOW); break;
    case 2: digitalWrite(PINA, HIGH); digitalWrite(PINB, HIGH); digitalWrite(PINC, LOW); digitalWrite(PIND, HIGH); digitalWrite(PINE, HIGH); digitalWrite(PINF, LOW); digitalWrite(PING, HIGH); break;
    case 3: digitalWrite(PINA, HIGH); digitalWrite(PINB, HIGH); digitalWrite(PINC, HIGH); digitalWrite(PIND, HIGH); digitalWrite(PINE, LOW); digitalWrite(PINF, LOW); digitalWrite(PING, HIGH); break;
    case 4: digitalWrite(PINA, LOW); digitalWrite(PINB, HIGH); digitalWrite(PINC, HIGH); digitalWrite(PIND, LOW); digitalWrite(PINE, LOW); digitalWrite(PINF, HIGH); digitalWrite(PING, HIGH); break;
    case 5: digitalWrite(PINA, HIGH); digitalWrite(PINB, LOW); digitalWrite(PINC, HIGH); digitalWrite(PIND, HIGH); digitalWrite(PINE, LOW); digitalWrite(PINF, HIGH); digitalWrite(PING, HIGH); break;
    case 6: digitalWrite(PINA, HIGH); digitalWrite(PINB, LOW); digitalWrite(PINC, HIGH); digitalWrite(PIND, HIGH); digitalWrite(PINE, HIGH); digitalWrite(PINF, HIGH); digitalWrite(PING, HIGH); break;
    case 7: digitalWrite(PINA, HIGH); digitalWrite(PINB, HIGH); digitalWrite(PINC, HIGH); digitalWrite(PIND, LOW); digitalWrite(PINE, LOW); digitalWrite(PINF, LOW); digitalWrite(PING, LOW); break;
    case 8: digitalWrite(PINA, HIGH); digitalWrite(PINB, HIGH); digitalWrite(PINC, HIGH); digitalWrite(PIND, HIGH); digitalWrite(PINE, HIGH); digitalWrite(PINF, HIGH); digitalWrite(PING, HIGH); break;
    case 9: digitalWrite(PINA, HIGH); digitalWrite(PINB, HIGH); digitalWrite(PINC, HIGH); digitalWrite(PIND, HIGH); digitalWrite(PINE, LOW); digitalWrite(PINF, HIGH); digitalWrite(PING, HIGH); break;
  }

  // Dot kontrol
  if (showClock) {
    // Dot atas dan bawah menyala pada mode jam digital
    digitalWrite(PINDDOT, HIGH);
    digitalWrite(PINUDOT, HIGH);
  } else {
    // Hanya dot atas menyala pada mode termometer
    digitalWrite(PINDDOT, dotOn ? HIGH : LOW);
    digitalWrite(PINUDOT, LOW);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN1, OUTPUT); pinMode(PIN2, OUTPUT); pinMode(PIN3, OUTPUT); pinMode(PIN4, OUTPUT);
  pinMode(PINA, OUTPUT); pinMode(PINB, OUTPUT); pinMode(PINC, OUTPUT);
  pinMode(PIND, OUTPUT); pinMode(PINE, OUTPUT); pinMode(PINF, OUTPUT); pinMode(PING, OUTPUT);
  pinMode(PINUDOT, OUTPUT); pinMode(PINLED, OUTPUT); pinMode(PINDDOT, OUTPUT);
  pinMode(PINBUTTON, INPUT_PULLUP);
  pinMode(PINLM35, INPUT);

  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12);
  OCR1A = 31250;
  TIMSK1 = (1 << OCIE1A);

  sei();  // Aktifkan global interrupt
}

void loop() {
  if (digitalRead(PINBUTTON) == LOW) {
    delay(50);
    if (digitalRead(PINBUTTON) == LOW) {
      showClock = !showClock;
      while (digitalRead(PINBUTTON) == LOW);
    }
  }

  if (showClock) {
    updateDigitsClock();
  } else {
    int adcValue = analogRead(PINLM35);
    float temp = adcValue * 4.887585533 / 10;
    updateDigitsTemp(temp);
  }

  for (int i = 1; i <= 4; i++) {
    displayDigit(i, digits[i - 1], (!showClock && i == 2));  // Dot aktif di digit kedua (termometer)
    delay(5);
  }
}
