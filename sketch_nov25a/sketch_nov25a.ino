int analogPins[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
int sendInterval = 16;
bool calibrate = true;

struct ValueBoundary
{
  int Min;
  int Max;

  int Range;
};

ValueBoundary ValueBoundaries[sizeof(analogPins) / sizeof(int)];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  resetCalibration();
}
void resetCalibration()
{
  memset(ValueBoundaries, 0, sizeof(ValueBoundaries));
  for (int i = 0; i < sizeof(ValueBoundaries)/sizeof(ValueBoundary); i++)
    ValueBoundaries[i].Min = 1023;
}

inline unsigned char readCalibrated(int pinIndex)
{
  int val = analogRead(analogPins[pinIndex]);
  ValueBoundary* boundary = ValueBoundaries + pinIndex;

  if (calibrate)
  {
    if (val > boundary->Max)
    {
      boundary->Max = val;
      boundary->Range = boundary->Max - boundary->Min;
    }
    else if (val < boundary->Min)
    {
      boundary->Min = val;
      boundary->Range = boundary->Max - boundary->Min;
    }
      if (boundary->Max <= boundary->Min)
        return 0;
  }
  else
  {
    if (val >= boundary->Max)
      return 254;
    else if (val <= boundary->Min)
      return 0;
  }

  //Wir wollen eine Skala von 0-254 (255 ist Trennzeichen), also mit x := val - boundary->Min und range := boundary->Range: 254 * (x/range)
  //Um Fliesskommarechnung zu vermeiden: (254*x)/range (braucht keine Kommarechnung, da wir den Nachkommaanteil im Endergebnis sowieso wegwerfen)
  //Optimierung: 254x = (256-2)x = 256x-2x = 2^(8)*x - 2^(1) * x entspricht: x um 8 Bit nach links verschoben minus x um 1 Bit nach links verschoben (shift ist schneller als Multiplikation - ob das mit der noetigen Subtraktion wirklich Performance bringt muessten wir testen)
  
  unsigned long x = (unsigned long)(val - boundary->Min); //Grosser Wertebereich fuer x, um es nach Multiplikation mit 256 noch aufnehmen zu koennen
  
  //return (unsigned char)(((x << 8) - (x << 1)) / boundary->Range);
  return (unsigned char)((254*x) / boundary->Range);
}
void loop() {
  // put your main code here, to run repeatedly:
  unsigned long StartTime = millis();
  unsigned char writeBuffer[(sizeof(analogPins) / sizeof(int)) + 1];
  writeBuffer[0] = 0xff;

  for (int i = 0; i < (sizeof(analogPins) / sizeof(int)); i++)
  {
    writeBuffer[i + 1] = readCalibrated(i);
    //writeBuffer[i] = (unsigned char)analogRead(analogPins[i]);
    
    //Serial.print(analogRead(analogPins[i]));
    //Serial.print(", ");
  }
  //Serial.println();
  
  Serial.write(writeBuffer, sizeof(writeBuffer));

  int readByte = Serial.read();
  /*if (readByte != -1)
  {
    calibrate = false;
  }*/
  if (readByte == 0x00)
    calibrate = false;
  else if (readByte == 0x01)
    calibrate = true;
  else if (readByte == 0x02)
    resetCalibration();

  unsigned long CurrentTime = millis();
  unsigned int ElapsedTime = CurrentTime - StartTime;

  if (ElapsedTime < sendInterval)
    delay(sendInterval-ElapsedTime-1);
}

