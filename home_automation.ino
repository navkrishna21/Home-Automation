#include <SoftwareSerial.h>  //Including the software serial library
#define DEBUG true
SoftwareSerial esp(2,3); // This will make the Arduino pin 2 as the RX pin and Arduino pin 3 as the TX. Software UART
/* So you have to connect the TX of the esp8266 to the pin 2 of the Arduino and the TX of the esp8266 to the pin 3 of the Arduino. This means that you need to connect the TX line from the esp to the Arduino's pin 2 */

int LDR = A0;      // LDR input at A0 pin.
int temp_pin= A1; //temperature sensor

int is_light_on = 0;
int is_fan_on = 0;

int threshold_val = 800; // Check your threshold and modify it.

int light_pin = 11;
int fan_pin = 10;

void setup() 
{
  Serial.begin(9600);   // Setting the baudrate to 9600
  esp.begin(115200);  // Set it according to your esp8266 baudrate. Different wifi-modules  have different baud rates.
  
  pinMode(temp_pin, INPUT);
  pinMode(LDR, INPUT);
  
  pinMode(11,OUTPUT);   // Setting the pin 11 as the output pin.
  digitalWrite(11,0); // Making it low.
  
  pinMode(10, OUTPUT);
  digitalWrite(10,0);
    
  sendData("AT+RST\r\n",2000,DEBUG);            //This command will reset module to default
  sendData("AT+CWMODE=2\r\n",1000,DEBUG);       // This will configure the mode as access point
  sendData("AT+CIFSR\r\n",1000,DEBUG);          // This will get ip address and will show it
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG);       // This will configure the ESP8266 for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // This will set the server on port 80
}

void loop() 
{
  
  if(esp.available()) // Checking that whether the esp8266 is sending a message or not (Software UART Data)
  { 
    if(esp.find("+IPD,"))
    { 
      
      delay(1000);        // Waiting for 1 sec
       
      int connectionId = esp.read()-48;   // Subtracting 48 from the character to get the number.
      esp.find("pin=");                   // Advancing the cursor to the "pin="
      int pinNumber = (esp.read()-48)*10; // Getting the first number which is pin 13
      pinNumber += (esp.read()-48);  // This will get the second number. For example, if the pin number is 11 then the 2nd number will be 1 and then add it to the first number

      Serial.print(pinNumber);
            
      if(pinNumber == 11)
        is_light_on = !is_light_on;
      else if(pinNumber == 10)
        is_fan_on = !is_fan_on;
        
      // The following commands will close the connection 
      String closeCommand = "AT+CIPCLOSE="; 
      closeCommand+=connectionId; 
      closeCommand+="\r\n";
      sendData(closeCommand,1000,DEBUG);     // Sending the data to the ESP8266 to close the command
    } 
  }
   
  int LDRReading = analogRead(LDR); //LDR reading
  control_light(LDRReading);
  
  double temp = (5.0 * analogRead(temp_pin) * 100.0) / 1024; //temperature reading
  control_fan(temp);
   
}

void control_light(int LDRReading)
{
  
  int inten = map(LDRReading, 0, 1023, 0, 255);  // to store the value for LED Brightness by mapping LDR reading in the range of 0 to 255
  control_light(LDRReading);
  
  if(is_light_on == 0)
  {
    digitalWrite(light_pin, 0);
  }
  else 
  {
    analogWrite(light_pin, inten);
  }  
}

void control_fan(double temp)
{
  int power; //to store fan's power
  
  if(temp > 28) 
  {
    power = 100;
  }
  else if(temp > 25)
  {
    power = 50;
  }
  else 
  {
    power = 50;
  }
  
  if(is_fan_on == 0)
  {
    digitalWrite(fan_pin, 0);
  }
  else 
  {
    analogWrite(fan_pin, power);
  } 
}

String sendData(String command, const int timeout, boolean debug) // Function to send the data to the esp8266
{
  String response = ""; 
  esp.print(command);           // Send the command to the ESP8266
  long int time = millis();
  while( (time+timeout) > millis()) // ESP8266 will wait for some time for the data to receive
  {
    while(esp.available())      // Checking whether ESP8266 has received the data or not
    {
      char c = esp.read();      // Read the next character.
      response+=c;                  // Storing the response from the ESP8266
    }
  }
  if(debug)
  { 
    Serial.print(response);         // Printing the response of the ESP8266 on the serial monitor.
  }
  return response;
} 
