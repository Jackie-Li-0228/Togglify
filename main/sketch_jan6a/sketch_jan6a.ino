#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>   //  ESP8266WebServer库

// 定义引脚
#define SERVO_PIN D3      // 舵机连接到 D3 (GPIO0)
#define LED_PIN LED_BUILTIN  // 内置 LED 引脚
#define BT_BAUDRATE 115200  // 蓝牙模块波特率


// 全局对象和变量
Servo myservo;              // 舵机对象
bool wifiEnabled = false;   // 默认 WiFi 模块关闭
String WIFI_SSID="zuanbigstage";
String WIFI_PASSWORD="G5014891@#";
bool disableControl = false;
unsigned long disableTime = 0;//冷却模式， 避免ESP8266短时间接收到大量开关灯指令流并处理。
int coolTime=5000;//5000ms，表示冷却模式的一次持续时间


ESP8266WebServer esp8266_server(80);  // 建立ESP8266WebServer对象，对象名称为esp8266_server


void setup() {
  // 初始化 LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // 默认关闭 LED
  
  // 开机提示：LED 长亮 3 秒
  digitalWrite(LED_PIN, LOW); // 点亮 LED
  delay(3000);
  digitalWrite(LED_PIN, HIGH); // 关闭 LED

  // 初始化舵机
  myservo.attach(SERVO_PIN,500,2500); // 将舵机连接到 D4,脉宽范围在0.5ms到2.5ms之间
  myservo.write(90);         // 初始位置为 90 度

  // 初始化串口用于蓝牙通信
  Serial.begin(BT_BAUDRATE); 
  Serial.println("ESP8266 is ready to communicate with Bluetooth module.");

  setServoAngle(90);//将初始舵机角度设置为水平

  // 默认关闭 WiFi
  // disableWiFi();
  
  // 默认开启 WiFi
  enableWiFi();
  InitServer();
}

void loop() {
  if (Serial.available()) {
    String message = Serial.readString(); // 接收蓝牙信息
    Serial.println("Received: " + message);

    // 根据接收到的指令执行不同操作
    if (message == "s1") { //开灯
      openLight();
    } else if (message == "s0") { //关灯
      closeLight();
    } else if (message == "w0") {
      disableWiFi();       // 关闭 WiFi 模块
      blinkLED(1);         // LED 闪烁 1 次
    } else if (message == "w1") {
      enableWiFi();        // 开启 WiFi 模块
      InitServer();
      blinkLED(3);         // LED 闪烁 3 次
    } else if (message == "test") {
      breathingLED(30);    // 呼吸灯模式 30 秒
    } else if (message == "whoareyou") {
      Serial.println("G501666"); // 回复固定字符串
    } 
    // else if (message == "testservo") {
    //   testServo();
    // } //为了避免不小心触发该模式而导致已经固定好的舵机损坏，故正式使用时去掉这个入口。仅供调试使用。
  }
  esp8266_server.handleClient();

  if (disableControl && millis() > disableTime) {
  disableControl = false;  // 恢复接收指令
  Serial.println("Control enabled again.");
  }
}

// 调整舵机角度
void setServoAngle(int angle) {
  myservo.write(angle); // 调整舵机到指定角度
  delay(500);           // 给舵机一些时间完成动作
}

void openLight(){
  if (disableControl)
  {
    Serial.println("Cooling ~ Control disabled temporarily ~");
    return ;
  }

  setServoAngle(125);  // 调整舵机到 120 度
  delay(100);
  setServoAngle(90);

  disableControl = true;
  disableTime = millis()+coolTime;
}

void closeLight(){
  if (disableControl)
  {
    Serial.println("Cooling ~ Control disabled temporarily ~");
    return ;
  }

  setServoAngle(65);   // 调整舵机到 0 度
  delay(100);
  setServoAngle(90);

  disableControl = true;
  disableTime = millis()+coolTime;
}

// 开启 WiFi 模块
void enableWiFi() {
  if (!wifiEnabled) {
    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // 替换为实际的 WiFi 名称和密码
    Serial.println("WiFi enabled. Connecting to ");Serial.print(WIFI_SSID);
    
    // 持续检测连接状态，超时时间为 10 秒
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500); // 每 500ms 检测一次
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP()); // 输出分配的 IP 地址
      wifiEnabled = true;
    } else {
      Serial.println("\nWiFi connection failed.");
      wifiEnabled = false; // 确保状态一致
    }
  }
}

void InitServer(){
  esp8266_server.begin();                   //  开启web服务器
  esp8266_server.on("/", handleRoot);       //  访问根路由即调用 handlerRoot 函数处理
  esp8266_server.on("/OPENLIGHT", HTTP_POST, openLightWEB);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/CLOSELIGHT", HTTP_POST, closeLightWEB);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.onNotFound(handleNotFound);        
  Serial.println("HTTP esp8266_server started"); //  告知用户ESP8266网络服务功能已经启动
}


void handleRoot() {   //处理网站根目录“/”的访问请求 
  String HTML="<!DOCTYPE html>"
              "<html lang=\"en\">"
              "<head>"
              "    <meta charset=\"UTF-8\">"
              "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
              "    <title>ESP8266 Control Panel</title>"
              "    <style>"
              "        body {"
              "            font-family: Arial, sans-serif;"
              "            background-color: #f4f4f4;"
              "            margin: 0;"
              "            padding: 0;"
              "        }"
              ""
              "        header {"
              "            background-color: #333;"
              "            color: white;"
              "            padding: 10px 0;"
              "            text-align: center;"
              "        }"
              ""
              "        .container {"
              "            width: 80%;"
              "            margin: 20px auto;"
              "            background-color: #fff;"
              "            padding: 20px;"
              "            border-radius: 8px;"
              "            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);"
              "        }"
              ""
              "        button {"
              "            background-color: #4CAF50;"
              "            color: white;"
              "            border: none;"
              "            padding: 15px 32px;"
              "            text-align: center;"
              "            text-decoration: none;"
              "            display: inline-block;"
              "            font-size: 16px;"
              "            border-radius: 5px;"
              "            cursor: pointer;"
              "            transition: background-color 0.3s ease;"
              "        }"
              ""
              "        button:hover {"
              "            background-color: #45a049;"
              "        }"
              ""
              "        .disabled {"
              "            background-color: #ccc;"
              "            cursor: not-allowed;"
              "        }"
              ""
              "        .overlay {"
              "            display: none;"
              "            position: fixed;"
              "            top: 0;"
              "            left: 0;"
              "            width: 100%;"
              "            height: 100%;"
              "            background-color: rgba(0, 0, 0, 0.5);"
              "            z-index: 1000;"
              "            justify-content: center;"
              "            align-items: center;"
              "            color: white;"
              "            font-size: 20px;"
              "        }"
              ""
              "        .LightBtn {"
              "            background-color: #4CAF50; /* 按钮背景颜色 */"
              "            color: white; /* 按钮文字颜色 */"
              "            padding: 10px 20px; /* 按钮内边距 */"
              "            border: none; /* 移除边框 */"
              "            border-radius: 5px; /* 圆角边框 */"
              "            cursor: pointer; /* 鼠标悬停时显示手型 */"
              "            font-size: 16px; /* 字体大小 */"
              "            transition: background-color 0.3s; /* 背景颜色过渡效果 */"
              "            margin-right: 10px; /* 按钮之间的间距 */"
              "        }"
              ""
              "        /* 鼠标悬停时的样式 */"
              "        .LightBtn:hover {"
              "            background-color: #45a049; /* 悬停时的背景颜色 */"
              "        }"
              ""
              "        /* 按钮按下时的样式 */"
              "        .LightBtn:active {"
              "            background-color: #398439; /* 按下时的背景颜色 */"
              "            transform: translateY(2px); /* 按下时稍微向下移动 */"
              "        }"
              ""
              "        .LightBtn {"
              "            display: inline-block; /* 设置为行内块元素 */"
              "        }"
              ""
              "        .inlineform {"
              "            display: inline-block; /* 设置为行内块元素 */"
              "        }"
              "    </style>"
              "</head>"
              "<body>"
              ""
              "<header>"
              "    <h1>ESP8266 Control Panel</h1>"
              "</header>"
              ""
              "<div class=\"container\">"
              "    <h2>Control Light</h2>"
              "    <form action=\"/OPENLIGHT\" method=\"POST\" class=\"inlineform\">"
              "        <button type=\"submit\" class=\"LightBtn\" id=\"openlightBtn\">Open Light</button>"
              "    </form>"
              "    <form action=\"/CLOSELIGHT\" method=\"POST\" class=\"inlineform\">"
              "        <button type=\"submit\" class=\"LightBtn\" id=\"closelightBtn\">Close Light</button>"
              "    </form>"
              "</div>"
              ""
              "<div class=\"overlay\" id=\"overlay\">"
              "    <p>Operating ~ Please wait...</p>"
              "</div>"
              "";
  if (disableControl)
  {
    HTML+="<script>"
          "    document.getElementById(\"openlightBtn\").classList.add(\"disabled\");"
          "    document.getElementById(\"closelightBtn\").classList.add(\"disabled\");"
          "    document.getElementById(\"overlay\").style.display = \"flex\";"
          "    setTimeout(() => {"
          "        document.getElementById(\"openlightBtn\").classList.remove(\"disabled\");"
          "        document.getElementById(\"closelightBtn\").classList.remove(\"disabled\");"
          "        document.getElementById(\"overlay\").style.display = \"none\";"
          "    }, 5000);"
          "</script>";
  }
  HTML += "</body>"
          "</html>";
  esp8266_server.send(200, "text/html", HTML);
  // esp8266_server.send(200, "text/plain" , "Hello from ESP8266!");
}

// 设置处理404情况的函数'handleNotFound'
void handleNotFound(){                                        // 当浏览器请求的网络资源无法在服务器找到时，
  esp8266_server.send(404, "text/plain", "404: Not found");   // NodeMCU将调用此函数。
}

// 关闭 WiFi 模块
void disableWiFi() {
  if (wifiEnabled) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi disabled.");
    wifiEnabled = false;
  }
}

void openLightWEB(){
  openLight();
  esp8266_server.sendHeader("Location","/");
  esp8266_server.send(303);
}

void closeLightWEB(){
  closeLight();
  esp8266_server.sendHeader("Location","/");
  esp8266_server.send(303);
}

// LED 闪烁功能
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW);  // 点亮 LED
    delay(200);
    digitalWrite(LED_PIN, HIGH); // 熄灭 LED
    delay(200);
  }
}

// 呼吸灯模式
void breathingLED(int duration) {
  int dir = 0;
  int pwmval = 750;
  unsigned long startTime = millis();
  analogWriteFreq(1000); // 设置 PWM 频率为 1kHz
  analogWriteRange(1000);

  while (millis() - startTime < duration * 1000) {
    if (dir) pwmval++;
    else pwmval--;
    if (pwmval <= 500) dir = 1;
    if (pwmval >= 1000) dir = 0;
    analogWrite(LED_PIN, pwmval); // 调整占空比
    if (pwmval == 1000) delay(300);
    delay(3);
  }
  digitalWrite(LED_PIN, HIGH); // 恢复 LED 为关闭状态
}

void testServo(){
  int pos=0;
  setServoAngle(pos);//初始化一下舵机位置
  delay(100);
  for (pos=0;pos<=180;pos+=10)
  {
    setServoAngle(pos);
    Serial.print(pos);Serial.print("\n");
    delay(15);
  }
  for (pos=180;pos>=0;pos-=10)
  {
    setServoAngle(pos);
    Serial.print(pos);Serial.print("\n");
    delay(15);
  }
  Serial.println("Servo Test Done.");
}
