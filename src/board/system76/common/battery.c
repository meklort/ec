#include <board/smbus.h>
#include <board/battery.h>
#include <common/debug.h>

#define BATTERY_ADDRESS 0x0B
#define CHARGER_ADDRESS 0x09

// ChargeOption0 flags
// Low Power Mode Enable
#define SBC_EN_LWPWR        ((uint16_t)(1 << 15))
// Watchdog Timer Adjust
#define SBC_WDTMR_ADJ_175S  ((uint16_t)(0b11 << 13))
// Switching Frequency
#define SBC_PWM_FREQ_800KHZ ((uint16_t)(0b01 << 8))
// IDCHG Amplifier Gain
#define SBC_IDCHC_GAIN      ((uint16_t)(1 << 3))

int battery_charger_disable(void) {
    int res = 0;

    // Set charge option 0 with 175s watchdog
    res = smbus_write(
        CHARGER_ADDRESS,
        0x12,
        SBC_EN_LWPWR |
        SBC_WDTMR_ADJ_175S |
        SBC_PWM_FREQ_800KHZ |
        SBC_IDCHC_GAIN
    );

    // Disable charge current
    res = smbus_write(CHARGER_ADDRESS, SBS_CHARGER_CHARGING_CURRENT, 0);
    if (res < 0) return res;

    // Disable charge voltage
    res = smbus_write(CHARGER_ADDRESS, SBS_CHARGER_CHARGING_VOLTAGE, 0);
    if (res < 0) return res;

    // Disable input current
    res = smbus_write(CHARGER_ADDRESS, SBS_CHARGER_INPUT_CURRENT, 0);
    if (res < 0) return res;

    return 0;
}

int battery_charger_enable(void) {
    int res = 0;

    res = battery_charger_disable();
    if (res < 0) return res;

    // Set charge current in mA
    res = smbus_write(CHARGER_ADDRESS, SBS_CHARGER_CHARGING_CURRENT, CHARGER_CHARGE_CURRENT);
    if (res < 0) return res;

    // Set charge voltage in mV
    res = smbus_write(CHARGER_ADDRESS, SBS_CHARGER_CHARGING_VOLTAGE, CHARGER_CHARGE_VOLTAGE);
    if (res < 0) return res;

    // Set input current in mA
    res = smbus_write(CHARGER_ADDRESS, SBS_CHARGER_INPUT_CURRENT, CHARGER_INPUT_CURRENT);
    if (res < 0) return res;

    // Set charge option 0 with watchdog disabled
    res = smbus_write(
        CHARGER_ADDRESS,
        0x12,
        SBC_EN_LWPWR |
        SBC_PWM_FREQ_800KHZ |
        SBC_IDCHC_GAIN
    );

    return 0;
}

uint16_t battery_temp = 0;
uint16_t battery_voltage = 0;
uint16_t battery_current = 0;
uint16_t battery_charge = 0;
uint16_t battery_remaining_capacity = 0;
uint16_t battery_full_capacity = 0;
uint16_t battery_status = 0;
uint16_t battery_design_capacity = 0;
uint16_t battery_design_voltage = 0;

void battery_event(void) {
    int res = 0;

    #define command(N, V) { \
        res = smbus_read(BATTERY_ADDRESS, V, &N); \
        if (res < 0) { \
            N = 0; \
        } \
    }

    command(battery_temp, SBS_BATTERY_TEMPERATURE);
    command(battery_voltage, SBS_BATTERY_VOLTAGE);
    command(battery_current, SBS_BATTERY_CURRENT);
    command(battery_charge, SBS_BATTERY_RELATIVE_CHARGE);
    command(battery_remaining_capacity, SBS_BATTERY_REMAINING_CAPACITY);
    command(battery_full_capacity, SBS_BATTERY_FULL_CAPACITY);
    command(battery_status, SBS_BATTERY_STATUS);
    command(battery_design_capacity, SBS_BATTERY_DESIGN_CAPACITY);
    command(battery_design_voltage, SBS_BATTERY_DESIGN_VOLTAGE);

    #undef command
}

void battery_debug(void) {
    uint16_t data = 0;
    int res = 0;

    #define command(N, A, V) { \
        DEBUG(#N ": "); \
        res = smbus_read(A, V, &data); \
        if (res < 0) { \
            DEBUG("ERROR %04X\n", -res); \
        } else { \
            DEBUG("%04X\n", data); \
        } \
    }

    DEBUG("Battery:\n");
    command(Temperature, BATTERY_ADDRESS, SBS_BATTERY_TEMPERATURE);
    command(Voltage, BATTERY_ADDRESS, SBS_BATTERY_VOLTAGE);
    command(Current, BATTERY_ADDRESS, SBS_BATTERY_CURRENT);
    command(Charge, BATTERY_ADDRESS, SBS_BATTERY_RELATIVE_CHARGE);
    command(Status, BATTERY_ADDRESS, SBS_BATTERY_STATUS);
    command(Cycle, BATTERY_ADDRESS, SBS_BATTERY_CYCLE_COUNT);

    DEBUG("Charger:\n");
    command(ChargeOption0, CHARGER_ADDRESS, 0x12);
    command(ChargeOption1, CHARGER_ADDRESS, 0x3B);
    command(ChargeOption2, CHARGER_ADDRESS, 0x38);
    command(ChargeOption3, CHARGER_ADDRESS, 0x37);
    command(ChargeCurrent, CHARGER_ADDRESS, SBS_CHARGER_CHARGING_CURRENT);
    command(ChargeVoltage, CHARGER_ADDRESS, SBS_CHARGER_CHARGING_VOLTAGE);
    command(DishargeCurrent, CHARGER_ADDRESS, 0x39);
    command(InputCurrent, CHARGER_ADDRESS, SBS_CHARGER_INPUT_CURRENT);
    command(ProchotOption0, CHARGER_ADDRESS, 0x3C);
    command(ProchotOption1, CHARGER_ADDRESS, 0x3D);
    command(ProchotStatus, CHARGER_ADDRESS, 0x3A);

    #undef command
}
