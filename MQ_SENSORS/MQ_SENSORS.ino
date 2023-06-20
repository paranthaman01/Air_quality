#define         MQ_PIN                       (0)     //define which analog input channel you are going to use
#define         RL_VALUE                     (5)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
 
/**********************Software Related Macros***********************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 

#define         GAS_LPG                      (0)
#define         GAS_METHANE                  (1)
#define         GAS_SMOKE                    (2)
 
/****************************Globals**********************************************/
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 
float           METHANECurve[3]  =  {3.0,0.12,-0.84};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15) 
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)                                                     
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms
/****************************MQ7*********************************************/
int gas_sensor = A2; //Sensor pin 
float m = -0.6527; //Slope 
float b = 1.30; //Y-Intercept 
float R0 = 21.91; //Sensor Resistance in fresh air from previous code

void setup()
{
  Serial.begin(9600);                               //UART setup, baudrate = 9600bps
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ_PIN);                       //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print(" Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");
  /*****************MQ7****************/
  Serial.begin(9600); //Baud rate 
  pinMode(gas_sensor, INPUT); //Set gas sensor as input
}
 
void loop()
{
   float sensor_volt; //Define variable for sensor voltage 
   float RS_gas; //Define variable for sensor resistance  
   float ratio; //Define variable for ratio
   int sensorValue = analogRead(gas_sensor); //Read analog values of sensor  Serial.print("LPG:"); MQ7
   sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage 

   RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas
   ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air
   double ppm_log = (log10(ratio)-b)/m; //Get ppm value in linear scale according to the the ratio value  
   double ppm = pow(10, ppm_log); //Convert ppm value to log scale
   Serial.print("LPG:");
   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) );
   Serial.print( "ppm" );
   Serial.print("  ,  ");   
   Serial.print("METHANE:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_METHANE) );
   Serial.print( "ppm" );
   Serial.print("  ,  ");   
   Serial.print("SMOKE:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) );
   Serial.print( "ppm" );
   Serial.print("  ,  ");
   Serial.print("RAW VALUE OF CO = ");
   Serial.println(sensorValue);
   Serial.print("  ,  ");  
   Serial.print("Our desired CO PPM = ");
   Serial.println(ppm);
   Serial.print("\n");
   delay(2000);
}

float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}
 
/*************************** MQCalibration **************************************/

float MQCalibration(int mq_pin)
{
  int i;
  float val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
 
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
 
  return val; 
}
/***************************  MQRead *******************************************/
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
  } else if ( gas_id == GAS_METHANE ) {
     return MQGetPercentage(rs_ro_ratio,METHANECurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }     
  return 0;
}
 
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
