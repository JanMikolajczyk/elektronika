// Definicje pinów
// Piezo sensor
const int knockSensor = 0;
// Przycisk programowania
const int programSwitch = 2;
// Silnik otwierania zamka
const int lockMotor = 3;
const int redLED = 4; // LED
const int greenLED = 5; // LED
// Stałe sterowania detektorem
const int threshold = 900;
const int rejectValue = 25;
const int averageRejectValue = 15;
const int knockFadeTime = 150;
const int lockTurnTime = 650;
const int maximumKnocks = 20;
const int knockComplete = 1200;
// Zmienne
int secretCode[maximumKnocks] = {
100, 100, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// Gdy ktoś puka ta macierz wypełnia się mierzonymi opóźnieniami
int knockReadings[maximumKnocks];
// Ostatni pomiar sensora uderzenia
int knockSensorValue = 0;
// Flaga do zapamiętywania naciśnięcia przycisku programowania
int programButtonPressed = false;
void setup() {
 pinMode(lockMotor, OUTPUT);
 pinMode(redLED, OUTPUT);
 pinMode(greenLED, OUTPUT);
 pinMode(programSwitch, INPUT);
 Serial.begin(9600); 
 Serial.println("Program start.");
 // Zapalenie zielonej diody LED
 digitalWrite(greenLED, HIGH);
}
void loop() {
 // Nasłuchiwanie pikania
 knockSensorValue = analogRead(knockSensor);
 // Naciśnięcie przycisku programowania
 if (digitalRead(programSwitch)==HIGH){
 // Jeśli tak, to zapisujemy stan programu
 programButtonPressed = true;
 // i zapalamy czerwoną diodę, aby to zasygnalizować
 digitalWrite(redLED, HIGH);
 } else {
 programButtonPressed = false;
 digitalWrite(redLED, LOW);
 }
 // Wykryto pukanie
 if (knockSensorValue >=threshold){
 // Dekodowanie pukania
 listenToSecretKnock();
 }
}
// Zapisuje czas uderzeń
void listenToSecretKnock(){
 Serial.println("knock starting");
 int i = 0;
 // resetuje wartości w macierzy
 for (i=0;i<maximumKnocks;i++){
 knockReadings[i]=0;
 }
 // Licznik
 int currentKnockNumber=0;
 // Start pomiaru czasu
 int startTime=millis();
 int now=millis();
 digitalWrite(greenLED, LOW);
 if (programButtonPressed==true){
 digitalWrite(redLED, LOW);
 }
 // Czekamy po uderzeniu pewien czas
 delay(knockFadeTime);
 digitalWrite(greenLED, HIGH);
 if (programButtonPressed==true){
 digitalWrite(redLED, HIGH);
 }
 do {
 // Czekamy na kolejne uderzenie lub timeout
 knockSensorValue = analogRead(knockSensor);
 // Jeśli wystapiło kolejne uderzenie
 if (knockSensorValue >=threshold){
 Serial.println("knock.");
// zapisuujemy czas,
now=millis();
knockReadings[currentKnockNumber] = now-startTime;
// zwiększamy licznik
currentKnockNumber ++;
// i resetujemy pomiar czasu
startTime=now;
digitalWrite(greenLED, LOW);
if (programButtonPressed==true){
digitalWrite(redLED, LOW);
}
// Czekamy po uderzeniu pewien czas
delay(knockFadeTime);
digitalWrite(greenLED, HIGH);
if (programButtonPressed==true){
digitalWrite(redLED, HIGH);
}
 }
 now=millis();
 // Czy nastąpił timeout lub ditarliśmy do limitu uderzeń?
 } while ((now-startTime < knockComplete)
 && (currentKnockNumber < maximumKnocks));
 // sprawdzanie poprawności sekwencji uderzeń
 if (programButtonPressed==false){

  if (validateKnock() == true){
triggerDoorUnlock();
 } else {
 Serial.println("Secret knock failed.");
digitalWrite(greenLED, LOW);
for (i=0;i<4;i++){
digitalWrite(redLED, HIGH);
delay(100);
digitalWrite(redLED, LOW);
delay(100);
}
digitalWrite(greenLED, HIGH);
 }
 } else {
 validateKnock();
 Serial.println("New lock stored.");
 digitalWrite(redLED, LOW);
 digitalWrite(greenLED, HIGH);
 for (i=0;i<3;i++){
delay(100);
digitalWrite(redLED, HIGH);
digitalWrite(greenLED, LOW);
delay(100);
digitalWrite(redLED, LOW);
digitalWrite(greenLED, HIGH);
 }
 }
}
// Otwiera drzwi silnikiem
void triggerDoorUnlock(){
 Serial.println("Door unlocked!");
 int i=0;
 // Obracanie silnikiem
 digitalWrite(lockMotor, HIGH);
 digitalWrite(greenLED, HIGH);
 // ...przez jakiś czas...
 delay (lockTurnTime);
 // po czym wyłączamy silnik
 digitalWrite(lockMotor, LOW);
 for (i=0; i < 5; i++){
digitalWrite(greenLED, LOW);
delay(100);
digitalWrite(greenLED, HIGH);
delay(100);
 }
}
// Sprawdza, czy nasza sekwencja zgadza się z zapisaną
boolean validateKnock(){
 int i=0;
 int currentKnockCount = 0;
 int secretKnockCount = 0;
 int maxKnockInterval = 0;
 for (i=0;i<maximumKnocks;i++){
 if (knockReadings[i] > 0){
currentKnockCount++;
 }
 if (secretCode[i] > 0){
secretKnockCount++;
 }
 if (knockReadings[i] > maxKnockInterval){
maxKnockInterval = knockReadings[i];
 }
 }
 if (programButtonPressed==true){
for (i=0;i<maximumKnocks;i++){
secretCode[i]= map(
knockReadings[i],0, maxKnockInterval, 0, 100);
}
digitalWrite(greenLED, LOW);
digitalWrite(redLED, LOW);
delay(1000);
digitalWrite(greenLED, HIGH);
digitalWrite(redLED, HIGH);
delay(50);
for (i = 0; i < maximumKnocks ; i++){
digitalWrite(greenLED, LOW);
digitalWrite(redLED, LOW);
if (secretCode[i] > 0){
delay(map(secretCode[i],0, 100, 0, maxKnockInterval));
digitalWrite(greenLED, HIGH);
digitalWrite(redLED, HIGH);
}
delay(50);
}
return false;
 }
 // Czy liczba uderzeń się zgadza?
 if (currentKnockCount != secretKnockCount){
 return false;
 }
 int totaltimeDifferences=0;
 int timeDiff=0;
 // Normalizacja czasu
 for (i=0;i<maximumKnocks;i++){
 knockReadings[i]= map(
knockReadings[i],0, maxKnockInterval, 0, 100);
 // Różnice czasu uderzeń
 timeDiff = abs(knockReadings[i]-secretCode[i]);
 // Pojedynczy czas zbyt odbiega od zapisanego
 if (timeDiff > rejectValue){
return false;
 }
 totaltimeDifferences += timeDiff;
 }
 if (totaltimeDifferences/secretKnockCount>averageRejectValue){
 // Średnia rozbieżność z zapisanym czasem
 return false;
 }
 return true;
}