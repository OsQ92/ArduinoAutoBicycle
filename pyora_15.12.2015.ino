const double WHEELROUND = 2.2343;    //Diameter of the wheel
const int REFRESHRATE = 2000;        //Refreshrate 2000 milliseconds
const int SENSORPIN1 = 2;            //Pin number for sensor1
const int M1DIRPIN = 4;				//Motor 1 direction signal pin
const int M1STEPPIN = 5;			//Motor 1 step pin
const int M2DIRPIN = 6;				//Motor 2 direction signal pin
const int M2STEPPIN = 7;			//Motor 2 step pin
const int GEAR_STEP = 500;			//Number of steps for motor to accomplish 1 gear change (needs calibration)

/*GEARTABLE has the speeds in m/s for each gear*/
const float GEARTABLE[11] = {				
								13, 15.5, 
								18, 20.5, 	
								23.5, 26, 
								29.5, 35, 
								42.5, 48.5
							};		

int sensor1Data;                    //Pulses from frontwheel during refreshtime
int gear = 1;						//Gear
int backGearWheel = 7;				
int frontGearWheel = 3;		

double bikeSpeed;                    //Speed of the bike in m/s
double speedTable[3];                //Table of speed where we calculate averagespeed

unsigned long refTime;               //Time from last refresh
unsigned long timer;                 //Time of operation

void setup() {
  /*Defining pins and I/O*/
  pinMode(SENSORPIN1, INPUT);
  pinMode(M1DIRPIN,OUTPUT);
  pinMode(M1STEPPIN,OUTPUT);
  pinMode(M2DIRPIN,OUTPUT);
  pinMode(M2STEPPIN, OUTPUT);
  
  attachInterrupt(0, frontWheel, RISING);		//Rising signal from frontwheel launches interrupt
  Serial.begin(9600);
  sensor1Data = 0;
  bikeSpeed = 0;
  speedTable[3] = 0;							//Table to calculate moving average of speeds
  refTime = 0;									//Reference time 
}

void loop() {
  // put your main code here, to run repeatedly:
  timer = millis();

  if(timer - refTime >= REFRESHRATE) {
    calculateSpeed();

    printData();
  }
        checkSpeed();
}

//haistelee nopeutta ja tekee päätöksen, vaihtaako vaihdetta suuremmalle
void checkSpeed(){
  //käydään läpi nopeustaulukkoa verraten nykyiseen nopeuteen ja vaihteeseen
  //toimintaesimerkki: tarkistaa ykkösiteraatiolla, onko ykkösvaihteella ja jos on, niin pitääkö
  //vaihdetta nostaa. Samassa iteraatiossa tarkistaa tarvittaessa, onko kakkosvaihteella ja onko tarvis
  //laskea vaihdetta
  int i;
  for(i = 0; i < sizeof(GEARTABLE); i++){
    int checkToRaiseGear = i + 1; //tarkistetaan, tätä vasten, tarvitseeko vaihdetta nostaa
    int checkToLowerGear = i + 2; //tarkistetaan tätä vasten, tarvitseeko vaihdetta laskea

    //vaihteenvaihtofunktioissa on huomioitu, että vaihdetta ei voi vaihtaa yli 11 tai alle 1

    if(gear == checkToRaiseGear && bikeSpeed >= GEARTABLE[i]){
      setBiggerGear();
    }

    //vaihteenvaihtamisnopeus alaspäin on 1 km/h pienempi kuin ylösvaihtonopeus hystereesin välttämiseksi
    else if(gear == checkToLowerGear && bikeSpeed <= GEARTABLE[i] - 1){
      setLowerGear();
    }
  }
}

/*Increase wheel pulse count in refreshtime*/
void frontWheel() {
    sensor1Data++;
}

void setBiggerGear(){
  //tässä vaiheessa suoritetaan siirtymät vaihtamalla takaratasta isommalle
  if(gear < 11){
    switch(gear + 1){
      case 2:
      case 4:
      case 5:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      changeRightBackGear();
      break;
  
      case 3:
      case 6:
      changeLeftBackGear(); //Backgear is moved left when frontgear is changed
      changeRightFrontGear();
      break;
    }
    gear++;
  }
}

void setLowerGear(){
  if(gear > 1){
    switch(gear - 1){
      case 10:
      case 9:
      case 8:
      case 7:
      case 6:
      case 4:
      case 3:
      case 1:
      changeLeftBackGear();
      break;

      case 5:
      case 2:
      changeRightBackGear();
      changeLeftFrontGear();
    }
    gear--;
  }
}

/*Gear is decreased*/
void changeLeftBackGear(){
  spinMotor1(LOW, GEAR_STEP);
}
/*Gear is increased*/
void changeRightBackGear(){
  spinMotor1(HIGH, GEAR_STEP);
}

void changeLeftFrontGear(){
  spinMotor2(LOW, GEAR_STEP);
}

void changeRightFrontGear(){
  spinMotor2(HIGH, GEAR_STEP);
}

/*Calculate speed and reset frontwheel pulse count and referencetime*/
double calculateSpeed() {
  speedTable[1] = speedTable[0];
  speedTable[2] = speedTable[1];
  speedTable[0] = (sensor1Data*WHEELROUND);
  double temp;
  temp += speedTable[0];					
  temp += speedTable[1];
  temp += speedTable[2];
  bikeSpeed = ((temp/3)*3600)/refreshRate;		// result is m/s
  sensor1Data = 0;
  refTime = millis();
}
/*Data printing to serialchannel*/
void printData() {
  Serial.print("[Speed KM/H]: "); Serial.print(bikeSpeed);
  Serial.print(" Gear: ");Serial.print(gear);
  Serial.println();
}

/*Spins motor to given direction and number of steps*/
void spinMotor1(boolean dir, int steps) {
  digitalWrite(M1DIRPIN, dir);
  for(int j = 0; j <= steps; j++){
    digitalWrite(M1STEPPIN,LOW);
    digitalWrite(M1STEPPIN,HIGH);
    delay(1);
  }
}

void spinMotor2(boolean dir, int steps) {
  digitalWrite(M2DIRPIN, dir);
  for(int j = 0; j <= steps; j++){
    digitalWrite(M2STEPPIN,LOW);
    digitalWrite(M2STEPPIN,HIGH);
    delay(1);
  }
}



