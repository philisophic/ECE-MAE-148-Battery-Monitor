void setup() {
	Serial.begin(9600);
	
	analogReadResolution(16);
}

void loop() {

	double sum = 0;
	for (int i = 0; i < 300; i++) {
		sum += analogRead(A2) * 3.3 / 65535 * 4/3;
		delay(1);
	}		

	Serial.print("A2: ");
	Serial.print(analogRead(A2));
	Serial.print(" | ");
	Serial.println(sum / 300);
	delay(5000);
}