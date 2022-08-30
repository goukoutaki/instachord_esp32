#include "M5Atom.h"
#include <BLEDevice.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

//Driverの対応付け
Adafruit_PWMServoDriver pwm  = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver pwm3 = Adafruit_PWMServoDriver(0x42);

//サーボモータの設定
#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 80 // Analog servos run at ~50 Hz updates


//ノート初期値　[場合]ドミノ -> 0 == C(-1)
const int note_start  = 28 + 12;


unsigned char state = 0;

//モーターの回転角設定 [0,5,10,15,24は開放弦]
int ang[44][2] = {
{0,0},//ch0
{235,175},//ch1
{260,200},//ch2
{265,205},//ch3
{425,485},//ch4
{0,0},//ch5
{275,215},//ch6
{260,200},//ch7
{265,205},//ch8
{320,380},//ch9
{0,0},//ch10
{255,195},//ch11
{305,245},//ch12
{300,240},//ch13
{310,370},//ch14
{0,0},//ch15
{295,355},//ch16
{325,385},//ch17
{305,365},//ch18
{320,260},//ch19
{425,485},//ch20
{330,390},//ch21
{410,470},//ch22
{275,215},//ch23
{0,0},//ch24
{340,460},//ch25
{320,425},//ch26
{305,375},//ch27
{245,190},//ch28
{0,0},//ch29
{0,0},//ch30
{0,0},//ch31
};

const CRGB COLOR[4] = {
  0xf00000,
  0x00f000,
  0x0000f0,
  0x707070
};


void setting_mode() {
  int type, ch, mm, value, v1, v2;
  M5.dis.drawpix(0, COLOR[2]);
  while (1) {
    if (Serial.available() > 0) {
      unsigned char buf[16];
      Serial.readBytes(buf, 1);
      type = buf[0];
      switch (type) {
        // set
        case 1:
          Serial.readBytes(buf, 1);
          ch = buf[0];
          Serial.readBytes(buf, 1);
          mm = buf[0];
          Serial.readBytes(buf, 1);
          v1 = buf[0];
          Serial.readBytes(buf, 1);
          v2 = buf[0];
          value = v1 * 256 + v2;
          if (ch >= 0 && ch <= 44 && mm <= 2 && value < 2048) {
            M5.dis.drawpix(1, COLOR[0]);
            ang[ch][mm] = value;
            if (ch < 16)
              pwm.setPWM(ch, 0, ang[ch][mm]);
            else if (ch < 32)
              pwm2.setPWM(ch - 16, 0, ang[ch][mm]);
            else
              pwm3.setPWM(ch - 32, 0, ang[ch][mm]);
          }
          break;
        // play
        case 2:

          break;

        // save & load
        case 3:

          Serial.readBytes(buf, 1);
          break;

        // get value
        case 4:
          Serial.readBytes(buf, 1);
          ch = buf[0];
          Serial.readBytes(buf, 1);
          mm = buf[0];
          if (ch >= 0 && ch <= 44 && mm <= 2) {
            buf[0] = ang[ch][mm] / 256;
            buf[1] = ang[ch][mm] % 256;
            Serial.write(buf, 2);
          }
          break;
        // end
        default:
          goto END_LOOP;
      }
    }
    M5.update();
  }
END_LOOP:
  M5.dis.drawpix(0, 0);
  M5.dis.drawpix(1, 0);
  M5.update();
  ;
}

void setServoPulse(uint8_t n, double pulse) {
  double pulselength;

  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= SERVO_FREQ;   // Analog servos run at ~60 Hz updates
  // Serial.print(pulselength); Serial.println(" us per period");
  pulselength /= 4096;  // 12 bits of resolution
  // Serial.print(pulselength); Serial.println(" us per bit");
  pulse *= 1000000;  // convert input seconds to us
  pulse /= pulselength;
  // Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}


/*
 *  ギターの場合、あるノートを押えたら、同じ弦のほかのノートはオフにしないといけない
 */

 //モーターの回転角設定 [0,5,10,15,24は開放弦]


int string_pos[][2] = {
  {0, 4},
  {5, 9},
  {10, 14},
  {15, 18},
  {19, 23},
  {24, 28}
  };

int  push_time[36] = {0};

int  push[36] = {0};

void off(int n) {
    if(n < 16){
        pwm.setPWM(n, 0, ang[n][0]);
    } else {
        pwm2.setPWM(n-16, 0, ang[n][0]);
    }
}


void on_ch1(int n)
{

  // 既に押さえてあれば何もしない
  if(push[n] == 1 || n >= 29)return;

  int flag = 0;
  // 同じ弦の他の音をはなす
  for(int k = 0;k < 6;k++){
    if(n <= string_pos[k][1]){
      
      for(int i = string_pos[k][0];i <= string_pos[k][1];i++){
        if(push[i] == 1){
          off(i);
          push[i] = 0;
        }
      }
      k = 100;
      break;
    }
  }

  //弦を押さえる
  if (n < 16) {
    pwm.setPWM(n, 0, ang[n][1]);
  } else if (n < 32) {
    pwm2.setPWM(n - 16, 0, ang[n][1]);
  } else {
    pwm3.setPWM(n - 32, 0, ang[n][1]);
  }
  push[n] = 1;
  push_time[n] = 100;

  
}



//************************************************************************************
// BLE-MIDI規格で定義されたUUID
static BLEUUID UUID_SERVICE_MIDI("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
static BLEUUID UUID_CHARACTERISTIC_MIDI("7772E5DB-3868-4112-A1A9-F2669D106BF3");

// BLE接続に使用するclientクラス
BLEClient *bleClient;
// 接続対象デバイスのアドレス
BLEAddress *bleAddressToConnect = NULL;
static BLEAdvertisedDevice _advertisedDevice;

// デバイスへの接続/切断が発生したときに呼び出されるコールバック処理
class BleClientCallbacks : public BLEClientCallbacks {
    void onConnect(BLEClient *bleClient) {
      Serial.println(" - Connected");
    }
    void onDisconnect(BLEClient *bleClient) {
      Serial.println(" - Disconnected");
    }
};

// デバイスのスキャン中に、周囲に接続可能な対象が見つかった時に呼び出されるコールバック
class BleAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("scan device:%s\n", advertisedDevice.getName().c_str());
      // MIDIサービスを持つデバイス以外は無視
      if (advertisedDevice.isAdvertisingService(UUID_SERVICE_MIDI)) {
        // 対象のアドレスを控えておく（接続処理自体はメインループのほうが始めてくれる）
        bleAddressToConnect = new BLEAddress(advertisedDevice.getAddress());
        _advertisedDevice = advertisedDevice;
        Serial.printf("[%s] %s", bleAddressToConnect->toString().c_str(), advertisedDevice.getName().c_str());
        // これ以上のデバイススキャンはしなくてよいので止める
        advertisedDevice.getScan()->stop();
      }
    }
};

// 何らかのデータ変更を受信したときに呼び出されるコールバック（主に接続後のMidiメッセージ受信）
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  const char *notenames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

  /*
  for(int i = 0;i < length;i++)
       Serial.printf("%x:", pData[i]);
  Serial.printf("\n");
  */
  
  int s;
  uint8_t cmd = pData[2];
  uint8_t note;
  uint8_t velocity = pData[s++];
  if (cmd == 0x9e) {
    s = 3;
    while (s < length) {
      uint8_t note = pData[s++];
      uint8_t velocity = pData[s++];
      s++;
      if (note < 80 && velocity > 50) {        // NoteOn
        uint8_t octave = note / 12;
        const char *notename = notenames[note % 12];
        Serial.printf("Note On: %s%d %d\n", notename, octave, velocity);
      }
    }
  }
}

// デバイスへの接続開始
bool connectToServer(BLEAddress pAddress) {
  // 通信内容の暗号化 ESP_BLE_SEC_ENCRYPT_NO_MITM  ESP_BLE_SEC_ENCRYPT
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  // 指定されたデバイスに接続し、Midi用のServiceとCharacteristicがあるかどうか確認する
  BLEDevice::getScan()->stop();
  if (bleClient->isConnected()) bleClient->disconnect();

  Serial.println("connecting...");
  //  bleClient->connect(pAddress);
  bleClient->connect(&_advertisedDevice);

  Serial.println("end\n");
  BLERemoteService *service = bleClient->getService(UUID_SERVICE_MIDI);
  if (service != nullptr) {
    Serial.println("found service\n");
    BLERemoteCharacteristic *characteristic = service->getCharacteristic(UUID_CHARACTERISTIC_MIDI);
    if (characteristic != nullptr) {
      Serial.println("found chara\n");
      const uint8_t notificationOn[] = {0x1, 0x0};
      characteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      // このCharacteristicに変更があった際に通知を受けるコールバックを設定。
      // 後はなんらかのデータが送られてくる度にコールバックが呼び出されるのでそちらで処理する
      characteristic->registerForNotify(notifyCallback);
      return true;
    }
  }
  return false;
}



void setup() {

 M5.begin(true, true, true);
  delay(50);
  M5.dis.clear();

  Serial.begin(115200);

  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
  delay(10);

  pwm2.begin();
  pwm2.setOscillatorFrequency(27000000);
  pwm2.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
  delay(10);

  pwm3.begin();
  pwm3.setOscillatorFrequency(27000000);
  pwm3.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
  delay(10);

  // Serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  BLEDevice::init("GuitarRobo");
  bleClient = BLEDevice::createClient();
  // 周囲のデバイスを検索するBLEScanの設定（singletonなのでgetScan()は毎回同じものを返す)
  BLEScan *bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new BleAdvertisedDeviceCallbacks());
  bleScan->setActiveScan(true);
  delay(500);
  Serial.println("Scanning BLE-MIDI Devices...");
  BLEDevice::getScan()->start(10);
  delay(1000);
  if (!connectToServer(*bleAddressToConnect)) {
    Serial.println(" - Failed");
  }
  
}


void OnMidiNoteOn(byte channel, byte note, byte velocity) {

  if (channel == 1) {
    if (velocity > 1 && note >= note_start && note < note_start + 44) {
      on_ch1(note - note_start);
    }
    if (note < note_start) {
      for (int i = 0; i < 44; i++)
        off(i);
    }
  }
}



void loop() {
 int type, ch, noteNum, velocity;

  while (1) {
    if (Serial.available() > 0) {
      unsigned char buf[3];
      Serial.readBytes(buf, 1);
      type = buf[0];
      ch = (type & 0x0f ) + 1;
      switch (type) {
        
        case 0x90:
          Serial.readBytes(buf, 1);
          noteNum = buf[0];
          Serial.readBytes(buf, 1);
          velocity = buf[0];
          OnMidiNoteOn(1, noteNum, velocity);
          break;

        case 0x80:
          break;

        // setting mode
        case 0x9F:
          Serial.readBytes(buf, 1);
          noteNum = buf[0];
          Serial.readBytes(buf, 1);
          velocity = buf[0];
          if (noteNum == 1 && velocity == 1)
            setting_mode();

          break;


        default:
          break;
      }
    }
    delayMicroseconds(100);
    for(int i = 0;i < 36;i++)
      if(push_time[i] > 0)push_time[i]--;
  }
}
