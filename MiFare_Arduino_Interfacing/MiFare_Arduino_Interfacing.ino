#include <LiquidCrystal.h>  //Include LCD library
LiquidCrystal lcd(8,9,4,5,6,7);  //declare an object of LiquidCrystal, define pins responsible for controlling LCD

char button_1 = 10;  //Define pin 10 as button_1
char button_2 = 11;  //Define pin 11 as button_2

/*****MiFare commands*****/
char LED_ON[] = {0xAA,0xBB,0x06,0x00,0x00,0x00,0x07,0x01,0x01,0x07};  //Command to turn on MiFare module onboard LED
char LED_OFF[] = {0xAA,0xBB,0x06,0x00,0x00,0x00,0x07,0x01,0x00,0x06}; //Command to turn off MiFare module onboard LED
char ANTENNA_ON[] = {0xAA,0xBB,0x06,0x00,0x00,0x00,0x0C,0x01,0x01,0x0C};  //Command to turn off MiFare module onboard antenna
char READ_DEV[] = {0xAA,0xBB,0x05,0x00,0x00,0x00,0x03,0x01,0x02};  //Command to read MiFare module device number
char CARD_REQ[] = {0xAA,0xBB,0x06,0x00,0x00,0x00,0x01,0x02,0x52,0x51};  //Command to request card type of MiFare card 
char ANTI_COL[] = {0xAA,0xBB,0x05,0x00,0x00,0x00,0x02,02,00};  //Command for anti-collision
char CARD_SEL[] = {0xAA,0xBB,0x09,0x00,0x00,0x00,0x03,0x02,0x00,0x00,0x00,0x00,0x00};  //Command to select specific MiFare card for access
                                                                                       //Byte 0 to byte 12 of this command is initialized with 0x00, since have to obtain the NUID of the MiFare card scanned 
                                                                                       //Byte 8 to byte 11 of this command will filled with NUID (started with the lowest byte) of the MiFare card desire to access
                                                                                       //Byte 12 of this command is checksum from byte 4 to byte 11 of this command may change for NUID different from that in this case
                                                                                       //This array will be filled during the subroutine of the get_nuid() in this program       
char AUTHEN_SEC0[] = {0xAA,0xBB,0x0D,0x00,0x00,0x00,0x07,0x02,0x60,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x65};  //Command to authenticate Sector 0 in memory of MiFare card
char AUTHEN_SEC1[] = {0xAA,0xBB,0x0D,0x00,0x00,0x00,0x07,0x02,0x60,0x04,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x61};  //Command to authenticate Sector 1 in memory of MiFare card
char READ_0[] = {0xAA,0xBB,0x06,0x00,0x00,0x00,0x08,0x02,0x00,0x0A};  //Command to read Block 0 (manufacturer block in Sector 0) in memory of MiFare card
char WRITE_4[] = {0xAA,0xBB,0x16,0x00,0x00,0x00,0x09,0x02,0x04,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x0F}; //Command to write data into Block 4 (in Sector 1) in memory of MiFare card
                                                                                                                                                      //Data to be written (started with the lowest byte) is from byte 9 to 24, 
                                                                                                                                                      //total of 16 bytes data
                                                                                                                                                      //Data is 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A
                                                                                                                                                      //0x0B,0x0C,0x0D,0x0E,0x0F in this case
                                                                                                                                                      //The last checksum byte of this command may change for data to written different from 
                                                                                                                                                      //that in this case
char READ_4[] = {0xAA,0xBB,0x06,0x00,0x00,0x00,0x08,0x02,0x04,0x0E};  //Command to read data in Block 4 (in Sector 1) in memory of MiFare card
/*****************************/

unsigned char reply_buffer[26]; //Buffer to store data received from MiFare module when a certain command is sent (maximum is 26 bytes in this case)
unsigned char NUID[4];  //Array for storing NUID of a MiFare card
char mode = 0;  //Variable storing current mode of the program 

void setup()
{
  pinMode(button_1,INPUT);  //Configure pins button_1 & button_2 as inputs
  pinMode(button_2,INPUT); 
  digitalWrite(button_1,HIGH);  //Pull-up pins button_1 & button_2 
  digitalWrite(button_2,HIGH);  
  
  Serial.begin(19200);  //Initiallize UART serial communication with baudrate = 19200bps (default baudrate of MiFare module)
  lcd.begin(8,2);  //Initialize the lcd object created as type 2 rows x 8 column
  
  lcd.print("MiFare");  //Print welcome message "MiFare Demo" on LCD  
  lcd.setCursor(0,1);
  lcd.print("Demo");
  delay(2000);
  lcd.clear();
}

void loop()
{
  lcd.setCursor(0,0);
  switch(mode)  
  {
     case 0:
     //Blinking of the MiFare module onboard LED in at rate of 0.5 Hertz
       lcd.print("0.LED    ");
       if(!digitalRead(button_2))
       {
           while(!digitalRead(button_2));  
           while(1)
           {
             lcd.setCursor(0,1);
             lcd.print("LED ON ");
             led_on();      
             delay(1000);  
             lcd.setCursor(0,1);
             lcd.print("LED OFF");
             led_off();
             delay(1000); 
           }
        }
        break;
         
      case 1:
      //Read the device number of MiFare module
        lcd.print("1.DEV NO"); 
        if(!digitalRead(button_2))
        {
            while(!digitalRead(button_2));
            read_dev();
            lcd.setCursor(0,1);
            lcd_to_hex(reply_buffer[10]);  //Byte 9 and byte 10 of replied bytes (started with lowest byte)contains MiFare module device number
            lcd_to_hex(reply_buffer[9]);
            while(1);
        }
        break;
       
       case 2:
       //Read the NUID of MiFare card 
         lcd.print("2.NUID    ");
         if(!digitalRead(button_2))
         {
            while(!digitalRead(button_2));
            get_nuid();  //NUID of MiFare card  is stored in NUID[] array after execution of this subroutine 
                         //There is total of 4 bytes started with the lowest byte
            lcd.setCursor(0,1);
            for(int i = 3; i>=0; i--)
            {
               lcd_to_hex(NUID[i]);  //Print out the NUID obtained on LCD
            }
            while(1);
         }
         break;
         
       case 3:
       //Read the data stored in Block 0 in Sector 0 in memory of MiFare card 
         lcd.print("3.READ0   ");
         if(!digitalRead(button_2))
         {
            while(!digitalRead(button_2));
            read_0();
            while(1)
            {
              //Byte 9 to byte 24  of the replied bytes contains data (started with the lowest byte) read from Block 0 in Sector 0
              //Due to the small screen of the LCD used, the data is printed on LCD for twice. The lower 8 bytes (started with lowest byte of the lower 8 bytes) is printed at the first time
              //While the upper 8 bytes (started with lowest byte of the upper 8 bytes)is printed at the second time.
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("0 to 7");
              lcd.setCursor(0,1);
              lcd.print("byte");
              delay(2000);
              lcd.clear();
              lcd.setCursor(0,0);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[9+i]); 
              }
              lcd.setCursor(0,1);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[13+i]); 
              }
              delay(5000); 
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("8 to 15");
              lcd.setCursor(0,1);
              lcd.print("byte");
              delay(2000);
              lcd.clear();
              lcd.setCursor(0,0);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[17+i]); 
              }
              lcd.setCursor(0,1);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[21+i]); 
              }
              delay(5000);
            }
         }
         break;
       case 4:
       //Write data into Block 4 in Sector 1 in memory of MiFare card 
         lcd.print("4.WRT_4 ");
         if(!digitalRead(button_2))
         {
            while(!digitalRead(button_2));
            write_4();
            lcd.setCursor(0,1);
            lcd.print("Success!");  //Print "Success" on LCD to indicate a success write
            while(1);
         }
         break;
       case 5:
       //Read the data stored in Block 4 in Sector 1 in memory of MiFare card 
         lcd.print("5.READ_4");
         if(!digitalRead(button_2))
         {
            while(!digitalRead(button_2));
            read_4();
            while(1)
            {
              //Byte 9 to byte 24  of the replied bytes contains data (started with the lowest byte) read from Block 4 in Sector 1
              //Due to the small screen of the LCD used, the data is printed on LCD for twice. The lower 8 bytes (started with lowest byte of the lower 8 bytes) is printed at the first time
              //While the upper 8 bytes (started with lowest byte of the upper 8 bytes) is printed at the second time
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("0 to 7");
              lcd.setCursor(0,1);
              lcd.print("byte");
              delay(2000);
              lcd.clear();
              lcd.setCursor(0,0);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[9+i]); 
              }
              lcd.setCursor(0,1);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[13+i]); 
              }
              delay(5000);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("8 to 15");
              lcd.setCursor(0,1);
              lcd.print("byte");
              delay(2000);
              lcd.clear();
              lcd.setCursor(0,0);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[17+i]); 
              }
              lcd.setCursor(0,1);
              for(int i = 0; i<4; i++)
              {
                 lcd_to_hex(reply_buffer[21+i]); 
              }
              delay(5000);
            }
         }
         break;
       default: break;      
  }
  
  if(!digitalRead(button_1))  //Detect the pressing down of button_1, change mode accordingly 
  {
     while(!digitalRead(button_1));
     mode++;
     if(mode > 5)  
     {
       mode = 0;
     } 
  }
}

//Turn on MiFare module LED subroutine
void led_on(void)
{
  for(int i = 0; i<10; i++)
  {
    Serial.write(LED_ON[i]); 
  }
  
  for(int i = 0; i<10; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  } 
}

//Turn on MiFare module LED subroutine
void led_off(void)
{
  for(int i = 0; i<10; i++)
  {
    Serial.write(LED_OFF[i]); 
  }
  
  for(int i = 0; i<10; i++)
  {
    while(!Serial.available());
    reply_buffer[i] = Serial.read();
  }
}

//Read MiFare device number subroutine
void read_dev(void)
{
  for(int i = 0; i<9; i++)
  {
    Serial.write(READ_DEV[i]); 
  }
  
  for(int i = 0; i<12; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }    
}

//Turn on antenna of MiFare module subroutine
void antenna_on(void)
{
  for(int i = 0; i<10; i++)
  {
    Serial.write(ANTENNA_ON[i]); 
  }
  
  for(int i = 0; i<10; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }  
}

//Request MiFare card type subroutine
void card_req(void)
{
  for(int i = 0; i<10; i++)
  {
    Serial.write(CARD_REQ[i]); 
  }
  
  for(int i = 0; i<12; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }   
}

//Perform Anti-Collison subroutine
void anti_col(void)
{
  for(int i = 0; i<9; i++)
  {
    Serial.write(ANTI_COL[i]); 
  }
  
  for(int i = 0; i<14; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  } 
  
  //Byte 9 to byte 12 of replied bytes contains NUID of the MiFare card 
  //Store them in NUID[] array
  for(int i = 0; i<4; i++)
  {
      NUID[i] = reply_buffer[9+i];
  }
}

//Select MiFare card subroutine
void card_sel(void)
{
  for(int i = 0; i<13; i++)
  {
    Serial.write(CARD_SEL[i]); 
  }
  
  for(int i = 0; i<11; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }   
}

//Authenticate Sector 0 in memory of MiFare card subroutine
void authen_sec0(void)
{
  for(int i = 0; i<17; i++)
  {
    Serial.write(AUTHEN_SEC0[i]); 
  }
  
  for(int i = 0; i<10; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }   
}

//Authenticate Sector 1 in memory of MiFare card subroutine
void authen_sec1(void)
{
  for(int i = 0; i<17; i++)
  {
    Serial.write(AUTHEN_SEC1[i]); 
  }
  
  for(int i = 0; i<10; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }   
}

//Read data from Block 0 (manufaturer block) in memory of MiFare card subroutine
void read_0 (void)
{
  get_nuid();
  card_sel();  //Select MiFare card 
  authen_sec0();  //Authenticate Sector 0 in memory of MiFare card
  for(int i = 0; i<10; i++)
  {
    Serial.write(READ_0[i]); 
  }
  
  for(int i = 0; i<26; i++)
  {
    while(!Serial.available());
    reply_buffer[i] = Serial.read(); 
  }
}

//Read data from Block 4 in memory of MiFare card subroutine
void read_4(void)
{
  get_nuid(); 
  card_sel();  //Select MiFare card 
  authen_sec1();  //Authenticate Sector 0 in memory of MiFare card
  for(int i = 0; i<10; i++)
  {
    Serial.write(READ_4[i]); 
  }
  
  for(int i = 0; i<26; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }  
}

//Write data into from Block 4 in memory of MiFare card subroutine
void write_4(void)
{
  get_nuid();
  card_sel();  //Select MiFare card 
  authen_sec1();  //Authenticate Sector 0 in memory of MiFare card
  for(int i = 0; i<26; i++)
  {
    Serial.write(WRITE_4[i]); 
  }
  
  for(int i = 0; i<10; i++)
  {
     while(!Serial.available());
     reply_buffer[i] = Serial.read();
  }   
}

//Get NUID of MiFare card subroutine
void get_nuid(void)
{
  unsigned char xor_temp = 0;
  antenna_on();  //On the MiFare module onboard antenna
  card_req();  //Request card type of MiFare card 
  anti_col();  //Perform anti-collision
  
  for(int i = 0; i<4; i++)  //Fill up byte 8 to byte 11 of CARD_SEL[] with NUID 
  {
     CARD_SEL[i+8] = NUID[i]; 
  }
  
  for(int i = 0; i<8; i++)  //Calculate the checksum of byte 4 to byte 11 of the CARD_SEL[] 
  {
    xor_temp ^= CARD_SEL[4+i];
  }
  CARD_SEL[12] = xor_temp; //Fill the checksum in byte 12 of CARD_SEL[]
}

//Print byte as hex numbers on LCD subroutine
void lcd_to_hex(unsigned char byte_receive)
{
  unsigned char byte_temp = 0;
  byte_temp = (byte_receive>>4) & 0x0F;
  lcd.print(byte_temp,HEX);
  byte_temp = byte_receive & 0x0F;
  lcd.print(byte_temp,HEX);
}

