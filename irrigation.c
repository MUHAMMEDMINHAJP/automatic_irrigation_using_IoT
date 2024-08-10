/***********************************************************************/
    #include <Wire.h>
    #include <LiquidCrystal_I2C.h>
    #if defined(ESP32)
    #include <WiFi.h>
    #elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #endif
    #include <Firebase_ESP_Client.h>
    #include <addons/TokenHelper.h>
    #include <addons/RTDBHelper.h>
    #define WIFI_SSID "Embedded"
    #define WIFI_PASSWORD "Embedded54321"
    #define API_KEY "AIzaSyDAr4EMfMvWIC9lhTNQlvn1jk7xp0naACs"
    #define DATABASE_URL "fir-project-ca528-default-rtdb.firebaseio.com"
    #define USER_EMAIL "user.embproject@gmail.com"
    #define USER_PASSWORD "Embedded54321"

    #define device_1 D0   // D0
    #define device_2 D3  // D3
    #define device_3 D4 // D5
    #define device_4 D5 // D6

    
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;

    LiquidCrystal_I2C lcd(0x27,16,2);
/***********************************************************************/
    int count = 0 ;
    int SMC = 450;
    int SM = 0;
    int pt,ot,dt=0;
    int d1=0,d2=0,d3=0,d4=0;
    String msg;
/***********************************************************************/
    void Connect_WiFi();
    void Firebase_Store(String PATH,String MSG);
    String Firebase_getString(String PATH);
    int ExtractIntFromString(String,char);
    void get_info(String);
    void timer();
/***********************************************************************/



    void setup()
    {
        lcd.begin();
        lcd.clear();
        lcd.print("Irrigation");
        lcd.setCursor(0, 1);
        lcd.print("System");
        delay(500);
        lcd.begin();
        lcd.clear();
        lcd.print("Embedded");
        lcd.setCursor(0, 1);
        lcd.print("Embedded54321");
        

        pinMode(device_1,OUTPUT);
        pinMode(device_2,OUTPUT);
        pinMode(device_3,OUTPUT);
        pinMode(device_4,OUTPUT);
        digitalWrite(device_1,HIGH);
        digitalWrite(device_2,HIGH);
        digitalWrite(device_3,HIGH);
        digitalWrite(device_4,HIGH);
        pt=ot=millis()/100;
        Connect_WiFi(); 
        
    }
/***********************************************************************/
    void loop()
    {
       SM = analogRead(A0); 
       timer();
        if(d4==2)
        {
          if(SM > SMC)
          {
            digitalWrite(device_4,LOW);
          }
          else
          {
            digitalWrite(device_4,HIGH);
          }
        }
        delay(200);       
    }
/***********************************************************************/
    void Connect_WiFi()
    {
         int i=0;
          Serial.begin(9600);
          delay(100); 
          WiFi.disconnect();
          delay(800); 
          lcd.clear();
          lcd.print("WiFi Connecting");  
          lcd.setCursor(0, 1);
          Serial.println("Connecting to Wi-Fi"); 
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
          Serial.print("Connecting to Wi-Fi");
          delay(100);
          while (WiFi.status() != WL_CONNECTED)
          {
            i++;
            if(i>15)
            {
              i=0;
              lcd.clear();
              lcd.print("WiFi Connecting");  
              lcd.setCursor(0, 1);
            }
            lcd.print("*");
            Serial.print(".");
            delay(300);
          }
          Serial.println();
          Serial.print("Connected with IP: ");
          Serial.println(WiFi.localIP());
          Serial.println();
          Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
          config.api_key = API_KEY;
          auth.user.email = USER_EMAIL;
          auth.user.password = USER_PASSWORD;
          config.database_url = DATABASE_URL;
          config.token_status_callback = tokenStatusCallback;
          #if defined(ESP8266)
            fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
          #endif
          Firebase.begin(&config, &auth);
          Firebase.reconnectWiFi(true);
          Firebase.setDoubleDigits(5);
          config.timeout.serverResponse = 10 * 1000;
          lcd.clear();
          lcd.print("WiFi CONNECTED"); 
    }
/***********************************************************************/
    void Firebase_Store(String PATH,String MSG)
    {
          Serial.print("Uploading data \" ");
          Serial.print(MSG);
          Serial.print(" \"  to the location \" ");
          Serial.print(PATH);
          Serial.println(" \"");
          Firebase.RTDB.setString(&fbdo, PATH, MSG);
          delay(50);
    }
/***********************************************************************/
    String Firebase_getString(String PATH)
    {
      String msg = (Firebase.RTDB.getString(&fbdo, PATH) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
      delay(50);
      return msg;
    }
/***********************************************************************/
    int ExtractIntFromString(String n,char x)
    {
      int num=0,i=0,j,r;
      for(i=0;n[i]!='\0';i++)
      {
        if(n[i]==x)
        {
          Serial.println("char found");
          for(j=i+1;n[j]>='0' && n[j]<='9';j++)
          {
            Serial.print(n[j]);
            r=n[j];
            num = num*10+(r-48);
          }
        }
      }
      Serial.println();
      return num;
    }
/***********************************************************************/
    void timer()
    {
      pt=millis()/100;
      if(pt-ot>10)
      {
        ot=pt;
        dt++;
        if(dt==1)
        {
          lcd.clear();
          lcd.print("Getting Info..");
          lcd.setCursor(0, 1);
          lcd.print("SMC : ");
              msg = Firebase_getString("msg");  
              SMC = ExtractIntFromString(msg,'a'); 
              get_info(msg); 
          lcd.print(SMC);            
          Firebase_Store("SM",String(SM));   
        }
        else if(dt==2)  // timer if 2nd frame
        {
          lcd.clear();
          if(d4==2)  // motor in auto mode
          {
                    lcd.setCursor(10, 1);
                    lcd.print(SM);
                    lcd.setCursor(10, 0);
                    lcd.print(SMC);
          }
          if(d1==1)
          {
            lcd.setCursor(0, 0);
            lcd.print("R1 ON");            
          }
          else if(d1==0)
          {
            lcd.setCursor(0, 0);
            lcd.print("R1 OFF");
          }
          if(d2==1)
          {
            lcd.setCursor(0, 1);
            lcd.print("R2 ON");
          }
          else if(d2==0)
          {
            lcd.setCursor(0, 1);
            lcd.print("R2 OFF");
          }
        }
        else if(dt==3) // timer if 3ed frame
        {
          lcd.clear();
          if(d4==2)   // Auto mode pump
          {
                    lcd.setCursor(10, 1);
                    lcd.print(SM);
                    lcd.setCursor(10, 0);
                    lcd.print(SMC);
          }
          if(d3==1)
          {
            lcd.setCursor(0, 0);
            lcd.print("R3 ON");            
          }
          else if(d3==0)
          {
            lcd.setCursor(0, 0);
            lcd.print("R3 OFF");
          }
          if(d4==1)
          {
            
            lcd.setCursor(0, 1);
            lcd.print("R4 ON (Manual)");
          }
          else if(d4==0)
          {            
            lcd.setCursor(0, 1);
            lcd.print("R4 OFF (Manual)");
          }
          else if(d4==2)
          {            
            lcd.setCursor(0, 1);
                  if(SM > SMC)
                  {
                    digitalWrite(device_4,LOW);
                    lcd.print("R4 A-ON");                    
                  }
                  else
                  {
                    digitalWrite(device_4,HIGH);
                    lcd.print("R4 A-OFF");
                  }
            
          }
          dt=0;
        }
      }
    }
/***********************************************************************/
    void get_info(String)
    {
      Serial.println(msg);
              /*Serial.println(msg[2]);
              Serial.println(msg[3]);
              Serial.println(msg[4]);
              Serial.println(msg[5]);*/
              if(msg[1] == '"')
              { 
                    if(msg[2] == '1')
                    {
                      Serial.println("Device-1 ON");
                      digitalWrite(device_1,LOW);
                      d1=1;
                    }
                    else if(msg[2] == '0')
                    {
                      Serial.println("Device-1 OFF");
                      digitalWrite(device_1,HIGH);
                      d1=0;
                    }
                    
                    if(msg[3] == '1')
                    {
                      Serial.println("Device-2 ON");
                      digitalWrite(device_2,LOW);
                      d2=1;
                    }
                    else if(msg[3] == '0')
                    {
                      Serial.println("Device-2 OFF");
                      digitalWrite(device_2,HIGH);
                      d2=0;
                    }
      
                    if(msg[4] == '1')
                    {
                      Serial.println("Device-3 ON");
                      digitalWrite(device_3,LOW);
                      d3=1;
                    }
                    else if(msg[4] == '0')
                    {
                      Serial.println("Device-3 OFF");
                      digitalWrite(device_3,HIGH);
                      d3=0;
                    }
      
                    if(msg[5] == '1')
                    {
                      Serial.println("Device-4 ON");
                      digitalWrite(device_4,LOW);
                      d4=1;
                    }
                    else if(msg[5] == '0')
                    {
                      Serial.println("Device-4 OFF");
                      digitalWrite(device_4,HIGH);
                      d4=0;
                    }
                    else if(msg[5] == '2')
                    {
                      Serial.println("Device-4 Auto Mode");
                      d4=2;
                      //digitalWrite(device_4,LOW);
                    }
              }
              else
              {
                Serial.println("Connection ERROR");
              }
      //return num;
    }
/***********************************************************************/


//testing codeeeeeeee