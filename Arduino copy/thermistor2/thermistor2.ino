int ThermistorPin = 5;
int VVo;
float RR1 = 10000;
float logRR2, RR2, TT, TTc, TTf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

void setup() {
Serial.begin(9600);
}

void loop() {

  VVo = analogRead(ThermistorPin);
  RR2 = RR1 * (1023.0 / (float)VVo - 1.0);
  logRR2 = log(RR2);
  TT = (1.0 / (c1 + c2*logRR2 + c3*logRR2*logRR2*logRR2));
  TTc = TT - 273.15;
  TTf = (TTc * 9.0)/ 5.0 + 32.0; 

  Serial.print("Temperature: "); 
  Serial.print(TTf);
  Serial.print(" F; ");
  Serial.print(TTc);
  Serial.println(" C");   

  delay(1000);
}

