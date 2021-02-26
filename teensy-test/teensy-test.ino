#define TEST_MODE             1     // If test mode is enable then pin-3 simulates the jetson nano wake up signal
#if TEST_MODE
  #define WAKE_UP_SIGNAL_PIN    3
  #define TEST_DELAY            2*1000 // 15 seconds
#endif        
#define WAKE_UP_INTERRUPT_PIN 0     // Wake up interrupt pin 
#define NO_CELLS              3     // Number of cells in the LIPO
#define ONE_CELL_VOLTAGE      4     // One cell voltage in LIPO 
#define ADC_REF_VOLTAGE       3.3   // ADC reference voltage
#define ADC_RESOLUTION        65535 // 16 bit resolution
#define NUM_ADC_SAMPLE        300   // Number of ADC samples to average

bool wake_up                    = true;
uint8_t adc_teensy[NO_CELLS]    = {A0, A3, A6};                                                           // ADC channels to read voltages
uint8_t cell_voltage[NO_CELLS]  = {(ONE_CELL_VOLTAGE/3), (2*ONE_CELL_VOLTAGE/3), (3*ONE_CELL_VOLTAGE/3)}; // Cells are in series on the LIPO
double  accumulate[NO_CELLS]    = {0, 0, 0};
float   lipo_voltage[NO_CELLS]  = {0, 0, 0};
char    jetson_log[6]           = ""; 
// Setup the Teensy  
void setup() 
{
	Serial.begin(9600);                                          // Baud rate for serial communication USB
	analogReadResolution(16);                                    // ADC resolution = 16 bits
  pinMode(WAKE_UP_INTERRUPT_PIN, INPUT_PULLUP);                // Set the wake up pin as an input
  attachInterrupt(WAKE_UP_INTERRUPT_PIN, wake_up_isr, RISING); // Configuring wake up interrupt from Jetson Nano
#if TEST_MODE
  pinMode(WAKE_UP_SIGNAL_PIN, OUTPUT);                         // Set the wake up pin as an input
#endif     
}

// Main processing loop
void loop() 
{
  Serial.println("Hello \r\n");
  // Disable global interrupts
  noInterrupts();
    
  // Read ADC if Jetson Nano has requested
  if(wake_up)
  { 
#if TEST_MODE
  Serial.println("Process ADC and respond to jetson nano \r\n");                       
#endif      
    // j = 0 Read one cell voltage       - 4V
    // j = 1 Read two cells voltage      - 8V
    // j = 2 Read the three cell voltage - 12V
    for (uint8_t j=0; j<NO_CELLS; j++)
    {
      accumulate[j]   = 0;
      lipo_voltage[j] = 0; 
	    for (uint8_t i = 0; i < NUM_ADC_SAMPLE; i++) 
	    {
	      accumulate[j] += analogRead(adc_teensy[j]) * ADC_REF_VOLTAGE / ADC_RESOLUTION * cell_voltage[j];
		    delay(1);
	    }
      lipo_voltage[j] = accumulate[j]/NUM_ADC_SAMPLE;
    }
    // Generate string of voltages for Jetson nano 
    sprintf(jetson_log, "%.2f,%.2f,%.2f\n", lipo_voltage[0], lipo_voltage[1], lipo_voltage[2]);
    Serial.println(jetson_log);
    
    // ready for next interrupt
    wake_up = false;
  }
  
  // Enable global interrupts
  interrupts();

#if TEST_MODE
  digitalWrite(WAKE_UP_SIGNAL_PIN, LOW); // Set the low
  delay(TEST_DELAY);                        
  digitalWrite(WAKE_UP_SIGNAL_PIN, HIGH); // Set the high
  delay(TEST_DELAY);                        
#endif 
}

// wake up ISR 
void wake_up_isr (void)
{
  wake_up = true;
#if TEST_MODE
  Serial.println("Wake up interrupt \r\n");                       
#endif   
}
