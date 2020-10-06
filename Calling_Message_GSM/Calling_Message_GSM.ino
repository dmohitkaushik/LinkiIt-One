#include <LGSM.h>
#include <LGPS.h>

gpsSentenceInfoStruct info;
char buff[256];

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  double latitude;
  double longitude;
  int tmp, hour, minute, second, num ;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
    Serial.println(buff);
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    sprintf(buff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
    Serial.println(buff); 
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "satellites number = %d", num);
    Serial.println(buff); 
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

String remoteNumber;
String command; 
String number = "";
String sms_text;
boolean call = false;
char charbuffer[20];




void setup(){

  Serial.begin(9600); 
  LGPS.powerOn();
  //Serial.println("LGPS Power on, and waiting ..."); 
  //delay(3000);
}

void loop(){
 
  // add any incoming characters to the String:
  while (Serial.available() > 0){

    
    
   command = Serial.readString();
     command.trim();
     if(command.substring(0,1).equals("C")){

      number = command.substring(1, command.length());
      call = true;
     }else if( command.substring(0, 1).equals("S")){

       
      
      for(int i = 0; i <command.length(); i++){
        if(command.substring(i, i+1).equals("T")){
          sms_text = command.substring(i+1, command.length());
          number = command.substring(1, i);
          //break;
      }
      }
     }
else if (command ==("G"))
{
  // put your main code here, to run repeatedly:
  Serial.println("LGPS loop"); 
  LGPS.getData(&info);
  Serial.println((char*)info.GPGGA); 
  parseGPGGA((const char*)info.GPGGA);
  //delay(2000);
  
}
//********************************   SMS    *****************************

       if(sms_text.length() > 0){

          number.toCharArray(charbuffer, 20);
          LSMS.beginSMS(charbuffer);
          LSMS.print(sms_text);
          Serial.println("To: " + number);
          Serial.println("Text: " + sms_text);


        if(LSMS.endSMS()){
          
          Serial.println("SMS is sent");
        }else{
          Serial.println("SMS is not sent");
          delay(5000);
        }
        sms_text = "";
      }

     

      //******************   CALL  ************************
      if (number.length() < 20 && call == true){
        
        // let the user know you're calling:
        Serial.print("Calling to : ");
        Serial.println(number);
        Serial.println();

        // Call the remote number
        number.toCharArray(charbuffer, 20);
        
        // Check if the receiving end has picked up the call
        if(LVoiceCall.voiceCall(charbuffer)){
          
          Serial.println("Call Established. Enter line to end");
          // Wait for some input from the line
          while(Serial.read() !='\n');
          // And hang up
          LVoiceCall.hangCall();
        }
        Serial.println("Call Finished");
        number="";
        call = false;
        Serial.println("Enter phone number");
      }else{
        
        //Serial.println("That's too long for a phone number or you forgot to add C or S"); 
        number = "";
        call = false;
      }
    }    
  } 
