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

void resetSegments(){
    for(int i = 0; i < 6; i++){
        analogWrite(PWMsegments[i], 0);
    }
    pinMode(green_seg_1, INPUT);
}

bool buttonPressed = false;
bool modeChangeInterrupt(){
    if(digitalRead(modeButton) && buttonPressed) buttonPressed = false;

    if(!digitalRead(modeButton) && !buttonPressed){
        mode++;
        mode %= 7;

        buttonPressed = true;

        resetSegments();

        delay(400);

        return true;
    }

    return false;
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
//Returns true if the whole fade cycle was made without interrupt
//Returns false otherwise
bool fadeInGreenSeg(unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        if(modeChangeInterrupt()) return false;

        float brightness = readBrightness();
        greenSegPWMCycle(((currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }

    return true;
}

bool fadeOutGreenSeg(unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        if(modeChangeInterrupt()) return false;

        float brightness = readBrightness();
        greenSegPWMCycle((255 - (currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }

    return true;
}

bool fadeIn(char PWM_pin, unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        if(modeChangeInterrupt()) return false;

        float brightness = readBrightness();
        analogWrite(PWM_pin, ((currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }

    return true;
}

bool fadeOut(char PWM_pin, unsigned long fadeTime){
    unsigned long startTime = millis();
    
    unsigned long currentTime = 0;

    while(currentTime <= fadeTime){
        if(modeChangeInterrupt()) return false;

        float brightness = readBrightness();
        analogWrite(PWM_pin, (255 - (currentTime / (float)fadeTime) * 255.f) * brightness);

        currentTime = millis() - startTime;
    }

    return true;
}
//---

void stillDisplay(){
    for(char i = 0; i < 6; i++){
        if(modeChangeInterrupt()) return;

        float brightness = readBrightness();

        analogWrite(PWMsegments[i], 255.f * brightness);
        greenSegPWMCycle(255.f * brightness);
    }
}

void serialFade(unsigned long fadeTime){
    for(char i = 0; i < 6; i++){
        if(!fadeIn(PWMsegments[i], fadeTime) || !fadeOut(PWMsegments[i], fadeTime)) return;
    }

    if(!fadeInGreenSeg(fadeTime) || !fadeOutGreenSeg(fadeTime)) return;
}

void parallelFade(unsigned long fadeTime){
    unsigned long startTime = millis();
    unsigned long currentTime = 0;

    //Fade in
    while(currentTime <= fadeTime / 2){
        for(char i = 0; i < 6; i++){
            if(modeChangeInterrupt()) return;

            float brightness = readBrightness();
            int duty = ((currentTime / (float)fadeTime) * 255.f) * brightness;
            
            analogWrite(PWMsegments[i], duty);
            greenSegPWMCycle(duty);
        }

        currentTime = millis() - startTime;
    }

    //Fade out
    while(currentTime <= fadeTime){
        for(char i = 0; i < 6; i++){
            if(modeChangeInterrupt()) return;

            float brightness = (float)analogRead(brightnessPot) / 1024.f;
            int duty = (255 - (currentTime / (float)fadeTime) * 255.f) * brightness;
            
            analogWrite(PWMsegments[i], duty);
            greenSegPWMCycle(duty);
        }

        currentTime = millis() - startTime;
    }

    resetSegments();

    delay(500);
}

void randomFade(unsigned long fadeTime){
    char randomSeg = random(0, 7);

    if(randomSeg == 6){
        if(!fadeInGreenSeg(fadeTime / 2) || !fadeOutGreenSeg(fadeTime / 2)) return;
    }
    else{
        if(!fadeIn(PWMsegments[randomSeg], fadeTime / 2) || !fadeOut(PWMsegments[randomSeg], fadeTime / 2)) return;
    }
}

void setup(){
    pinMode(brightnessPot, INPUT);
    pinMode(green_seg_1, INPUT);

    pinMode(modeButton, INPUT_PULLUP);

    randomSeed(analogRead(0));
}

void loop(){
    switch(mode){
        case 0:
            stillDisplay();
            break;

        case 1:
            serialFade(600);
            break;

        case 2:
            serialFade(300);
            break;

        case 3:
            parallelFade(2500);
            break;

        case 4:
            parallelFade(1250);
            break;

        case 5:
            randomFade(1200);
            break;

        case 6:
            randomFade(600);
            break;
    }
}
