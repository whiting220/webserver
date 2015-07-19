/* Web_Demo.pde -- sample code for Webduino server library */

/*
 * To use this demo,  enter one of the following USLs into your browser.
 * Replace "host" with the IP address assigned to the Arduino.
 *
 * http://host/
 * http://host/json
 *
 * This URL brings up a display of the values READ on digital pins 0-9
 * and analog pins 0-5.  This is done with a call to defaultCmd.
 * 
 * 
 * http://host/form
 *
 * This URL also brings up a display of the values READ on digital pins 0-9
 * and analog pins 0-5.  But it's done as a form,  by the "formCmd" function,
 * and the digital pins are shown as radio buttons you can change.
 * When you click the "Submit" button,  it does a POST that sets the
 * digital pins,  re-reads them,  and re-displays the form.
 * 
 */
#include <application.h>
//#include "freeMemory.h"
#define WEBDUINO_FAVICON_DATA ""
#define WEBDUINO_SERIAL_DEBUGGING 2
#include "WebServer.h"
//#include "sd-card-library.h"  pww 20/06/15
#include "SD.h"  // this is for the new SD card stuff as 20/06/15
#include "DallasTemperature.h"
#include "LiquidCrystal_I2C.h"

File myFile, dataFile;
boolean sdcard=false;
boolean bHeat=false;
boolean bWater=false;
char mydate [] ="Thursday 24/07/2014";
char mytime [] ="21:48";
char cOn [] = "<span style=\"color: rgb(51, 204, 0);\">On</span>";
char cOff [] = "<span style=\"color: rgb(255, 0, 0);\">Off</span>";
char* weekdays [] = { "Sun", "Mon", "Tues", "Wednes", "Thurs", "Fri", "Satur" };
int pwminute;
int pwsecond;
unsigned long pwnow;
boolean advance=false;
boolean advanceHW=false;
int period=3;  // off, on, once, twice, thrice.

int table[84]={6,30,8,30,12,00,12,00,16,30,22,30,     //monday     1    on and off times
               6,30,8,30,12,10,12,10,16,30,22,30,     //tuesday    2
               6,30,8,30,12,20,12,20,16,30,22,30,     //wednesday  3
               6,30,8,30,12,30,12,30,16,30,22,30,     //thursday   4
               6,30,8,30,12,40,12,40,16,30,22,30,     //friday     5
               8,30,10,30,12,50,12,50,16,30,22,30,     //saturday   6
               8,30,10,30,12,55,12,55,16,30,22,30} ;   //sunday     7
               
          
long times[6]= {6*60+30,8*60+30,12*60+0,12*60+0,16*60+30,22*60+30};
long * ptimes= &times[0];
int * ptable[7]= {&table[0],&table[12],&table[24],&table[36],&table[48],&table[60],&table[72]};
int * ptab;
int ahr,amin;
long amins;
int count;
/* Function prototypes -------------------------------------------------------*/
int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);
int tinkerAnalogRead(String pin);
int tinkerAnalogWrite(String command);

SYSTEM_MODE(AUTOMATIC);

char OBuff[64] = "Hello World";
int bindex =0;
int door = 0;
int tempdoor =0;
char Cmd[32]= "\0";



// SOFTWARE SPI pin configuration - modify as required
// The default pins are the same as HARDWARE SPI
const uint8_t chipSelect = A2;    // Also used for HARDWARE SPI setup
const uint8_t mosiPin = A5;
const uint8_t misoPin = A4;
const uint8_t clockPin = A3;
// no-cost stream operator as described at 
// http://sundial.org/arduino/?page_id=119
template<class T>
inline Print &operator <<(Print &obj, T arg)
{ obj.print(arg); return obj; }


#define PREFIX ""

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//LiquidCrystal_I2C lcd(0x38,20,4); // Set the LCD I2C address
LiquidCrystal_I2C *lcd;
 /*
unsigned int __heap_start;
    void *__brkval;

    // The free list structure as maintained by the 
     //avr-libc memory allocation routines.
     //
    struct __freelist {
      size_t sz;
      struct __freelist *nx;
    };

    // The head of the free list structure 
    struct __freelist *__flp;

    // MemoryFree library based on code posted here:
    // http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720/15
    // 
    // Extended by Matthew Murdoch to include walking of the free list.

    //#ifdef __cplusplus
        //extern "C" {
    //#endif

    int freeMemory();

    //#ifdef __cplusplus
        //}
    //#endif

    // Calculates the size of the free list 
    int freeListSize() {
      struct __freelist* current;
      int total = 0;

      for (current = __flp; current; current = current->nx) {
        total += 2; // Add two bytes for the memory block's header  
        total += (int) current->sz;
      }

      return total;
    }

    int freeMemory() {
      int free_memory;

      if ((int)__brkval == 0) {
        free_memory = ((int)&free_memory) - ((int)&__heap_start);
      } else {
        free_memory = ((int)&free_memory) - ((int)__brkval);
        free_memory += freeListSize();
      }
      return free_memory;
    }
 */





WebServer webserver(PREFIX, 80);

// commands are functions that get called by the webserver framework
// they can read any posted data from client, and they output to server

boolean checktimes()
{ 

pwminute = Time.minute();
amins = pwminute + 60*Time.hour();

//  Serial1.print("mode ");Serial.println(mode);
  Serial1.print("period = ");
  Serial1.println(period);
  
  if(advance){
  if ((amins==times[0])||(amins==times[1])||(amins==times[4])||(amins==times[5]))advance=false;
  if ((times[2]!=times[3])&&((amins==times[2])||(amins==times[3])))advance=false;
  
  }
 // else
  {if (period==0) return false;
  if (period==1) return true;
  if (period==2)
  {
    if ((amins <times[0] )||(amins>times[5]))return false;
    return true;  
  }
  if (period==3)
  {
    if (amins<times[0]) return advance; //false;//advance; //
    if (amins<times[1]) return !advance; //true;//!advance //
    if (amins<times[4]) return advance; //false;//advance
    if (amins<times[5]) return !advance; //true;//!advance
    return advance; //false;//advance   etc
  }
   if (period==4)
   {
    if(amins<times[0]) return advance; //false;
    if (amins<times[1]) return !advance; //true;
    if(amins<times[2]) return advance; //false;
    if (amins<times[3]) return !advance; //true;
    if (amins<times[4]) return advance; //false;
    if (amins<times[5]) return !advance; //true;
    return advance; //false;
   }
  } 
 return false;
 }









void jsonCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
 Serial1.println("\r\n json Cmd called\r\n");
  
  if (type == WebServer::POST)
  {
    server.httpFail();
    return;
  }

  //server.httpSuccess(false, "application/json");
  server.httpSuccess("application/json");
  
  if (type == WebServer::HEAD)
    return;

  int i;    
  server << "{ ";
  for (i = 0; i <= 7; ++i)
  {
    // ignore the pins we use to talk to the Ethernet chip
    int val = digitalRead(i);
    server << "\"d" << i << "\": " << "\""  << val << "\"" << ", ";
  }

  for (i = 0; i <= 1; ++i)
  {
    int val = analogRead(i);
    server << "\"a" << i << "\": " << "\""  << val << "\"" << ", ";
    
  }
	server << "\"result"  << "\": " << "\""  << OBuff << "\"" ;
	
	//if (i != 1)
    //  server << ", ";
  
  
  server << " }";
}

void outputPins(WebServer &server, WebServer::ConnectionType type, bool addControls = false)
{
 


 P(htmlHead) =
    "<!DOCTYPE HTML>"
	"<html>"
    "<head>"
    "<title>Sparks Web Server</title>"
    "<style \"color:red\" type=\"text/css\">"
    "BODY { font-family: sans-serif }"
    "H1 { font-size: 14pt; text-decoration: underline }"
    "P  { font-size: 10pt; }"
    "</style>"
    "</head>"
    "<body>"
	"<h1>Hello from Peter's Spark Core</h1>"
	"<p>A page from Peter's Spark Core Server. Page ";

  int i;
  server.httpSuccess();
  server.printP(htmlHead);
  server << count;
  if (addControls)
    server << "<form action='" PREFIX "/form' method='post'>";
if(sdcard) { server << "<h2>SD card good</h2><p>";}

  for (i = 0; i <= 7; ++i)
  {
    // ignore the pins we use to talk to the Ethernet chip
    int val = digitalRead(i);
    server << "Digital " << i << ": ";
    if (addControls)
    {
      char pinName[4];
      pinName[0] = 'd';
   //pww   itoa(i, pinName + 1, 10);
      server.radioButton(pinName, "1", "On", val);
      server << " ";
      server.radioButton(pinName, "0", "Off", !val);
    }
    else
      server << (val ? "HIGH" : "LOW");

    server << "<br/>";
  }

  server << "</p><h2>Analog Pins</h2><p>";
  for (i = 0; i <= 1; ++i)
  {
    int val = analogRead(i);
    server << "Analog " << i << ": " << val << "<br/>";
  }

  server << "</p>";

  if (addControls)
    server << "<input type='submit' value='Submit'/></form>";

  server << "</body></html>";
}

void sendSDpage(WebServer &server){
char inc;
char chdol ='$';
char chT ='T';char chW ='W';char chD ='D';char chH ='H';
char advHT []="anceHT";
char advHW []="anceHW"; 


delay(10);
server.httpSuccess(); 
// Initialize HARDWARE SPI with user defined chipSelect
delay(10);
  if(!sdcard)
  {
  Serial1.println("SD not ready!");
  if (SD.begin(chipSelect)) 
  {sdcard=true;
    Serial1.println("SD initialization done!");}
    else
  { Serial1.println("initialization failed.");
    sdcard=false;}
  }
  delay(10);
  myFile = SD.open("hc1.xht",FILE_READ);
  
  if (myFile) {
    Serial1.println("file opened");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	inc = myFile.read();
    if (inc ==chdol){
      inc = myFile.read();
      if (inc==chT){server << ((Time.hour() <10)?"0":"") <<  (Time.hour()+1) << ":" << ((Time.minute() <10)?"0":"") << Time.minute();}
      if (inc==chD){server << weekdays[Time.weekday()-1] << "day  " << ((Time.day() <10)?"0":"") << Time.day() << "/" << ((Time.month() <10)?"0":"") << Time.month() << "/" << Time.year();}
      if (inc==chH){if(bHeat){server << cOn;} else {server << cOff;} }
     // if (inc==chW){if(bWater){server << cOn;} else {server << cOff;} }
	 if (inc==chW){if(advanceHW){server << cOn;} else {server << cOff;} }
    }
    else    server << inc;
}
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial1.println("file did not open");
  } 
 
  
}

void SDCardWrite(String message)
{
  if(!sdcard)
  {
  Serial1.println("SD not ready!");
  if (SD.begin(chipSelect)) 
  {sdcard=true;
    Serial1.println("SD initialization done!");}
    else
  { Serial1.println("initialization failed.");
    sdcard=false;}
  }

// open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
   dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(message);
    dataFile.close();
    // print to the serial port too:
    Serial1.println(message);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial1.println("error opening datalog.txt");
  } 
}


void commandin(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
Serial1.println("\r\ncommandin called\r\n");

strcpy(Cmd,url_tail);

Serial1.println(Cmd);

if(Cmd[0]=='c' && Cmd[1]=='m' && Cmd[2]=='d' && Cmd[3]=='=' )  Serial1.println("\r\nwe have a command\r\n");
//server.httpSuccess("application/stringout");
if(Cmd[4]=='D' && Cmd[5]=='W' &&Cmd[6]=='D' )
{
	int pinNumber = Cmd[7] - '0';
	int value = Cmd[8] - '0';
	pinMode(pinNumber, OUTPUT);
	digitalWrite(pinNumber, value);
}


server.httpSuccess("commandin");
delay(10);


server << "{ ";
server << "\"result" << "\": " << "\"" <<Cmd<<"\"";

server << "} ";
}





void outputOBuff(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
Serial1.println("\r\noutputOBuff called\r\n");

//server.httpSuccess("application/stringout");
server.httpSuccess("stringout");
delay(10);

server << "{ ";
server << "\"result" << "\": " << "\"" <<OBuff<<"\"";

server << "} ";
}

void advanceHTCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{

count++;
advance=!advance;
delay(10);
SDCardWrite( Time.timeStr() );
bHeat = checktimes();
sendSDpage(server); 
}


void advanceHWCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{

count++;
advanceHW=!advanceHW;
P(htmlHead) =
    "<!DOCTYPE HTML>"
	"<html>"
    "<head>"
    "<title>Sparks Web Server</title>"
    "<style \"color:red\" type=\"text/css\">"
    "BODY { font-family: sans-serif }"
    "H1 { font-size: 14pt; text-decoration: underline }"
    "P  { font-size: 10pt; }"
    "</style>"
    "</head>"
    "<body>"
	"<h1>Hello from Peter's Spark Core</h1>"
	"<p>A page from Peter's Spark Core Server. Page ";

 
  server.httpSuccess();
  server.printP(htmlHead);
  server << count;
  
  
 server << "<h2>SD card </h2></p>";

delay(10);
  if(!sdcard)
  {
  Serial1.println("SD not ready!");
  if (SD.begin(chipSelect)) 
  {sdcard=true;
    Serial1.println("SD initialization done!");}
    else
  { Serial1.println("initialization failed.");
    sdcard=false;}
  }


// open the file for reading:
  dataFile = SD.open("datalog.txt");
  if (dataFile) {
   // Serial1.println("datalog.txt:");
    server << " <p>";   //pww
    // read from the file until there's nothing else in it:
    while (dataFile.available()) {
    //  Serial1.write(dataFile.read());
	char c = dataFile.read();
	
	if ( c== '\n' )  server << " </p>\r\n<p>";   //pww
	else if (c=='\r');
	else  server << c;
    }
    // close the file:
    dataFile.close();
	server << " </p>\r\n";   //pww
  } else {
    // if the file didn't open, print an error:
    Serial1.println("error opening test.txt");
  }


bHeat = checktimes();
//sendSDpage(server); 
}


void formCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      repeat = server.readPOSTparam(name, 16, value, 16);
      if (name[0] == 'd')
      {
        int pin = strtoul(name + 1, NULL, 10);
        int val = strtoul(value, NULL, 10);
        digitalWrite(pin, val);
      }
    } while (repeat);

    server.httpSeeOther(PREFIX "/form");
  }
  else
    outputPins(server, type, true);
}

void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  count++;
//  outputPins(server, type, false);
 Serial1 <<  " send page - default command " ;
sendSDpage(server); 
}
/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin 
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerDigitalRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT_PULLDOWN);
		return digitalRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT_PULLDOWN);
		return digitalRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerDigitalWrite(String command)
{
	bool value = 0;
	
	
	
	
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(command.substring(3,7) == "HIGH") value = 1;
	else if(command.substring(3,6) == "LOW") value = 0;
	else return -2;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, value);
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		digitalWrite(pinNumber+10, value);
		return 1;
	}
	else return -3;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin 
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerAnalogRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) //return -1;
     //else
	 {
      int ret = (int) OBuff[bindex];    // this is pww  stuff fir getting characters  out
	  bindex++;                         // it works for pinAn where n >7
	  if (ret==0){ bindex=0;}
	  return ret;
	 } 
	else if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT);
		return analogRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT);
		return analogRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerAnalogWrite(String command)
{
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	String value = command.substring(3);

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		analogWrite(pinNumber, value.toInt());
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		analogWrite(pinNumber+10, value.toInt());
		return 1;
	}
	else return -2;
}
void setup()
{
  
  //Setup the Tinker application here

	//Register all the Tinker functions

	Spark.function("digitalwrite", tinkerDigitalWrite);

	 // Expose stringout to the cloud
    Spark.variable("stringout", &OBuff, STRING);
	
	
     
	
	  Time.timeStr().toCharArray(OBuff,64,0);
	  strcat(OBuff, " : started");
	  
	  bindex=0;
  
      sensors.begin();
	  
 
  
      lcd = new LiquidCrystal_I2C(0x27, 20, 4);
      lcd->init();
      lcd->backlight();
      lcd->clear();
      lcd->print("***Spark Time***");
  
  
  
  
  sdcard=false;
  count=0;
  pwminute= Time.minute();
  pwsecond= Time.second();
  pwnow=millis();
  
  // set pins 0-8 for digital input
 // for (int i = 0; i <= 9; ++i)pinMode(i, INPUT);
  
 // pinMode(9, OUTPUT);
  
  Serial1.begin(9600);
 //  while (!Serial1.available());

 // Serial1.print("Initializing SD card...");
   
  // Initialize HARDWARE SPI with user defined chipSelect
  delay(100);
  if (SD.begin(chipSelect)) 
  {sdcard=true;
    Serial1.println("SD initialization is done!");}
    else
  { Serial1.println(" SD initialization failed.");}
 // Spark.connect();    //pww
  
  
  webserver.begin();

  webserver.setDefaultCommand(&defaultCmd);
  webserver.addCommand("json", &jsonCmd);
  webserver.addCommand("form", &formCmd);
  webserver.addCommand("advanceHT", &advanceHTCmd);
  webserver.addCommand("advanceHW", &advanceHWCmd);
  webserver.addCommand("stringout", &outputOBuff);
  webserver.addCommand("cmdin", &commandin);
  Serial1 <<  " end of set up. Loop starts next " ;
}

void loop()
{
 // process incoming connections one at a time forever
  
  if (Time.minute() != pwminute) bHeat = checktimes();
  if (Time.second() != pwsecond )
  { webserver.processConnection();

	pwsecond= Time.second(); Serial1 << "." ; 
	//   lcd->clear();
	lcd->setCursor(0,0);
    lcd->print(Time.timeStr());
	
	tempdoor = digitalRead(7);
	if (tempdoor ==1 && door==0) { door = 1; OBuff[0]=0;Time.timeStr().toCharArray(OBuff,64,0);
	  strcat(OBuff, " : opened"); SDCardWrite(OBuff); }  // door opened
	else if (tempdoor==0 && door ==1) { door =0; OBuff[0]=0; Time.timeStr().toCharArray(OBuff,64,0);
	  strcat(OBuff, " : closed "); SDCardWrite(OBuff);  }// door closed
	else  if(Time.second()==0)
	{sensors.requestTemperatures(); // Send the command to get temperatures
     int temp = sensors.getTempCByIndex(0);OBuff[0]=0;
	 snprintf(OBuff, 64, "%d",temp );strcat(OBuff, " degrees "); 
	 lcd->setCursor(0,3);
	 lcd->print(OBuff);}

  }
  // if you wanted to do other work based on a connecton, it would go here
}
