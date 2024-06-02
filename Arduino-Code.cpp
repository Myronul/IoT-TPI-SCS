//Wi-fi
#include <WiFi.h>
#include <cstring>
//Display
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 
#include <SPI.h>
//Senzor temp
#include "DHT.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------

#define DHT11PIN 16

//Configurare ecran

#define TFT_CS         25
#define TFT_RST        14
#define TFT_DC         12

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int ok=0;
int ok1=0;

int k=0;
String ComandaEcran;
String ComandaEcranText="Text:";
String IpEcran;

String MesajMonitorizare;
int CheckAlarma=0;

//Configurare senzor DHT

DHT dht(DHT11PIN, DHT11);

//configurare WiFi

const char* ssid = "Casanova"; // Numele rețelei WiFi
const char* password = "patte1212"; // Parola rețelei WiFi
const int serverPort = 80; // Portul pe care serverul Arduino va asculta

WiFiServer server(serverPort); // Creează un server WiFi pe portul specificat

void initializare_ecran(float temp,float humi, String IpEcran)//parametrii standard pentru ecran ip+temp+umiditate
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0,0);
    tft.print("IP: "+IpEcran+"  ");
    tft.println(" ");
    tft.print("Temperatura: ");
    tft.print(temp);
    tft.println(" ");
    tft.print("Umiditate: ");
    tft.print(humi);
    tft.print("%");
}

void setup() {

  //Set-up initial ecran
  dht.begin();//initializare senzor temperatura
  tft.initR(INITR_144GREENTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0,0);
  tft.println("Neonectat...");

  //Set-up DHT
  
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();

  //----------------------------------------------------------SET-up Wifi-----------------------------------------------------------
  Serial.begin(115200); // Inițializează comunicarea serială

  // Conectează Arduino la rețeaua WiFi
  Serial.print("Conectare la rețeaua WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectat la rețeaua WiFi cu succes");
  ok=1;
  
  server.begin(); // Pornirea serverului
  Serial.println("Server pornit cu adresa ip:");
  Serial.println(WiFi.localIP());

  //---------------------------------------------------------------------------------------------------------------------------------
  //-----------------------------------------------------Set-up pentru RGB-----------------------------------------------------------
  
  pinMode(15, OUTPUT);//rosu
  pinMode(2,OUTPUT);//verde
  pinMode(5,OUTPUT);//albastru

  //---------------------------------------------------Set-up pentru Buzzer----------------------------------------------------------

  pinMode(4,OUTPUT);

  //---------------------------------------------------Initializare Ecran-------------------------------------------------------------------

  //tft.initR(INITR_144GREENTAB);

  if(ok==1)
  {
    IpEcran=WiFi.localIP().toString();
    initializare_ecran(temp,humi,IpEcran);
  }

  //-------------------------------------------------Initializaire Led Alarma----------------------------------------------------------------

  pinMode(13,OUTPUT);
  
}

int stare=0;
int stins=0;

void Control_RGB()
{
    stins=1;
    stare++;
    if(stare%4==0)//0 1 2 3
    {
      stare=0;
    }

    if(stare==0)
    {
        digitalWrite(15,HIGH);//rosu
        digitalWrite(5,LOW);
        digitalWrite(2,LOW);
    }

    if(stare==1)
    {
        digitalWrite(15,LOW);
        digitalWrite(5,HIGH);//verde
        digitalWrite(2,LOW);
    }

    if(stare==2)
    {
        digitalWrite(15,LOW);
        digitalWrite(5,LOW);
        digitalWrite(2,HIGH);//blue
    }

    if(stare==3)
    {
        digitalWrite(15,HIGH);//alb
        digitalWrite(5,HIGH);
        digitalWrite(2,HIGH);
    }
    
}

void Scriere_Ecran_IP(char *text, uint16_t color) //nefolosita momentan in testare
{
  
  tft.setCursor(0,0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
  
}

void ReconnectToWiFi() {
  Serial.println("conectare...");
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("incercare de conectare...");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexiune cu succes");
  } else {
    Serial.println("Conexiune esuata");
  }
  
}

void loop() {

  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
       
  int i=0;
  
  while(WiFi.status() != WL_CONNECTED) //daca reteaua wifi locala a cazut, asteapta pana cand revine
  {
    delay(1000);
    Serial.print(".");
    
    if(ok1==0)
    {
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0,0);
      tft.print("Neconectat...");
      ok1=1;
      ok=0;
    }

    ReconnectToWiFi();
    
  }
  ok1=0;

  if(ok==0) //In cazul in care ne reconecram vom rescrie pe ecran datele de baza
  {
    initializare_ecran(temp,humi,IpEcran);
    ok=1;
  }


  WiFiClient client = server.available(); // Așteaptă conexiuni de la client
  
  if (client) { // Dacă s-a conectat un client
    Serial.println("Client conectat");

    while (client.connected()) {
      if (client.available()) {
        String command = client.readStringUntil('\n'); // Citește comanda trimisă de client

        //Check-Connection din aplicatie
        if (command == "1") { 
          
          Serial.println("Primire pachet date receptionate");
          client.println("1\n");//folosim \n deoarece in Java socketul folosit asteapta un caracter \n pentru a termina de citi readline
        } else if (command == "0") { 
          
          Serial.println("Inchidere receptionat");
          client.println("0\n");
        }
        
        //-------------------------------------------aici incepe partea de monitorizare/control--------------------------------------------

        //------------------------------------------------------Control LED RGB------------------------------------------------------------

        if(command == "Aprinde")
        {
          Serial.println("Receptionat control aprindere RGB");
          client.println("Aprindere RGB\n");
          Control_RGB();
        }

        if(command == "Stinge")
        {
          Serial.println("Receptionat control stingere RGB");
          client.println("Stingere RGB\n");
          digitalWrite(15,LOW);
          digitalWrite(5,LOW);
          digitalWrite(2,LOW);
          stins=0;
        }

        //------------------------------------------------------------------ControlBuzzer----------------------------------------------------

        if(command == "AprindeBuzzer")
        {
          tone(4,1000);
          client.println("Aprindere Buzzer\n");
        }

        if(command == "StingeBuzzer")
        {
          noTone(4);
          client.println("Stingere Buzzer\n");
        }

        //------------------------------------------------------------------ControlEcran-------------------------------------------------------

        k=0;
        
        for(i=0;i<5;i++)
        {
          if(command[i]==ComandaEcranText[i])//comparam sa vedem daca s-a transmis <<Text:>>
          {
            k++;
          }
        }

        if(k==5)//S-a transmis mesajul Text: care inseamna sa afisam un text pe ecran vom prelua mesajul transmis dupa text <<Text: OriceText>>
        {
          
          ComandaEcran="";//reinitializare variabila de stocare a textului de afisat
          
          i = 6; 
          while (command[i] != '\0') 
          {
            ComandaEcran += command[i];//concatenare siruri de caractere IMPORTANT variabila va trebui reinitializata cu "" dupa afisare ca sa eliberam textul anterior
            i++;
          }

            Serial.println("S-a receptionat mesajul de control ecran:");
            Serial.println(ComandaEcran);
            client.println("Afisare Text pe Ecran\n");
            k=0;


            //Partea de afisare efectiva pe ecran

            initializare_ecran(temp,humi,IpEcran);
            tft.println("");//new line
            tft.println("");
            tft.println(ComandaEcran);
            
            Serial.println(temp);
            Serial.println(humi);
          
        }

        //------------------------------------------------------------------Monitorizare----------------------------------------------------------

        if(command == "Monitorizare") //ne vom crea un String in care ne vom adauga folosind spatii elementele de monitorizare astfel:
        {                             //StareLed ProcentUmiditate Temperatura Alarma -> exemplu: "Aprins/Stins 34.00% 25.30 AlarmaOn/AlarmaOff"  

          MesajMonitorizare="";//resetam Stringul de salvare a mesajului
          
          if(stins==0)
          {
            MesajMonitorizare += "Stins ";
          }

          if(stins==1)
          {
            MesajMonitorizare += "Aprins ";
          }

          MesajMonitorizare += String(humi);
          MesajMonitorizare += " " + String(temp);

          if(temp>24) //pentru modul de alarma ledul rosu
          {
            digitalWrite(13,HIGH);
            CheckAlarma=1;
            tone(4,1000);
          }

          if(CheckAlarma==0)
          {
            MesajMonitorizare += " AlarmaOff";
          }

          if(CheckAlarma==1)
          {
            MesajMonitorizare += " AlarmaOn";
          }

          client.println(MesajMonitorizare);
          Serial.println(MesajMonitorizare);

          initializare_ecran(temp,humi,IpEcran);
          tft.println("");//new line
          tft.println("");
          tft.println(ComandaEcran);
  
        }

       //-----------------------------------------------------------------ModAlarma----------------------------------------------------------------

       
       
       if(command == "StingeAlarma")
       {
          
          client.println("Alarma Stinsa!\n");
          digitalWrite(13,LOW);
          CheckAlarma=0;
          noTone(4);
          
       }
        
      }
    }
    
    client.stop(); // Deconectează clientul
    Serial.println("Client deconectat");
  }

  
}
