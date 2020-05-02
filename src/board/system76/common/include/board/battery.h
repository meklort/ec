#ifndef _BOARD_BATTERY_H
#define _BOARD_BATTERY_H

#include <stdint.h>
#include <stdbool.h>

/* Types */
#define SBS_BATTERY_STRING_BLOCK_SIZE   (32u) /* 1 length byte and 31 char bytes */
#define SBS_BATTERY_STRING_MAX_LEN      (31u)
typedef struct {
    uint8_t len;
    char str[SBS_BATTERY_STRING_MAX_LEN + 1]; /* Max 31 bytes + terminator */
} sbs_string_t;

/* Smart Battery Functions */
#define SBS_BATTERY_MODE                (0x03u)
#define SBS_BATTERY_MODE_CAPACITY_MODE_MASK (1 << 15)

#define SBS_BATTERY_TEMPERATURE         (0x08u) /* Returns the cell-pack's internal temperature (°K) in 0.1°K units. */
#define SBS_BATTERY_VOLTAGE             (0x09u) /* Returns the cell-pack voltage (mV). */
#define SBS_BATTERY_CURRENT             (0x0Au) /* Returns the current being supplied (or accepted) through the battery's terminals (mA). */
#define SBS_BATTERY_AVERAGE_CURRENT     (0x0Bu) /* Returns a one-minute rolling average based on the current being supplied (or accepted) through the battery's terminals (mA). */
#define SBS_BATTERY_MAX_ERROR           (0x0Cu) /* Returns the expected margin of error (%) in the state of charge calculation. */
#define SBS_BATTERY_RELATIVE_CHARGE     (0x0Du) /* Returns the predicted remaining battery capacity expressed as a percentage of FullChargeCapacity() (%).*/
#define SBS_BATTERY_ABSOLUTE_CHARGE     (0x0Eu) /* Returns the predicted remaining battery capacity expressed as a percentage of DesignCapacity() (%). */
#define SBS_BATTERY_REMAINING_CAPACITY  (0x0Fu) /* Returns the predicted remaining battery capacity. */
#define SBS_BATTERY_FULL_CAPACITY       (0x10u) /* Returns the predicted pack capacity when it is fully charged. */
#define SBS_BATTERY_STATUS              (0x16u) /* Returns the Smart Battery's status word which contains Alarm and Status bit flags. */

#define SBS_BATTERY_CYCLE_COUNT         (0x17u) /* Returns the number of cycles the battery has experienced. */
#define SBS_BATTERY_DESIGN_CAPACITY     (0x18u) /* Returns the theoretical capacity of a new pack. */
#define SBS_BATTERY_DESIGN_VOLTAGE      (0x19u) /* Returns the theoretical voltage of a new pack (mV). */
#define SBS_BATTERY_MANUFACTURE_DATE    (0x1Bu) /* Returns the date the cell pack was manufactured in a packed integer. The date is packed in the following fashion: (year-1980) * 512 + month * 32 + day. */
#define SBS_BATTERY_SERIAL_NUMBER       (0x1Cu) /* Returns a serial number. */
#define SBS_BATTERY_MANUFACTURER_NAME   (0x20u) /* Returns a string containing the battery's manufacturer's name. */
#define SBS_BATTERY_DEVICE_NAME         (0x21u) /* Returns a string containing the battery's name. */
#define SBS_BATTERY_DEVICE_CHEMISTRY    (0x22u) /* Returns a string containing the battery's chemistry. */

/* Smart Charger Functions */
#define SBS_CHARGER_MODE                (0x12u) /* Allows the System Host to configure the charger and change the default modes. */
#define SBS_CHARGER_MODE_INHIBIT_CHARGE (1 << 0) /* When this bit is 0, battery charging is enabled with valid value in REG 0x14() and REG 0x15() */
#define SBS_CHARGER_MODE_ENABLE_POLLING (1 << 1)
#define SBS_CHARGER_MODE_POR_RESET      (1 << 2)
#define SBS_CHARGER_MODE_RESET_TO_ZERO  (1 << 3)

#define SBS_CHARGER_STATUS              (0x13u) /* */
#define SBS_CHARGER_STATUS_CHARGE_INHIBITED (1 << 0)
#define SBS_CHARGER_STATUS_POLLING_ENABLED  (1 << 1)
#define SBS_CHARGER_STATUS_VOLTAGE_NOTREG   (1 << 2)
#define SBS_CHARGER_STATUS_AC_PRESENT       (1 << 15)

#define SBS_CHARGER_CHARGING_CURRENT    (0x14u) /* The desired (maximum) charging rate */
#define SBS_CHARGER_CHARGING_VOLTAGE    (0x15u) /* The desired charging voltage */
#define SBS_CHARGER_INPUT_CURRENT       (0x3Fu) /* Mfg1: The maximium input current to pull from the power adapter. */


typedef struct {
    uint16_t design_capacity;
} battery_information_t;

typedef struct
{
    uint16_t cycle_count;
} battery_status_t;

extern uint16_t battery_temp;
extern uint16_t battery_voltage;
extern uint16_t battery_current;
extern uint16_t battery_charge;
extern uint16_t battery_remaining_capacity;
extern uint16_t battery_full_capacity;
extern uint16_t battery_status;
extern uint16_t battery_design_capacity;
extern uint16_t battery_design_voltage;
extern uint16_t battery_cycle_count;
extern uint16_t battery_serial;

extern sbs_string_t battery_manufacturer;
extern uint16_t battery_serial;
extern sbs_string_t battery_device;
extern sbs_string_t battery_type;
extern uint16_t battery_cycle_count;
extern bool battery_present;


int battery_charger_disable(void);
int battery_charger_enable(void);
void battery_event(void);
void battery_debug(void);

void battery_init_information(void);

#endif // _BOARD_BATTERY_H
