//---PWM---
#define blue_seg_1 10
#define blue_seg_2 6
#define blue_seg_3 5
#define blue_seg_4 9

#define yellow_seg_1 11

#define red_seg_1 3
//---------

#define green_seg_1 A1

#define brightnessPot A5
#define modeButton 2

const char PWMsegments[6] = {blue_seg_1, blue_seg_2, blue_seg_3, blue_seg_4, yellow_seg_1, red_seg_1};

volatile char mode = 0;

void modeChange(){
    //---Turning all segments off---
    for(int i = 0; i < 6; i++){
        analogWrite(PWMsegments[i], 0);
    }

    pinMode(green_seg_1, INPUT);
    //------

    mode++;
    mode %= 6;
}

float readBrightness(){
    return (float)analogRead(brightnessPot) / 1024.f;
}

void greenSegPWMCycle(int duty){
    //Duty is a value from 0 to 255, like in analogWrite function
    //490Hz => 1 cycle per 2040 ms

    //2040 / 255 = 8
    int onTime = 8 * duty;
    int offTime = 2040 - onTime;
    //Pin switches PNP transistor therefore I'm switching between sinking current and high impedance mode.
    pinMode(green_seg_1, OUTPUT);
    delayMicroseconds(onTime);
    pinMode(green_seg_1, INPUT);
    delayMicroseconds(offTime);
}

//---Fade functions---
//TODO move this directly to serial and parallel fade
//so it can easier be broken by interrupt
void fadeInGreenSeg(unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        float brightness = readBrightness();
        greenSegPWMCycle(((currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }
}

void fadeOutGreenSeg(unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        float brightness = readBrightness();
        greenSegPWMCycle((255 - (currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }
}

void fadeIn(char PWM_pin, unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        float brightness = readBrightness();
        analogWrite(PWM_pin, ((currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }
}

void fadeOut(char PWM_pin, unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        float brightness = readBrightness();
        analogWrite(PWM_pin, (255 - (currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }
}
//---

void serialFade(unsigned long fadeTime){
    for(char i = 0; i < 6; i++){
        fadeIn(PWMsegments[i], fadeTime);
        fadeOut(PWMsegments[i], fadeTime);
    }

    fadeInGreenSeg(fadeTime);
    fadeOutGreenSeg(fadeTime);
}

void parallelFade(unsigned long fadeTime){
    unsigned long startTime = millis();
    unsigned long currentTime = 0;

    //Fade in
    while(currentTime <= fadeTime / 2){
        for(char i = 0; i < 6; i++){
            float brightness = (float)analogRead(brightnessPot) / 1024.f;
            int duty = ((currentTime / (float)fadeTime) * 255.f) * brightness;
            
            analogWrite(PWMsegments[i], duty);
            greenSegPWMCycle(duty);
        }

        currentTime = millis() - startTime;
    }

    //Fade out
    while(currentTime <= fadeTime){
        for(char i = 0; i < 6; i++){
            float brightness = (float)analogRead(brightnessPot) / 1024.f;
            int duty = (255 - (currentTime / (float)fadeTime) * 255.f) * brightness;
            
            analogWrite(PWMsegments[i], duty);
            greenSegPWMCycle(duty);
        }

        currentTime = millis() - startTime;
    }
}

void randomFade(unsigned long fadeTime){
    char randomSeg = random(0, 7);

    if(randomSeg == 7){
        fadeInGreenSeg(fadeTime / 2);
        fadeOutGreenSeg(fadeTime / 2);
    }
    else{
        fadeIn(PWMsegments[randomSeg], fadeTime / 2);
        fadeOut(PWMsegments[randomSeg], fadeTime / 2);
    }
}

void setup(){
    pinMode(brightnessPot, INPUT);
    pinMode(green_seg_1, INPUT);

    pinMode(modeButton, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(mode), modeChange, FALLING);
}

void loop(){
    switch(mode){
        case 0:
            serialFade(600);
            break;

        case 1:
            serialFade(300);
            break;

        // case 2:
        //     //serialFade(300);
        //     break;

        // case 3:
        //     //parallelFade(600);
        //     break;

        // case 4:
        //     //parallelFade(300);
        //     break;

        // case 5:
        //     // randomFade(1200);
        //     break;
    }
}
