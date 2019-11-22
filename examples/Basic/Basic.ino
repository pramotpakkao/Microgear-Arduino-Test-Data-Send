#include "MicroGear.h"
#include "WiFi.h"
#include "FirebaseESP32.h"

const char* ssid     = "ttec_qr_ap";
const char* password = "ndrs_2010";

#define APPID   "TestSendApp"
#define KEY     "lZmeboInFFETRlF"
#define SECRET  "OvpLZ8k3EUFGVAHIrHdsack7b"
#define ALIAS   "esp32"


#define FIREBASE_HOST "loratrain-f7ecc.firebaseio.com" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "WPd5lYzs3zFfp6SH59yJUYOTEPqU9UvsIDbRQ5K1"
#define WIFI_SSID "ttec_qr_ap"
#define WIFI_PASSWORD "ndrs_2010"

//Define FirebaseESP32 data object
FirebaseData firebaseData;

FirebaseJson json;

WiFiClient client;
int timer = 0;
int i = 0;
String path;
MicroGear microgear(client);

/* If a new message arrives, do this */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Found new member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Lost member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

/* When a microgear is connected, do this */
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  /* Set the alias of this microgear ALIAS */
  microgear.setAlias(ALIAS);
}


void setup() {
  /* Add Event listeners */

  /* Call onMsghandler() when new message arraives */
  microgear.on(MESSAGE, onMsghandler);

  /* Call onFoundgear() when new gear appear */
  microgear.on(PRESENT, onFoundgear);

  /* Call onLostgear() when some gear goes offline */
  microgear.on(ABSENT, onLostgear);

  /* Call onConnected() when NETPIE connection is established */
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  /* Initial WIFI, this is just a basic method to configure WIFI on ESP8266.                       */
  /* You may want to use other method that is more complicated, but provide better user experience */
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  /* Initial with KEY, SECRET and also set the ALIAS here */
  microgear.init(KEY, SECRET, ALIAS);
  Serial.println("init Complete...");

  /* connect to NETPIE to a specific APPID */
  microgear.connect(APPID);
  Serial.println("connected to Netpie...");

  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  /*
  This option allows get and delete functions (PUT and DELETE HTTP requests) works for device connected behind the
  Firewall that allows only GET and POST requests.
  
  Firebase.enableClassicRequest(firebaseData, true);
  */

  path = "/TestUpdate";

}

void loop() {
  /* To check if the microgear is still connected */
  if (microgear.connected()) {
    Serial.println("connected");

    /* Call this method regularly otherwise the connection may be lost */
    microgear.loop();

//    if (timer >= 1000) {
//      Serial.println("Publish...");
//
//      /* Chat with the microgear named ALIAS which is myself */
//      microgear.chat(ALIAS, "Hello");
//      timer = 0;
//    }
//    else timer += 100;


    Serial.println("------------------------------------");
  Serial.println("Update test...");

 
    json.set("Data" , i);

    if (Firebase.updateNode(firebaseData, path + "/counter", json))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      //No ETag available
      Serial.print("VALUE: ");
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    i++;
    
  }
  else {
    Serial.println("connection lost, reconnect...");
    if (timer >= 5000) {
      microgear.connect(APPID);
      timer = 0;
    }
    else timer += 100;
  }
  delay(100);
}
