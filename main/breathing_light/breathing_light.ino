/*
  ESP8266 使用PWM实现LED呼吸灯
*/

void setup() {
  analogWriteFreq(1000);            // 频率设置为1kHz，即周期为1ms
  analogWriteRange(1000);           // 范围设置为1000，即占空比步长为1us
  analogWrite(2, 750);              // GPIO2 - D4 - LED
}

void loop() {
  uint8_t dir = 0;
  uint32_t pwmval = 750;
  while(1)
  {
    if(dir) pwmval++;                 // dir==1  pwmval递增
    else pwmval--;                    // dir==0  pwmval递减
    if( pwmval <= 500 ) dir=1;        // pwmval降低至500后，方向为递增
    if( pwmval == 1000) dir=0;        // pwmval递增到1000后，方向改为递减
    analogWrite(2, pwmval);           // 修改占空比
    if( pwmval==1000 ) delay(300);    // 在LED熄灭时等待300ms
    delay(3);
  }
}
