#include <stdio.h>
#include "stdbool.h"
#include "Timer.h"


#define TIMER_DIVIDER   (80)

bool flag = 0;


static void IRAM_ATTR timer_group0_isr (void *param){
    flag = 1;
}


void TimerInit(){
    timer_config_t config = {
        .divider = TIMER_DIVIDER, // 1MHZ timer
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_EN
    };
    timer_init(TIMER_GROUP_0, TIMER_1, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);
    timer_isr_callback_add(TIMER_GROUP_0,TIMER_1,&timer_group0_isr,NULL,0);
    timer_set_alarm_value(TIMER_GROUP_0,TIMER_1,3e3);
    timer_enable_intr(TIMER_GROUP_0, TIMER_1);
    timer_start(TIMER_GROUP_0, TIMER_1);
}
