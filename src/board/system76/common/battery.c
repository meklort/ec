#include <board/battery.h>
#include <board/gpio.h>
#include <board/smbus.h>
#include <common/config.h>
#include <common/debug.h>

#define BATTERY_ADDRESS 0x0B
#define CHARGER_ADDRESS 0x09

#ifndef CHARGER_INPUT_MULT
#define CHARGER_INPUT_MULT 1 /* 1:1 mapping from input current register to reality */
#endif

// ChargeOption0 flags
// Low Power Mode Enable
#define SBC_EN_LWPWR        ((uint16_t)(1 << 15))
// Watchdog Timer Adjust
#define SBC_WDTMR_ADJ_175S  ((uint16_t)(0b11 << 13))
// Switching Frequency
#define SBC_PWM_FREQ_800KHZ ((uint16_t)(0b01 << 8))
// IDCHG Amplifier Gain
#define SBC_IDCHC_GAIN      ((uint16_t)(1 << 3))

config_t battery_cfg_charger_start = {
    .config_id = {'B', 'S', 'T', 'A'},
    .config_short = "Battery Charge Start Threshold",
    .config_desc = "Charging is enabled when the battery drops below this level",
    .value = {
        .min_value = 60,
        .max_value = 100,
        .value = 100 /* Default value */
    },
    .set_callback = NULL,
};

config_t battery_cfg_charger_stop = {
    .config_id = {'B', 'S', 'T', 'O'},
    .config_short = "Battery Charge Stop Threshold",
    .config_desc = "Charging is disabled when the batter is greater or equal to this value",
    .value = {
        .min_value = 80,
        .max_value = 100,
        .value = 100 /* Default value */
    },
    .set_callback = NULL,
};


bool battery_charger_enabled = false;
uint16_t battery_temp = 0;
uint16_t battery_voltage = 0;
uint16_t battery_current = 0;
uint16_t battery_charge = 0;
uint16_t battery_remaining_capacity = 0;
uint16_t battery_full_capacity = 0;
uint16_t battery_status = 0;
uint16_t battery_design_capacity = 0;
uint16_t battery_design_voltage = 0;

sbs_string_t battery_manufacturer = { 0 };
uint16_t battery_serial = 0;
sbs_string_t battery_device = { 0 };
sbs_string_t battery_type = { 0 };
uint16_t battery_cycle_count = 0;
bool battery_present = false;

int battery_charger_disable(void) {
    int res = 0;

    // Set charge option 0 with 175s watchdog
    res = smbus_write(
        CHARGER_ADDRESS,
        SBS_CHARGER_MODE,
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

    DEBUG("Charger disabled.\n");
    battery_charger_enabled = false;
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
    res = smbus_write(CHARGER_ADDRESS, SBS_CHARGER_INPUT_CURRENT, CHARGER_INPUT_CURRENT / CHARGER_INPUT_MULT);
    if (res < 0) return res;

    // Set charge option 0 with watchdog disabled
    res = smbus_write(
        CHARGER_ADDRESS,
        SBS_CHARGER_MODE,
        SBC_EN_LWPWR |
        SBC_PWM_FREQ_800KHZ |
        SBC_IDCHC_GAIN
    );

    DEBUG("Charger enabled.\n");
    battery_charger_enabled = true;

    return 0;
}

void battery_read_string(sbs_string_t *str, uint8_t address)
{
    int res = 0;
    res = smbus_read_bytes(BATTERY_ADDRESS, address, (char *)str, SBS_BATTERY_STRING_BLOCK_SIZE);
    if (res < 0) {
        str->len = 0;
        str->str[0] = 0;
    }
    else if(str->len < sizeof(str->str))
    {
        str->str[str->len] = 0;
    }
    else
    {
        // Invalid length.
        str->str[0] = 0;
    }
}

void battery_init(void)
{
    config_register(&battery_cfg_charger_start);
    config_register(&battery_cfg_charger_stop);

    battery_init_information();
}

void battery_init_information(void)
{
    int res = 0;

    res = smbus_read(BATTERY_ADDRESS, SBS_BATTERY_SERIAL_NUMBER, &battery_serial);
    if (0 <= res) {
        battery_present = true;
        battery_read_string(&battery_manufacturer, SBS_BATTERY_MANUFACTURER_NAME);
        battery_read_string(&battery_device, SBS_BATTERY_DEVICE_NAME);
        battery_read_string(&battery_type, SBS_BATTERY_DEVICE_CHEMISTRY);

        #define command(N, V) { \
            res = smbus_read(BATTERY_ADDRESS, V, &N); \
            if (res < 0) { \
                N = 0; \
                battery_present = false; \
            } \
        }

        command(battery_design_capacity, SBS_BATTERY_DESIGN_CAPACITY);
        command(battery_design_voltage, SBS_BATTERY_DESIGN_VOLTAGE);
        command(battery_full_capacity, SBS_BATTERY_FULL_CAPACITY);

        #undef command
    }
}

void battery_event(void) {
    int res = 0;

    #define command(N, V) { \
        res = smbus_read(BATTERY_ADDRESS, V, &N); \
        if (res < 0) { \
            N = 0; \
        } \
    }

    // Battery Status
    command(battery_voltage, SBS_BATTERY_VOLTAGE);
    command(battery_current, SBS_BATTERY_CURRENT);
    command(battery_charge, SBS_BATTERY_RELATIVE_CHARGE);
    command(battery_status, SBS_BATTERY_STATUS);

    // Externded Status
    command(battery_temp, SBS_BATTERY_TEMPERATURE);
    command(battery_remaining_capacity, SBS_BATTERY_REMAINING_CAPACITY);
    command(battery_cycle_count, SBS_BATTERY_CYCLE_COUNT);

    // If we are below the start level, ensure the charger is enabled.
    bool on_bat = gpio_get(&ACIN_N);
    if(!on_bat) {
        if (battery_charge < battery_cfg_charger_start.value.value) {
            if (!battery_charger_enabled) {
                battery_charger_enable();
            }
        } else if (battery_charge >= battery_cfg_charger_stop.value.value) {
            if (battery_charger_enabled) {
                battery_charger_disable();
            }
        }
    }

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
    command(Temperature, BATTERY_ADDRESS, 0x08);
    command(Voltage, BATTERY_ADDRESS, 0x09);
    command(Current, BATTERY_ADDRESS, 0x0A);
    command(Charge, BATTERY_ADDRESS, 0x0D);
    command(Status, BATTERY_ADDRESS, 0x16);

    DEBUG("Charger:\n");
    command(ChargeOption0, CHARGER_ADDRESS, 0x12);
    command(ChargeOption1, CHARGER_ADDRESS, 0x3B);
    command(ChargeOption2, CHARGER_ADDRESS, 0x38);
    command(ChargeOption3, CHARGER_ADDRESS, 0x37);
    command(ChargeCurrent, CHARGER_ADDRESS, 0x14);
    command(ChargeVoltage, CHARGER_ADDRESS, 0x15);
    command(DishargeCurrent, CHARGER_ADDRESS, 0x39);
    command(InputCurrent, CHARGER_ADDRESS, SBS_CHARGER_INPUT_CURRENT);
    command(ProchotOption0, CHARGER_ADDRESS, 0x3C);
    command(ProchotOption1, CHARGER_ADDRESS, 0x3D);
    command(ProchotStatus, CHARGER_ADDRESS, 0x3A);

    #undef command

    DEBUG("Manufacturer: %s\n", battery_manufacturer.str);
    DEBUG("Device: %s\n", battery_device.str);
    DEBUG("Type: %s\n", battery_type.str);
    DEBUG("Serial: %04X\n", battery_serial);
}
