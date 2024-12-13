
/////newest lab
/*
2) Write a displayTime(long unsigned int inTime) function which takes a copy of your global
time count as its input argument. Your function should convert the time in seconds
that was passed in to Month, Day, Hour, Minutes and Seconds. It should then create
ascii array(s) and display date and time to the LCD in the format specified Step 6
below. Explain why it is important to pass a copy of the time into the function rather
than just using the global variable.

3) Write a displayTemp(float inAvgTempC) function which take a copy of the floating point
temperature in C and displays temperature in both C and F to the LCD as specified in
Step 6 below. This function is just a display function. It can convert C to F but it should
not do the averaging of past temperature readings described below.
*/

#include <msp430.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stdio.h"
#include "peripherals.h"


#define CALADC12_15V_30C *((unsigned int *)0x1A1A)
#define CALADC12_15V_85C *((unsigned int *)0x1A1C)
const int BUFFER_SIZE = 36;
unsigned int in_temp;

long startTime = 0;
int startMonth = 2;
int currMonth = 2;
int startDay = 1;
int startYear = 2003;

int startingScroll;
float prollConst = 1;
unsigned long currInterruptBlock = 0;
unsigned int in_value;
volatile float temperatureDegC;
volatile float temperatureDegF;
volatile float degC_per_bit;
double tempSum = 0;
double buffer[BUFFER_SIZE] = {0};

volatile unsigned int bits30, bits85;

int months;

long increaseYears = 0;
long increaseMonths = 0;
long increaseDays = 0;
long increaseHours = 0;
long increaseMin = 0;
long increaseSec = 0;
unsigned long editTime = 0;




enum State{
    DEFAULT,
    EDIT_MODE
};

enum Time{
    YEAR,
    MONTH,
    DAY,
    HOUR,
    MIN,
    SEC
};

enum Time selectedTime = MONTH;

enum State currState = DEFAULT;

//set up timer
void runtimerA2(void)
{
TA2CTL = TASSEL_1 + MC_1 + ID_0;
TA2CCR0 = 32767; //1 second count no leap count nessecary poggers!
TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
}

void configureButtons() {
    // Ensure pins are set to IO (0)
    P7SEL &= ~BIT0;
    P3SEL &= ~BIT6;
    P2SEL &= ~BIT2;
    P7SEL &= ~BIT4;

    //set the pins P7.0, P3.6, P2.2, and P7.4 as inputs (0)
    P7DIR &= ~BIT0;
    P3DIR &= ~BIT6;
    P2DIR &= ~BIT2;
    P7DIR &= ~BIT4;

    //enable pull resistors on P7.0, P3.6, P2.2, and P7.4 (1)
    P7REN |= BIT0;
    P3REN |= BIT6;
    P2REN |= BIT2;
    P7REN |= BIT4;

    //set the pull-up resistors (1)
    P7OUT |= BIT0;
    P3OUT |= BIT6;
    P2OUT |= BIT2;
    P7OUT |= BIT4;
}

//reads buttons and outputs them as buttonState
char readButtons() {
    int buttonState = 0;

    //check each button and update the corresponding bit
    if ((P7IN & BIT0) == 0) {
        buttonState |= 0x08;  //S1
    }
    if ((P3IN & BIT6) == 0) {
        buttonState |= 0x04;  //S2
    }
    if ((P2IN & BIT2) == 0) {
        buttonState |= 0x02;  //S3
    }
    if ((P7IN & BIT4) == 0) {
        buttonState |= 0x01;  //S4
    }

    return buttonState;
}

void intToAscii(int number, char* asciiString) {
    int buffer[5];
    int maxIndex = 0;
    int currIndex = 0;
    while (number >= 10){
        buffer[maxIndex] = number % 10;
        number /= 10;
        maxIndex += 1;
    }
    buffer[maxIndex] = number;
    int i;
    int k = 0;
    for (i = maxIndex; i >= 0; i-- ){
        asciiString[k] = '0' + buffer[i];
        k++;
    }
    asciiString[maxIndex + 1] = '\0';
}



void displayTime(unsigned long inTime){
    int years = (inTime / 31536000);
    inTime = inTime % 31536000;
    years += startYear;
    int daysInMonth = 31;
    int days = (inTime / 86400);
    inTime = inTime % 86400;
    days += startDay;
    switch (currMonth){
        case 4:
        case 9:
        case 6:
        case 11:
            daysInMonth = 30;
            break;
        case 2:
            daysInMonth = 28;
            break;
    }
    while (days > daysInMonth){
        days -= daysInMonth;
        currMonth += 1;
        if(currMonth > 12){
            startYear++;
            currMonth = 1;
        }
        switch (currMonth){
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                daysInMonth = 31;
                break;
            case 4:
            case 9:
            case 6:
            case 11:
                daysInMonth = 30;
                break;
            case 2:
                daysInMonth = 28;
                break;
        }
    }
    months = currMonth;
    int hours = (inTime / 3600);
    inTime = inTime % 3600;
    int minutes = (inTime / 60);
    inTime = inTime % 60;
    int seconds = inTime;



    uint8_t asciiStringYear[5];
    uint8_t asciiStringMonth[5];
    uint8_t asciiStringDay[5];
    uint8_t asciiStringHour[5];
    uint8_t asciiStringSec[5];
    uint8_t asciiStringMin[5];
    intToAscii(years, asciiStringYear);
    intToAscii(months, asciiStringMonth);
    intToAscii(days, asciiStringDay);
    intToAscii(hours, asciiStringHour);
    intToAscii(minutes, asciiStringMin);
    intToAscii(seconds, asciiStringSec);
    Graphics_drawStringCentered(&g_sContext, asciiStringYear, AUTO_STRING_LENGTH, 48, 30, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, asciiStringMonth, AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, asciiStringDay, AUTO_STRING_LENGTH, 48, 50, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, asciiStringHour, AUTO_STRING_LENGTH, 48, 60, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, asciiStringMin, AUTO_STRING_LENGTH, 48, 70, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, asciiStringSec, AUTO_STRING_LENGTH, 48, 80, TRANSPARENT_TEXT);

}

void displayTempDefault(float inAvgTempC){
    int tempC = inAvgTempC;
    int tempF = (inAvgTempC * 9/5) + 32;

    uint8_t tempCMessage[5];
    uint8_t tempFMessage[5];
    intToAscii(tempC, tempCMessage);
    intToAscii(tempF, tempFMessage);
    uint8_t newCMessage[8];
    uint8_t newFMessage[8];
    newCMessage[0] = 'C';
    newCMessage[1] = ':';
    newCMessage[2] = ' ';
    newFMessage[0] = 'F';
    newFMessage[1] = ':';
    newFMessage[2] = ' ';
    int i;
    for (i = 3; i < 8; i++){
        newCMessage[i] = tempCMessage[i-3];
        newFMessage[i] = tempFMessage[i-3];
    }


    if(true){
        Graphics_drawStringCentered(&g_sContext, newCMessage, AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, newFMessage, AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
    }
}

//this function is from their example
void tempADCInit(){
    REFCTL0 &= ~REFMSTR; // Reset REFMSTR to hand over control of
    // internal reference voltages to
    // ADC12_A control registers
    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON; // Internal ref = 1.5V
    ADC12CTL1 = ADC12SHP; // Enable sample timer
    // Using ADC12MEM0 to store reading
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10; // ADC i/p ch A10 = temp sense
    // ACD12SREF_1 = internal ref = 1.5v
    __delay_cycles(100); // delay to allow Ref to settle
    ADC12CTL0 |= ADC12ENC; // Enable conversion
    // Use calibration data stored in info memory
    bits30 = CALADC12_15V_30C;
    bits85 = CALADC12_15V_85C;
    degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));

}

void scrollADCInit(){
        P8SEL &= ~BIT0;
        P8DIR |= BIT0;
        P8OUT |= BIT0;

        // Scroll Wheel (P6.0), Input Channel A0
        P6SEL |= BIT0;
        // ADC Input (P6.1), Input Channel A1
        P6SEL |= BIT1;
        // Reset REFMSTR to hand over control of internal reference voltages to ADC12_A control registers
        REFCTL0 &= ~REFMSTR;

        // Vref+ to be AVcc = 3.3V and Vref+=GND
        ADC12CTL0 = ADC12SHT0_9 | ADC12ON;
        ADC12CTL1 = ADC12SHP; // Enable sample timer
        ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_0; // Scroll Wheel

    //    ADC12MCTL1 = ADC12SREF_0 | ADC12INCH_1; // ADC Input
        __delay_cycles(100); // delay to allow ref to settle
        ADC12CTL0 |= ADC12ENC; // Enable conversion

}

int scrollMain(){
    // Set Channel to Read
    ADC12CTL1 = ADC12CSTARTADD_1 | ADC12SHP;

    // Start Bit
    ADC12CTL0 &= ~ADC12SC;
    ADC12CTL0 |= ADC12SC;

    while (ADC12CTL1 & ADC12BUSY)
    {
        __no_operation();
    }

    int codeValue = ADC12MEM0;
    return ((float) codeValue / 4095) * 3.3;
}

//this function is from there example
void readTemp(){
    ADC12CTL0 &= ~ADC12SC; // clear the start bit
    ADC12CTL0 |= ADC12SC; // Sampling and conversion start
    // Single conversion (single channel)
    // Poll busy bit waiting for conversion to complete
    while (ADC12CTL1 & ADC12BUSY)
    __no_operation();
    in_temp = ADC12MEM0; // Read in results if conversion
    // Temperature in Celsius. See the Device Descriptor Table section in the
    // System Resets, Interrupts, and Operating Modes, System Control Module
    // chapter in the device user's guide for background information on the
    // used formula.
    temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit + 30.0;
}





double movingAverage(double newTemp){

    double currentC = newTemp;
    int index = currInterruptBlock % BUFFER_SIZE;

    tempSum -= buffer[index];
    buffer[index] = currentC;
    tempSum += currentC;

    double averageDegrees = tempSum / ((float)(BUFFER_SIZE));

    return averageDegrees;

}

void initAverage(){
    int i;
    for (i=0; i < BUFFER_SIZE; i++){
        buffer[i] = temperatureDegC;
        tempSum += temperatureDegC;
    }
}







void main(void)

{


    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
    initLeds();
    configDisplay();
    configKeypad();
    configureButtons();
    setLeds(0);
    runtimerA2();
    Graphics_clearDisplay(&g_sContext);
    Graphics_flushBuffer(&g_sContext);


    tempADCInit();

    _BIS_SR(GIE); //enable interrupts

    readTemp();
    initAverage();


    while(1)
    {
        switch (currState){
            case DEFAULT:


                    readTemp();
                    if (currInterruptBlock % 3 == 0){
                        Graphics_clearDisplay(&g_sContext);
                        displayTempDefault(movingAverage(temperatureDegC));
                        displayTime(currInterruptBlock);
                        Graphics_flushBuffer(&g_sContext);
                    }


                    int buttonState = readButtons();
                    if(buttonState & 0x08){
                        currState = EDIT_MODE;
                        startingScroll = 1;
                        editTime = currInterruptBlock;
                        scrollADCInit();
                    }



                break;
            case EDIT_MODE:

                    //display temp as normal
                    Graphics_clearDisplay(&g_sContext);
                    displayTime((unsigned long)((signed long)(editTime) + increaseYears + increaseMonths + increaseDays + increaseHours + increaseMin + increaseSec));
                    if(selectedTime == YEAR){
                        Graphics_drawStringCentered(&g_sContext, "YEAR", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
                    }else if (selectedTime == MONTH)
                    {
                        Graphics_drawStringCentered(&g_sContext, "MONTH", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
                    }else if (selectedTime == DAY)
                    {
                        Graphics_drawStringCentered(&g_sContext, "DAY", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
                    }else if (selectedTime == HOUR)
                    {
                        Graphics_drawStringCentered(&g_sContext, "HOUR", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
                    }else if (selectedTime == MIN)
                    {
                        Graphics_drawStringCentered(&g_sContext, "MIN", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
                    }else if (selectedTime == SEC)
                    {
                        Graphics_drawStringCentered(&g_sContext, "SEC", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
                    }
                    Graphics_flushBuffer(&g_sContext);


                    int proll = scrollMain();
                    int daysInMonth;
                    int increasingMonths;

                    switch (selectedTime){
                        case YEAR:
                            increaseYears = (long)((proll - startingScroll) * prollConst) * 31536000;
                            int buttonState = readButtons();
                            if(buttonState & 0x08){
                                selectedTime = MONTH;
                                startingScroll = 1;
                            }
                            if(buttonState & 0x01){
                                currState = DEFAULT;
                                currInterruptBlock = (unsigned long)((signed long)(editTime) + increaseYears + increaseMonths + increaseDays + increaseHours + increaseMin + increaseSec);
                                editTime = 0;
                                increaseYears = 0;
                                increaseMonths = 0;
                                increaseDays = 0;
                                increaseHours = 0;
                                increaseMin = 0;
                                increaseSec = 0;

                                tempADCInit();
                            }
                            break;
                        case MONTH:
                            daysInMonth = 31;
                            switch (currMonth){
                                case 4:
                                case 9:
                                case 6:
                                case 11:
                                    daysInMonth = 30;
                                    break;
                                case 2:
                                    daysInMonth = 28;
                                    break;
                            }
                            long increasingSecondsFromMonths = (long)((proll - startingScroll) * prollConst) * daysInMonth * 86400;
                            increaseMonths = increasingSecondsFromMonths;
                            buttonState = readButtons();
                            if(buttonState & 0x08){
                                selectedTime = DAY;
                                startingScroll = 1;
                            }
                            if(buttonState & 0x01){
                                currState = DEFAULT;
                                currInterruptBlock =  (unsigned long)((signed long)(editTime) + increaseYears + increaseMonths + increaseDays + increaseHours + increaseMin + increaseSec);
                                editTime = 0;
                                increaseYears = 0;
                                increaseMonths = 0;
                                increaseDays = 0;
                                increaseHours = 0;
                                increaseMin = 0;
                                increaseSec = 0;
                                tempADCInit();
                            }
                            break;
                        case DAY:
                            increaseDays = (long)((proll - startingScroll) * prollConst) * 86400;
                            buttonState = readButtons();
                            if(buttonState & 0x08){
                                selectedTime = HOUR;
                                startingScroll = 1;
                            }
                            if(buttonState & 0x01){
                                currState = DEFAULT;
                                currInterruptBlock =  (unsigned long)((signed long)(editTime) + increaseYears + increaseMonths + increaseDays + increaseHours + increaseMin + increaseSec);
                                editTime = 0;
                                increaseYears = 0;
                                increaseMonths = 0;
                                increaseDays = 0;
                                increaseHours = 0;
                                increaseMin = 0;
                                increaseSec = 0;
                                tempADCInit();
                            }
                            break;
                        case HOUR:
                            increaseHours =  (long)((proll - startingScroll) * prollConst) * 3600;
                            buttonState = readButtons();
                            if(buttonState & 0x08){
                                selectedTime = MIN;
                                startingScroll = 1;
                            }
                            if(buttonState & 0x01){
                                currState = DEFAULT;
                                currInterruptBlock =  (unsigned long)((signed long)(editTime) + increaseYears + increaseMonths + increaseDays + increaseHours + increaseMin + increaseSec);
                                editTime = 0;
                                increaseYears = 0;
                                increaseMonths = 0;
                                increaseDays = 0;
                                increaseHours = 0;
                                increaseMin = 0;
                                increaseSec = 0;
                                tempADCInit();
                            }
                            break;
                        case MIN:
                            increaseMin = (long)((proll - startingScroll) * prollConst) * 60;
                            buttonState = readButtons();
                            if(buttonState & 0x08){
                                selectedTime = SEC;
                                startingScroll = 1;
                            }
                            if(buttonState & 0x01){
                                currState = DEFAULT;
                                currInterruptBlock =  (unsigned long)((signed long)(editTime) + increaseYears + increaseMonths + increaseDays + increaseHours + increaseMin + increaseSec);
                                editTime = 0;
                                increaseYears = 0;
                                increaseMonths = 0;
                                increaseDays = 0;
                                increaseHours = 0;
                                increaseMin = 0;
                                increaseSec = 0;
                                tempADCInit();
                            }
                            break;
                        case SEC:
                           increaseSec = (long)((proll - startingScroll) * prollConst);
                            buttonState = readButtons();
                            if(buttonState & 0x08){
                                selectedTime = YEAR;
                                startingScroll = 1;
                            }
                            if(buttonState & 0x01){
                                currState = DEFAULT;
                                currInterruptBlock =  (unsigned long)((signed long)(editTime) + increaseYears + increaseMonths + increaseDays + increaseHours + increaseMin + increaseSec);
                                editTime = 0;
                                increaseYears = 0;
                                increaseMonths = 0;
                                increaseDays = 0;
                                increaseHours = 0;
                                increaseMin = 0;
                                increaseSec = 0;
                                tempADCInit();
                            }
                            break;

                    }

                break;
        }

    }


}



#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER_A2_ISR(void){
    currInterruptBlock++;
}
