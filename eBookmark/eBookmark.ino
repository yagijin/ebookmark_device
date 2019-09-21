#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

int vibPin = 2;                     //バイブレーションモータの制御ピン番号
int soundPin = 3;                   //サウンドモジュールの制御ピン番号（圧電スピーカではないのでPWMの必要なし（たぶん））

int trigger = 0;                    //任意のタグを見つけたかどうか判定するためのトリガー
int nfc_id[7] = {04, 243, 99, 186, 44, 84, 128};//見つけたいタグの識別子（10進数）

void setup() {
	Serial.begin(9600);		//NFCリーダとのシリアル通信のボーレートの設定
	while (!Serial);		  //シリアルポートが開くまで待機 (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			    //SPI　BUSの初期化
	mfrc522.PCD_Init();		//MFRC522の初期化
	delay(4);				      //Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	//Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  //各アクチュエータのピンの設定
  pinMode(vibPin, OUTPUT);
  pinMode(soundPin, OUTPUT);
}

void loop() {
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  //これが最後にPCDを初期化している理由っぽい

  //カード（探しているもの以外も）が見つからなかった場合
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	//カードが複数枚認識されている場合
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  //UIDの表示と比較
  Serial.print(F("HEY!This is Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    if(mfrc522.uid.uidByte[i] != nfc_id[i]){
      trigger = 1;
    }
    Serial.print(mfrc522.uid.uidByte[i]);
  } 
  Serial.println();

  //任意のカードを見つけた場合表示する
  if(!trigger){
    digitalWrite(vibPin, HIGH);
    digitalWrite(soundPin, HIGH);
    delay(100);
    digitalWrite(vibPin, LOW);
    digitalWrite(soundPin, LOW);
  }

  /*
   * ここの処理はカードの情報を表示する場所ですが，重い処理になり，Arduinoの動作速度に大きい
   * 影響を与えるのでコメントアウトしています．
	Dump debug info about the card; PICC_HaltA() is automatically called
	mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  //アンテナのオフ
  mfrc522.PCD_AntennaOff();
  
  /*
   * この部分は任意のIDを見つけた際にArduinoの処理を止める部分です
  if(!trigger){
    while(1){
    }
  }
  */
  
  if(!trigger){
    delay(300);
  }
  
  //アンテナのオン
  mfrc522.PCD_AntennaOn();

  //PCDの初期化
  mfrc522.PCD_Init();

  //トリガーの初期化
  trigger = 0;
}
