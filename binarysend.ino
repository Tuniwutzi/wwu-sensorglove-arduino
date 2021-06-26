int analogPins[] = { 0,1,2,3,4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
int val;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long StartTime = millis();

  for (int i = 0; i < (sizeof(analogPins) / sizeof(int)); i++)
  {
    val = analogRead(analogPins[i]);    // read the input pin
    /*Serial.print(analogPins[i]);
    Serial.print(": ");*/
    //Serial.println(val);

    Serial.write((char*)&val, sizeof(val));
  }

  unsigned long CurrentTime = millis();
  unsigned int ElapsedTime = CurrentTime - StartTime;

  //Serial.write((char*)&ElapsedTime, sizeof(ElapsedTime));

  Serial.write('|');

  if (ElapsedTime < 1000)
    delay(1000-ElapsedTime);
}
