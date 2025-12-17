#include "driverlib.h"
#include "device.h"
#include "board.h"
#include <stdio.h>

// ============================================================================
//  전역 변수 선언
// ============================================================================

// ---------------------- 초음파 (ECAP) ----------------------
volatile uint32_t g_echoRise = 0;
volatile uint32_t g_echoFall = 0;
volatile uint32_t g_echoDelta = 0;
volatile uint16_t g_distance_mm = 0;
volatile uint16_t g_measureDone = 0;
volatile uint16_t g_ultraTick = 0;

#define DIST_NUM 17UL
#define DIST_DEN 10000UL

// ---------------------- 작은별 멜로디 ----------------------
#define NOTE_C4  262U
#define NOTE_D4  294U
#define NOTE_E4  330U
#define NOTE_F4  349U
#define NOTE_G4  392U
#define NOTE_A4  440U
#define NOTE_B4  494U
#define NOTE_C5  523U
#define NOTE_REST 0U

typedef struct { uint16_t f, t; } Note;

#define Q 500U   // 1/4 박자
#define H 1000U  // 1/2 박자

const Note melody[] = {
    {NOTE_C4,Q},{NOTE_C4,Q},{NOTE_G4,Q},{NOTE_G4,Q},
    {NOTE_A4,Q},{NOTE_A4,Q},{NOTE_G4,H},
    {NOTE_F4,Q},{NOTE_F4,Q},{NOTE_E4,Q},{NOTE_E4,Q},
    {NOTE_D4,Q},{NOTE_D4,Q},{NOTE_C4,H},
};
#define MELODY_LEN (sizeof(melody)/sizeof(melody[0]))

volatile uint16_t g_noteIndex = 0;
volatile uint32_t g_noteRemain = 0;

// ---------------------- ADC / CAN ----------------------
volatile uint16_t adc1_raw = 0;
volatile uint16_t adc2_raw = 0;

uint16_t txBuf[8];
uint16_t rx700[8];
uint16_t rx710[8];

volatile uint16_t loopCnt = 0;

// ============================================================================
// 초음파 트리거 출력 (SysConfig: myGPIO_trig)
// ============================================================================
static inline void triggerUltrasonic(void)
{
    GPIO_writePin(myGPIO_trig, 0);
    DEVICE_DELAY_US(2);

    GPIO_writePin(myGPIO_trig, 1);
    DEVICE_DELAY_US(20);

    GPIO_writePin(myGPIO_trig, 0);

    g_measureDone = 0;
}

// ============================================================================
// EPWM: 작은별 멜로디 주파수 설정 (SysConfig: myEPWM0)
// ============================================================================
static inline void EPWM_setNote(uint16_t freq)
{
    if(freq == 0){
        EPWM_setTimeBasePeriod(myEPWM0_BASE, 0);
        EPWM_setCounterCompareValue(myEPWM0_BASE, EPWM_COUNTER_COMPARE_A, 0);
        return;
    }

    uint32_t tbclk = DEVICE_SYSCLK_FREQ / 8U;
    uint16_t prd = tbclk / (2U * freq);
    if(prd > 65535) prd = 65535;

    EPWM_setTimeBasePeriod(myEPWM0_BASE, prd);
    EPWM_setCounterCompareValue(myEPWM0_BASE, EPWM_COUNTER_COMPARE_A, (prd * 7U) / 10U);
}

// ============================================================================
// Timer1 ISR (1ms)  멜로디 재생
// ============================================================================
__interrupt void INT_myCPUTIMER1_ISR(void)
{
    if(g_noteRemain > 0)
        g_noteRemain--;
    else
    {
        g_noteIndex = (g_noteIndex + 1) % MELODY_LEN;
        g_noteRemain = melody[g_noteIndex].t;
        EPWM_setNote(melody[g_noteIndex].f);
    }

    CPUTimer_clearOverflowFlag(myCPUTIMER1_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

// ============================================================================
// ADC ISR (SysConfig: myADC0)
// ============================================================================
__interrupt void INT_myADC0_1_ISR(void)
{
    adc1_raw = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    adc2_raw = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

// ============================================================================
// CAN ISR  700, 710 수신 유지 (SysConfig: myCAN0)
// ============================================================================
__interrupt void INT_myCAN0_0_ISR(void)
{
    uint32_t id = CAN_getInterruptCause(myCAN0_BASE);

    if(id == 11)      CAN_readMessage(myCAN0_BASE, 11, rx700);
    else if(id == 12) CAN_readMessage(myCAN0_BASE, 12, rx710);

    CAN_clearInterruptStatus(myCAN0_BASE, id);
    CAN_clearGlobalInterruptStatus(myCAN0_BASE, CAN_GLOBAL_INT_CANINT0);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}

// ============================================================================
// eCAP ISR  초음파 Echo 측정
// ============================================================================
__interrupt void INT_myECAP0_ISR(void)
{
    g_echoRise = ECAP_getEventTimeStamp(myECAP0_BASE, ECAP_EVENT_1);
    g_echoFall = ECAP_getEventTimeStamp(myECAP0_BASE, ECAP_EVENT_2);

    if(g_echoFall >= g_echoRise)
        g_echoDelta = g_echoFall - g_echoRise;

    uint32_t mm = (g_echoDelta * DIST_NUM) / DIST_DEN;
    if(mm > 65535) mm = 65535;

    g_distance_mm = (uint16_t)mm;
    g_measureDone = 1;

    ECAP_clearInterrupt(myECAP0_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
    ECAP_clearGlobalInterrupt(myECAP0_BASE);
    Interrupt_clearACKGroup(INT_myECAP0_INTERRUPT_ACK_GROUP);
}

// ============================================================================
// 사용하지 않지만 SysConfig가 요구하는 CAN1 ISR
// ============================================================================
__interrupt void INT_myCAN0_1_ISR(void)
{
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}

// ============================================================================
// MAIN  과제 1~6 전체 동작
// ============================================================================
void main(void)
{
    Device_init();
    Interrupt_initModule();
    Interrupt_initVectorTable();
    Board_init();

    // -------- Interrupt 등록 --------
    Interrupt_register(INT_myCPUTIMER1, &INT_myCPUTIMER1_ISR);
    Interrupt_enable(INT_myCPUTIMER1);

    Interrupt_register(INT_myADC0_1, &INT_myADC0_1_ISR);
    Interrupt_enable(INT_myADC0_1);

    Interrupt_register(INT_myCAN0_0, &INT_myCAN0_0_ISR);
    Interrupt_enable(INT_myCAN0_0);

    Interrupt_register(INT_myECAP0, &INT_myECAP0_ISR);
    Interrupt_enable(INT_myECAP0);

    EINT;
    ERTM;

    // -------- 초기 멜로디 설정 --------
    EPWM_setNote(melody[0].f);
    g_noteRemain = melody[0].t;

    // -------- Timer 시작 --------
    CPUTimer_startTimer(myCPUTIMER0_BASE);  // 20ms
    CPUTimer_startTimer(myCPUTIMER1_BASE);  // 1ms

    // -------- LED 초기값 --------
    GPIO_writePin(myLED04, 1);
    GPIO_writePin(myLED05, 0);
    uint16_t ledTick = 0;

    // ========================================================================
    // 메인 Loop (20ms 루프)
    // ========================================================================
    while(1)
    {
        if(!CPUTimer_getTimerOverflowStatus(myCPUTIMER0_BASE))
            continue;
        CPUTimer_clearOverflowFlag(myCPUTIMER0_BASE);

        loopCnt++;

        // ----------------- LED 토글 (0.5초) -----------------
        if(++ledTick >= 25){
            ledTick = 0;
            GPIO_togglePin(myLED04);
            GPIO_togglePin(myLED05);
        }

        // ----------------- 초음파 TRIG 약 60ms -----------------
        if(++g_ultraTick >= 3){
            g_ultraTick = 0;
            triggerUltrasonic();
        }
//        triggerUltrasonic();
        // ----------------- 초음파 거리 출력 -----------------
        if(g_measureDone){
            char msg[40];
            sprintf(msg, "DIST: %u \r\n", g_distance_mm);
            SCI_writeCharArray(mySCI0_BASE, (uint16_t*)msg, 40);
            g_measureDone = 0;
        }

        // ----------------- ADC 변환 -----------------
        ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
        ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER1);

        float v1 = adc1_raw * 3.3f / 4095.f;
        float v2 = adc2_raw * 3.3f / 4095.f;

        uint16_t a1 = (uint16_t)(v1 * 10);
        uint16_t a2 = (uint16_t)(v2 * 100);

        // ----------------- 0x100 송신 -----------------
        txBuf[0] = loopCnt & 0xFF;
        txBuf[1] = loopCnt >> 8;
        txBuf[2] = a1 & 0xFF;
        txBuf[3] = a1 >> 8;
        txBuf[4] = a2 & 0xFF;
        txBuf[5] = a2 >> 8;
        txBuf[6] = 0;
        txBuf[7] = 0;
        CAN_sendMessage(myCAN0_BASE, 1, 8, txBuf);

        // ----------------- 0x110 : 초음파 거리(mm) 송신 -----------------
        uint16_t dist = g_distance_mm;

        // Byte0 = 하위비트(LSB)
        txBuf[0] = dist & 0xFF;
        // Byte1 = 상위비트(MSB)
        txBuf[1] = (dist >> 8) & 0xFF;

        // 나머지 2~7 바이트는 0
        txBuf[2] = 0;
        txBuf[3] = 0;
        txBuf[4] = 0;
        txBuf[5] = 0;
        txBuf[6] = 0;
        txBuf[7] = 0;

        CAN_sendMessage(myCAN0_BASE, 2, 8, txBuf);



        // ----------------- 0x120 계산 -----------------
        uint16_t X1 = rx700[0] | (rx700[1] << 8);
        uint16_t X2 = rx700[2] | (rx700[3] << 8);
        uint16_t X3 = rx700[4] | (rx700[5] << 8);
        uint16_t X4 = rx700[6] | (rx700[7] << 8);

        uint16_t Y1 = rx710[0] | (rx710[1] << 8);
        uint16_t Y2 = rx710[2] | (rx710[3] << 8);
        uint16_t Y3 = rx710[4] | (rx710[5] << 8);
        uint16_t Y4 = rx710[6] | (rx710[7] << 8);

        int16_t Z1 = (int16_t)X1 + (int16_t)Y1;
        int16_t Z2 = (int16_t)X2 - (int16_t)Y2;
        int16_t Z3 = (int16_t)X3 * (int16_t)Y3;

        uint16_t Z4 = (Y4 != 0) ? (uint16_t)((float)X4 / Y4 * 100.0f) : 0;

        txBuf[0] = Z1 & 0xFF;
        txBuf[1] = Z1 >> 8;
        txBuf[2] = Z2 & 0xFF;
        txBuf[3] = Z2 >> 8;
        txBuf[4] = Z3 & 0xFF;
        txBuf[5] = Z3 >> 8;
        txBuf[6] = Z4 & 0xFF;
        txBuf[7] = Z4 >> 8;
        CAN_sendMessage(myCAN0_BASE, 3, 8, txBuf);
    }
}
