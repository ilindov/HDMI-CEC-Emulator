#include <CEC_Device.h>

#define IN_LINE 2
#define OUT_LINE 3
#define MB_PWR_STATE_PIN 4
#define MB_PWR_PIN 5

// ugly macro to do debug printing in the OnReceive method
#define report(X) do { DbgPrint("report " #X "\n"); report ## X (); } while (0)

#define phy1 ((_physicalAddress >> 8) & 0xFF)
#define phy2 ((_physicalAddress >> 0) & 0xFF)

int flagOff = false;
int flagOn  = false;

class MyCEC: public CEC_Device {
  public:
    MyCEC(int physAddr): CEC_Device(physAddr,IN_LINE,OUT_LINE) { }

    void reportPhysAddr()    { unsigned char frame[4] = { 0x84, phy1, phy2, 0x04 }; TransmitFrame(0x0F,frame,sizeof(frame)); } // report physical address
    void reportStreamState() { unsigned char frame[3] = { 0x82, phy1, phy2 };       TransmitFrame(0x0F,frame,sizeof(frame)); } // report stream state (playing)
    
    void reportPowerState()  { unsigned char frame[2] = { 0x90, 0x00 };             TransmitFrame(0x00,frame,sizeof(frame)); } // report power state (on)
    void reportReqPowerState()  { unsigned char frame[1] = { 0x8f };             TransmitFrame(0x00,frame,sizeof(frame)); } // report power state (on)
    void reportCECVersion()  { unsigned char frame[2] = { 0x9E, 0x04 };             TransmitFrame(0x00,frame,sizeof(frame)); } // report CEC version (v1.3a)
    
    void reportOSDName()     { unsigned char frame[5] = { 0x47, 'K','O','D','I' };  TransmitFrame(0x00,frame,sizeof(frame)); } // FIXME: name hardcoded
    void reportVendorID()    { unsigned char frame[4] = { 0x87, 0x00, 0xF1, 0x0E }; TransmitFrame(0x0F,frame,sizeof(frame)); } // report fake vendor ID
    // TODO: implement menu status query (0x8D) and report (0x8E,0x00)
    
    void handleKey(unsigned char key) {
      switch (key) {
        case 0x00: Serial.println("KEY_RETURN"); break;       //
        case 0x01: Serial.println("KEY_UP_ARROW"); break;     //
        case 0x02: Serial.println("KEY_DOWN_ARROW"); break;   //
        case 0x03: Serial.println("KEY_LEFT_ARROW"); break;   //
        case 0x04: Serial.println("KEY_RIGHT_ARROW"); break;  //
        case 0x09: Serial.println("KEY_HOME"); break;         //
        case 0x0A: Serial.println("KEY_OPTIONS"); break;      //
        case 0x0D: Serial.println("KEY_ESC"); break;          //
        case 0x35: Serial.println("KEY_INFO"); break;         //
        case 0x44: Serial.println("KEY_PLAY"); break;         //
        case 0x45: Serial.println("KEY_STOP"); break;         //
        case 0x46: Serial.println("KEY_PAUSE"); break;        //
        case 0x47: Serial.println("KEY_REC"); break;
        case 0x48: Serial.println("KEY_RWND"); break;         //
        case 0x49: Serial.println("KEY_FFWD"); break;         //
        case 0x4B: Serial.println("KEY_PAGE_DOWN"); break;
        case 0x4C: Serial.println("KEY_PAGE_UP"); break;
        case 0x51: Serial.println("KEY_SUBS"); break;         //
        case 0x53: Serial.println("KEY_GUIDE"); break;        //
        case 0x71: Serial.println("KEY_TT_BLUE"); break;
        case 0x72: Serial.println("KEY_TT_RED"); break;
        case 0x73: Serial.println("KEY_TT_GREEN"); break;     //
        case 0x74: Serial.println("KEY_TT_YELLOW"); break;
      }
    }
        
    void OnReceive(int source, int dest, unsigned char* buffer, int count) {
      if (count == 0) return;

      CEC_Device::OnReceive(source,dest,buffer,count);
      
      switch (buffer[0]) {
        
        case 0x36:  DbgPrint("standby\n");
                    flagOff = true;
                  break;
        case 0x44: handleKey(buffer[1]);
                  break;
        case 0x46: report(OSDName);
                  break;
        case 0x80: if (buffer[3] == phy1 && buffer[4] == phy2)
                    flagOn = true;
                  break;
        case 0x81:  if (buffer[1] == phy1 && buffer[2] == phy2)
                    flagOn = true;
                  break;
        case 0x83:  report(PhysAddr);
                  break;
//        case 0x84:  if (source == 00 && !flagReported) { // TV is powered on
//                      report(PhysAddr);
//                      flagOn = true;
//                      flagReported = true;
//                    }
//                  break;            
        case 0x86:  if (buffer[1] == phy1 && buffer[2] == phy2)
                    report(StreamState);
                    flagOn = true;
                  break;
//        case 0x87:  if (source == 00 && !flagReportedV) {// TV sends VendorID
//                      report(VendorID);
//                      flagReportedV = true;
//                    }
//                  break;
        case 0x8C: report(VendorID);
                  break;
        case 0x8F:  report(PowerState);
                  break;
        case 0x9F: report(CECVersion);
                  break;  
        
//        TODO: case 0x45: Keyboard.releaseAll(); break;
        
      }
    }
};

MyCEC device(0x1500);

void setup()
{
  pinMode(MB_PWR_STATE_PIN, INPUT);
  pinMode(MB_PWR_PIN, OUTPUT);
  digitalWrite(MB_PWR_PIN, LOW);
  
  Serial.begin(115200);

  device.Promiscuous = false;
  device.MonitorMode = false;
  device.Initialize(CEC_LogicalDevice::CDT_PLAYBACK_DEVICE);

  Serial.println("Device started!");
}

void loop()
{
//  Serial.println("OUTTT");
  if (flagOn == true) {
    if (digitalRead(MB_PWR_STATE_PIN) == LOW) {
      digitalWrite(MB_PWR_PIN, HIGH);
      delay(100);
      digitalWrite(MB_PWR_PIN, LOW);
    }
    flagOn = false;
  }
  if (flagOff == true) {
    if (digitalRead(MB_PWR_STATE_PIN) == HIGH) {
      digitalWrite(MB_PWR_PIN, HIGH);
      delay(100);
      digitalWrite(MB_PWR_PIN, LOW);
    }
    flagOff = false;
  }
  
  device.Run();
}
