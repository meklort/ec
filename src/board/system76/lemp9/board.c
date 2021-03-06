#include <board/board.h>
#include <board/gpio.h>
#include <board/power.h>

extern uint8_t main_cycle;

void board_init(void) {
    // Allow CPU to boot
    gpio_set(&SB_KBCRST_N, true);
    // Allow backlight to be turned on
    gpio_set(&BKL_EN, true);
    // Enable camera
    gpio_set(&CCD_EN, true);
    // Enable wireless
    gpio_set(&BT_EN, true);
    gpio_set(&WLAN_EN, true);
    gpio_set(&WLAN_PWR_EN, true);
    // Enable right USB port
    gpio_set(&USB_PWR_EN_N, false);
    // Assert SMI#, SCI#, and SWI#
    gpio_set(&SCI_N, true);
    gpio_set(&SMI_N, true);
    gpio_set(&SWI_N, true);
}

void board_on_ac(bool ac) { /* Fix unused variable */ ac = ac; }

void board_event(void) {
    if (main_cycle == 0) {
        if (power_state == POWER_STATE_S0 || power_state == POWER_STATE_S3 || power_state == POWER_STATE_DS3) {
            // System is on
        } else if (gpio_get(&ACIN_N)) {
            // Power off VDD3 if system should be off
            gpio_set(&XLP_OUT, 0);
        }
    }
}
