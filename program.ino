// author: Jan Buenker

int analogPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
int sendInterval = 16;
bool calibrate = true;

struct ValueBoundary
{
  int Min;
  int Max;

  int Range;
};

ValueBoundary ValueBoundaries[sizeof(analogPins) / sizeof(int)];

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  resetCalibration();
}
void resetCalibration()
{
  memset(ValueBoundaries, 0, sizeof(ValueBoundaries));
  for (int i = 0; i < sizeof(ValueBoundaries) / sizeof(ValueBoundary); i++)
  {
    ValueBoundaries[i].Min = 1023;
  }
}

inline unsigned char readCalibrated(int pinIndex)
{
  int val = analogRead(analogPins[pinIndex]);
  ValueBoundary *boundary = ValueBoundaries + pinIndex;

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
    {
      return 0;
    }
  }
  else
  {
    if (val >= boundary->Max)
    {
      return 254;
    }
    else if (val <= boundary->Min)
    {
      return 0;
    }
  }

  // We want a scale of 0-254 (255 is used as a separator), so with x := val - boundary->Min and range := boundary->Range: 254 * (x/range)
  // To avoid issues with floating point arithmetic: (254*x) / range

  unsigned long x = (unsigned long)(val - boundary->Min); // Long for x to avoid overflow when multiplied by 254 (int is 2 bytes on arduino)

  return (unsigned char)((254 * x) / boundary->Range);
}
void loop()
{
  // put your main code here, to run repeatedly:
  unsigned long StartTime = millis();
  unsigned char writeBuffer[(sizeof(analogPins) / sizeof(int)) + 1];
  writeBuffer[0] = 0xff;

  for (int i = 0; i < (sizeof(analogPins) / sizeof(int)); i++)
  {
    writeBuffer[i + 1] = readCalibrated(i);
  }

  Serial.write(writeBuffer, sizeof(writeBuffer));

  int readByte = Serial.read();
  if (readByte == 0x00)
  {
    calibrate = false;
  }
  else if (readByte == 0x01)
  {
    calibrate = true;
  }
  else if (readByte == 0x02)
  {
    resetCalibration();
  }

  unsigned long CurrentTime = millis();
  unsigned int ElapsedTime = CurrentTime - StartTime;

  if (ElapsedTime < sendInterval)
  {
    delay(sendInterval - ElapsedTime - 1);
  }
}
