/*
 * Ticker库的基本使用
 * 
 * 每个Ticker需要建立一个回调函数，当定时时间到了后，回调函数将被调用；
 * 理论上可以新建足够多的Ticker对象，但这会受到内存容量的限制
 *
 * 使用Ticker实现LED闪烁，并查询定时器存活状态。
*/

#include <Ticker.h>                       // 使用Ticker库，需要包含头文件

Ticker timer1;                            // 创建一个定时器对象

void setup() {
  Serial.begin(9600);                     // 初始化串口
  
  pinMode(LED_BUILTIN, OUTPUT);           // 设置LED引脚为输出引脚
  digitalWrite(LED_BUILTIN, LOW);         // 设置引脚为低电平，点亮LED

  /* 设置周期性定时0.5s，即500ms，回调函数为timer1_cb，参数为LED引脚号，并启动定时器 */
  timer1.attach(0.5, timer1_cb, LED_BUILTIN);
}

void loop() {
  /* 检测定时器是否为存活状态 */
  if(timer1.active()){
    Serial.println("timer1 is active.");
  }
  else{
    Serial.println("timer1 is not active!");
  }
  delay(100);
}

void timer1_cb(int led_pin) 
{
  int state = digitalRead(led_pin);  // 获取当前led引脚状态
  digitalWrite(led_pin, !state);     // 翻转LED引脚电平
}
