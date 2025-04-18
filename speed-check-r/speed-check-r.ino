int PHOTORESLIMIT = 60;

void setup() {
  Serial.begin(9600); // Запускаем Serial монитор
}

bool oneOnA0(){
  int sum = 0, del = 10;
  for (int i = 0; i < del ; ++i){
    sum += analogRead(A0);
    // int j;
    // j = analogRead(A0);
    // sum += j;
    // Serial.println(j);
    __asm__ __volatile__("nop"); // подождать цикл
  } 
  if(sum / del > PHOTORESLIMIT) return true;
  else return false; 
}

int counter = 0;
bool lastOne = false;
void loop() {

  bool isOneOnA0 = oneOnA0();
  // Serial.println(isOneOnA0);

  if(lastOne != isOneOnA0){
    counter++;
    lastOne = isOneOnA0;
  }
  // Serial.println(counter);
  Serial.print(lastOne);
  Serial.print(" ");
  Serial.print(isOneOnA0);
  Serial.print(" ");
  Serial.println(counter);

}