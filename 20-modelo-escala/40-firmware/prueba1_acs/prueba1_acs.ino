void setup() {
  pinMode(18, OUTPUT);
}

void loop() {
  Serial.begin(9600);
  int var=0;
  var = analogRead(18);
  Serial.println(var);

  
}