int i = 5;
int delay_var = 10;
void setup() {
  // Настроим пин D2 как выход
  // uint i = 8;
  pinMode(2, OUTPUT);
}

void loop() {
  if(i > 0){
    --i;
    digitalWrite(2, HIGH);
    delay(delay_var); 
  
    digitalWrite(2, LOW);
    delay(delay_var);
  }
}