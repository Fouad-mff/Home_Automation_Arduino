#include <dht.h>

#define         dht_dpin A0 //Input Analog pour le capteur DHT11.

#define         SPEAKER 3
#define         VENTILO 4
#define         LED_HUM 7
#define         LED_TEMP 8
#define         LED_GAS 12

#define         MQ_PIN                       (1)     
#define         RL_VALUE                     (20)    
#define         RO_CLEAN_AIR_FACTOR          (10)   
                                                   
#define         CALIBARAION_SAMPLE_TIMES     (50)
#define         CALIBRATION_SAMPLE_INTERVAL  (300)
#define         READ_SAMPLE_INTERVAL         (50)
#define         READ_SAMPLE_TIMES            (5)

#define         GAS_LPG                      (0)
#define         GAS_CH4                      (1)
 

float           LPGCurve[3]  =  {3,   0,  -0.4};
float           CH4Curve[3]  =  {3.3, 0,  -0.38};
float           Ro           =  10;
float           DhtHum, DhtTemp, Lpg, Ch4;
int a,val;
dht DHT;

float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

float MQCalibration(int mq_pin)
{
  int i;
  float val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                  
 
  val = val/RO_CLEAN_AIR_FACTOR;                        
 
  return val; 
}

float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}

int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CH4 ) {
      return MQGetPercentage(rs_ro_ratio,CH4Curve);
  }    
 
  return 0;
}

int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10, (((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

void alarmGas(int i) {
  float sinVal;
  int toneVal;
  
  for (int x=0; x<180; x++) {    
    sinVal = (sin(x*(3.1412/180)));   
    toneVal = 2000+(int(sinVal*1000));
    tone(SPEAKER, toneVal);
  } 

  switch (i) {
    case 1:
      delay(400);
      noTone(SPEAKER);
      delay(200);
      break;
    case 2:
      delay(250);
      noTone(SPEAKER);
      break;
    case 3:
      delay(90);
      noTone(SPEAKER);
      break;
  }
}

void alarmTemp(int delayms){
  analogWrite(SPEAKER, 20);      
  delay(delayms);         
  analogWrite(SPEAKER, 0);       
  delay(delayms);         
} 

void checkForHumAlarm(){
  if(DhtHum >= 50)
    digitalWrite(LED_HUM, HIGH);
  else
    digitalWrite(LED_HUM, LOW);
} 

void checkForTempAlarm(){
  if(DhtTemp >= 40 && DhtTemp < 45){
    alarmTemp(100);
    digitalWrite(LED_TEMP, HIGH);
    digitalWrite(VENTILO, LOW);
  }
  else if(DhtTemp >= 45 && DhtTemp < 50){
    alarmTemp(500);
    digitalWrite(LED_TEMP, HIGH);
    digitalWrite(VENTILO, LOW);
  }
  else if(DhtTemp >= 50){
    alarmTemp(1000);
    digitalWrite(LED_TEMP, HIGH);
    digitalWrite(VENTILO, LOW);    
  }
  else{ 
    digitalWrite(LED_TEMP, LOW);
    digitalWrite(VENTILO, HIGH);
  }
} 

bool checkForLpgAlarm(){
  if(int(Lpg) >= 1 && int(Lpg) < 5){
    alarmGas(1);
    digitalWrite(LED_GAS, HIGH);
    digitalWrite(VENTILO, LOW);
    return true;
  }
  else if(int(Lpg) >= 5 && int(Lpg) < 10){
    alarmGas(2);
    digitalWrite(LED_GAS, HIGH);
    digitalWrite(VENTILO, LOW);
    return true;
  }
  else if(int(Lpg) >= 10){
    alarmGas(3);
    digitalWrite(LED_GAS, HIGH);
    digitalWrite(VENTILO, LOW);
    return true;
  }
  else{
    digitalWrite(LED_GAS, LOW);
    digitalWrite(VENTILO, HIGH);
    return false;
  }
} 

void checkForCh4Alarm(){
  if(int(Ch4) >= 1 && int(Ch4) < 5){
    alarmGas(1);
    digitalWrite(LED_GAS, HIGH);
    digitalWrite(VENTILO, LOW);
  }
  else if(int(Ch4) >= 5 && int(Ch4) < 10){
    alarmGas(2);
    digitalWrite(LED_GAS, HIGH);
    digitalWrite(VENTILO, LOW);
  }
  else if(int(Ch4) >= 10){
    alarmGas(3);
    digitalWrite(LED_GAS, HIGH);
    digitalWrite(VENTILO, LOW);
  }
  else{
    digitalWrite(LED_GAS, LOW);
    digitalWrite(VENTILO, HIGH);
  }
} 

void checkForAlarm(){
  checkForHumAlarm();
  checkForTempAlarm();
  if(!checkForLpgAlarm())
  checkForCh4Alarm();
} 

void ledControl(){
  if(Serial.available()) {
      char c = Serial.read();
      switch (c){
        case '1':
          digitalWrite(11, HIGH);
          break;
        case '2':
          digitalWrite(10, HIGH);
          break;
        case '3':
          digitalWrite(9, HIGH);
          break;
        case '4':
          digitalWrite(6, HIGH);
          break;
        case '5':
          digitalWrite(5, HIGH);
          break;
         case 'a':
          digitalWrite(11, LOW);
          break;
        case 'b':
          digitalWrite(10, LOW);
          break;
        case 'c':
          digitalWrite(9, LOW);
          break;
        case 'd':
          digitalWrite(6, LOW);
          break;
        case 'e':
          digitalWrite(5, LOW);
          break;        
      }
  }  
}

void setup(){ 
  Serial.begin(9600);
  for(int i=2;i<=13;i++){
  pinMode(i,OUTPUT);
  }
  digitalWrite(VENTILO, HIGH);
  Serial.print("Calibrage de capteur MQ-6...\n");                
  Ro = MQCalibration(MQ_PIN);                                       
  Serial.print("Calibrage termine...\n"); 
  Serial.print("Ro = ");
  Serial.print(Ro);
  Serial.print(" kohm");
  Serial.print("\n");  
}

void loop(){
    
  DHT.read11(dht_dpin);
  
  DhtHum = DHT.humidity;
  DhtTemp = DHT.temperature;
  Lpg = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG);
  Ch4 = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CH4);

    ledControl();
    
    Serial.print("\n");
    Serial.print("Taux de l'humidite :       ");
    Serial.print(DhtHum);
    Serial.println(" %");
    Serial.print("Valeur de temperature :    ");
    Serial.print(DhtTemp); 
    Serial.println(" C  ");
    
    Serial.print("Valeur de gaz LPG :         "); 
    Serial.print(Lpg);
    Serial.println( " ppm" );   
    Serial.print("Valeur de gaz Methane CH4 : "); 
    Serial.print(Ch4);
    Serial.print( " ppm" );
    Serial.print("\n");

    checkForAlarm();
   
}
