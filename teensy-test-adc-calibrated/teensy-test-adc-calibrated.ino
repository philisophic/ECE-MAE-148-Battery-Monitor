//Macros
#define TEST_MODE               1     // If test mode is enable then pin-3 simulates the jetson nano wake up signal
#if TEST_MODE
  #define WAKE_UP_SIGNAL_PIN    6
  #define TEST_DELAY            5*1000        // 2 seconds
#endif        
#define WAKE_UP_INTERRUPT_PIN   2             // Wake up interrupt pin 
#define NO_CELLS                3             // Number of cells in the LIPO
#define ONE_CELL_VOLTAGE        4.0000        // One cell voltage in LIPO 
#define ADC_REF_VOLTAGE         3.3000        // ADC reference voltage
#define ADC_RESOLUTION          65535.0000    // 16 bit resolution
#define NUM_ADC_SAMPLE          512.0000      // Number of ADC samples to average

// Global variables
bool    wake_up                        = true;
int     adc_teensy[NO_CELLS]           = {A9, A3, A6};                                                           // ADC channels to read voltages
float   cell_voltage_ratios[NO_CELLS]  = {(1.0000*ONE_CELL_VOLTAGE/3.0650), (2.0000*ONE_CELL_VOLTAGE/3.2850), (3.0000*ONE_CELL_VOLTAGE/3.0550)}; // Cells are in series on the LIPO
float   accumulate                     = 0.0000;
float   lipo_voltage[NO_CELLS]         = {0.0000, 0.0000, 0.0000};
char    jetson_log[6]                  = ""; 

// Setup the Teensy  
void setup() 
{
	Serial.begin(9600);                                          // Baud rate for serial communication USB
	analogReadResolution(16);                                    // ADC resolution = 16 bits
  pinMode(WAKE_UP_INTERRUPT_PIN, INPUT_PULLUP);                // Set the wake up pin as an input
  attachInterrupt(WAKE_UP_INTERRUPT_PIN, wake_up_isr, RISING); // Configuring wake up interrupt from Jetson Nano
#if TEST_MODE
  pinMode(WAKE_UP_SIGNAL_PIN, OUTPUT);                         // Set the wake up pin as an output for testing purposes
#endif     
}

// Main processing loop
void loop() 
{ 
  // Disable global interrupts
  noInterrupts();
    
  // Read ADC if Jetson Nano has requested
  if(wake_up)
  { 
#if TEST_MODE
  Serial.println("Process ADC and respond to jetson nano");                       
#endif      
    // j = 0 Read one cell voltage       - 4V
    // j = 1 Read two cells voltage      - 8V
    // j = 2 Read the three cell voltage - 12V
    for (int j=0; j<NO_CELLS; j++)
    {
      accumulate      = 0.0000;
      lipo_voltage[j] = 0.0000;
            
	    for (int i = 0; i < NUM_ADC_SAMPLE; i++) 
	    {  
        if(j == 2) // 12 V
        {    
          // Compensating the error hysteresis 
          if((accumulate/i)<11.0000)
          {
            accumulate = accumulate + ((analogRead(adc_teensy[j]) * ADC_REF_VOLTAGE / ADC_RESOLUTION * cell_voltage_ratios[j]) - 0.1500);
          }
          else
          {
            accumulate = accumulate + ((analogRead(adc_teensy[j]) * ADC_REF_VOLTAGE / ADC_RESOLUTION * cell_voltage_ratios[j]) - 0.0500);
          }
        }
        else // 8V and 4V
        {
          accumulate += analogRead(adc_teensy[j]) * ADC_REF_VOLTAGE / ADC_RESOLUTION * cell_voltage_ratios[j];  
        }
		    delay(2);
	    }
      lipo_voltage[j] = accumulate/NUM_ADC_SAMPLE; 
          
    }
    // Generate string of voltages for Jetson nano 
    sprintf(jetson_log, "%.2f,%.2f,%.2f\n", lipo_voltage[0], lipo_voltage[1], lipo_voltage[2]);
    Serial.print(jetson_log);
    
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
  analogReadResolution(16);
#if TEST_MODE
  Serial.println("Wake up interrupt");                       
#endif   
}
