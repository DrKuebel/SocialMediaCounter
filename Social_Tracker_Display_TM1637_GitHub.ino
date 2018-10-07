/*******************************************************************
 *  Here is the Sourcecode for my Social Media Counter
 *  
 *  https://www.youtube.com/c/DrKuebel
 *  https://www.instagram.com/DrKuebel
 *  https://www.facebook.com/DrKuebel
 *  https://twitter.com/vw_181
 *  
 *  This Social Media Counter is based on an idea and sourcecode from Becky Stern                   
 *  
 *  Have a look at her Social media channels
 *  https://youtu.be/3Q2JZhgOsG4
 *  https://www.instructables.com/id/EOGVPMLJD2FC7WM/
 *******************************************************************/
 
// requires the following libraries, search in Library Manager or download from github:

#include <Wire.h>                  // installed by default
#include <TwitterApi.h>            // https://github.com/witnessmenow/arduino-twitter-api
#include <ArduinoJson.h>           // V5.1.3  // https://github.com/bblanchon/ArduinoJson    
#include "InstagramStats.h"        // https://github.com/witnessmenow/arduino-instagram-stats
#include "JsonStreamingParser.h"   // https://github.com/squix78/json-streaming-parser  
#include <YoutubeApi.h>            // https://github.com/witnessmenow/arduino-youtube-api
#include <TM1637Display.h>         // https://github.com/avishorp/TM1637

// these libraries are included with ESP8266 support
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

//------- Replace the following! ------
char ssid[] = "Your WLAN SSID";       // your network SSID (name)
char password[] = "Your WLAN Password";  // your network key

// Twitter Details
//******************************
// Normally we would use these to generate the bearer token but its not working yet :/
// Use steps on the readme to generate the Bearer Token

#define BEARER_TOKEN "Your Token"
bool haveBearerToken = false;

//Using curl to get bearer token
// curl -u "$CONSUMER_KEY:$CONSUMER_SECRET" \
//    --data 'grant_type=client_credentials' \
//    'https://api.twitter.com/oauth2/token'

WiFiClientSecure secureClient;
WiFiClient client;

TwitterApi TwitterStats(secureClient);
InstagramStats instaStats(secureClient);

unsigned long api_delay = 1 * 60000; //time between api requests (1mins)
unsigned long api_due_time;

//Inputs

String screenName = "your Twittername";    // Your Twittername here
String userName = "your Instagram name";    // Yout Instagramname here

// YouTube Details
// *************************
// google API key
// create yours: https://support.google.com/cloud/answer/6158862?hl=en
#define API_KEY "your API key"
// youtube channel ID
// find yours: https://support.google.com/youtube/answer/3250431?hl=en
#define CHANNEL_ID "your Channel ID"
YoutubeApi api(API_KEY, secureClient);

unsigned long api_mtbs = 1000; //mean time between api requests
unsigned long api_lasttime;   //last time api request has been done

long subs = 0;

// Facebook Details
//***************************
String page_url = "your Facebook name";              
const char* host = "mbasic.facebook.com";
const char* fingerprint = "93:6F:91:2B:AF:AD:21:6F:A5:15:25:6E:57:2C:DC:35:A1:45:1A:A5";

// 7 Segment Displays
//***************************
const int CLK_TW = D1; 
const int DIO_TW = D2; 
TM1637Display displayTW(CLK_TW, DIO_TW);

const int CLK_FB = D1; 
const int DIO_FB = D3;
TM1637Display displayFB(CLK_FB, DIO_FB);

const int CLK_YT = D1; 
const int DIO_YT = D4;
TM1637Display displayYT(CLK_YT, DIO_YT);

const int CLK_IN = D1; 
const int DIO_IN = D5;
TM1637Display displayIN(CLK_IN, DIO_IN);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.println("");
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address : ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  Serial.println("******************");
  TwitterStats.setBearerToken(BEARER_TOKEN);
  haveBearerToken = true;
     
     displayTW.setBrightness(15);
     displayYT.setBrightness(15);
     displayIN.setBrightness(15);
     displayFB.setBrightness(15);
}

void loop() {
 if (millis() > api_due_time)  {
    if(haveBearerToken){
      getTwitterStats(screenName);
      }
      getInstagramStatsForUser();
      getFacebookUsers();  
      getYoutubeUsers();

   delay(10000);
   // api_due_time = millis() + api_delay;
    Serial.println("******************"); 
  }
}

void getTwitterStats(String name) {
   String responseString = TwitterStats.getUserStatistics(name);
     DynamicJsonBuffer jsonBuffer;
     JsonObject& response = jsonBuffer.parseObject(responseString);
   if (response.success()) {
      int followerCount = response["followers_count"];
      Serial.print("Twitter Followers   : ");
      Serial.println(followerCount);
      
      displayTW.showNumberDec(followerCount);

    } else {
      Serial.println("Failed to parse Json");
    }
}

void getInstagramStatsForUser() {
  InstagramUserStats response = instaStats.getUserStats(userName);
  Serial.print("Instagram Followers : ");
      int like_in = response.followedByCount;
      Serial.println(like_in);
      displayIN.showNumberDec(like_in);

}

void getYoutubeUsers() {
 if (millis() > api_lasttime + api_mtbs)  {
    if(api.getChannelStatistics(CHANNEL_ID))
    {
      Serial.print("Youtube Subscriber  : ");
      Serial.println(api.channelStats.subscriberCount);

            displayYT.showNumberDec(api.channelStats.subscriberCount);

     }
    api_lasttime = millis();
  }
}

void getFacebookUsers() {
  if (!secureClient.connect(host, 443)) { Serial.println("connection failed");}
 
  secureClient.verify(fingerprint, host);
  String url = "/"+page_url+"/community?locale=en_US";  //
  secureClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +               
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");
   while (secureClient.connected()) {
      String line = secureClient.readStringUntil('>');
      if(line.startsWith("<meta name=\"description\"")){      
      int st = line.indexOf(".")+2;
      int sp = line.indexOf("likes",st)-1;
      int st2 = sp + 14;
      int sp2 = line.indexOf("talking",st2)-1;
      String like = line.substring(st,sp);
      String talk = line.substring(st2,sp2);
      Serial.print("Facebook Likes      : ");
      Serial.println(like);
          int like_i = (like.toInt());
          displayFB.showNumberDec(like_i);
  }
  }
}
   
