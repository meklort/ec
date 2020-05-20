#include <board/acpi.h>
#include <board/battery.h>
#include <board/gpio.h>
#include <board/kbled.h>
#include <board/lid.h>
#include <board/peci.h>
#include <common/debug.h>

#ifndef HAVE_LED_AIRPLANE_N
#define HAVE_LED_AIRPLANE_N 1
#endif // HAVE_LED_AIRPLANE_N

typedef struct {
    uint8_t NMSG;
	uint8_t SLED:4;
	// Offset (0x02),
	uint8_t MODE:1;
	uint8_t FAN0:1;
	uint8_t TME0:1;
	uint8_t TME1:1;
	uint8_t FAN1:1;
    uint8_t rsvd_2:2;
	// Offset (0x03),
	uint8_t LSTE:1;
	uint8_t LSW0:1;
	uint8_t LWKE:1;
	uint8_t WAKF:1;
	uint8_t rsvd_3:2;
	uint8_t PWKE:1;
	uint8_t MWKE:1;

	uint8_t AC0;
	uint8_t PSV;
	uint8_t CRT;
	uint8_t TMP;
	uint8_t AC1;
	uint8_t BBST;
	// Offset (0x0B),
	// Offset (0x0C),
	// Offset (0x0D),
	// Offset (0x0E),
	uint8_t SLPT;
	uint8_t SWEJ:1;
	uint8_t SWCH:1;

	// Offset (0x10),
	uint8_t ADP:1; /* 0x10 <- Adapter Present */
	uint8_t AFLT:1;
	uint8_t BAT0:1; /* 0x10 <- BAT0 Present */
	uint8_t BAT1:1; /* 0x10 <- BAT1 Present */
    uint8_t resv_16:3;
	uint8_t PWOF:1;
	uint8_t WFNO;   // 0x11
	uint32_t BPU0;  // 0x12
	uint32_t BDC0;  // 0x16 <- Battery Desing Capacity
	uint32_t BFC0;  // 0x1a <- Battery Full Capacity
	uint32_t BTC0;  // 0x1e <- Battery Total Capacity
	uint32_t BDV0;  // 0x22 <- Battery Design Voltage
	uint32_t BST0;  // 0x26 <- Battery Status
	uint32_t BPR0;  // 0x2a <- Battery Present Current ??
	uint32_t BRC0;  // 0x2e <- Battery Remaining Capacity
	uint32_t BPV0;  // 0x32 <- Battery Present Voltage
	uint16_t BTP0;  // 0x36 <- Battery Temeprature
	uint16_t BRS0;  // 0x3a
	uint32_t BCW0;  // 0x3e
	uint32_t BCL0;  // 0x42
	uint32_t BCG0;  // 0x46 <- Battery Capacity Granularity
	uint32_t BG20;  // 0x4a <- Battery Capacity Granularity 2
	uint64_t BMO0;  // 0x4e <- Battery Model[8]
	uint64_t BIF0;  // 0x56 <-
	uint32_t BSN0;  // 0x5e <- Battery Serial Number
	uint64_t BTY0;  // 0x62 <- Battery Type[8] (LION)
	// Offset (0x67),
	// Offset (0x68),
	// uint8_t ECOS,
	// uint8_t LNXD,   8,
	// uint8_t ECPS,   8,
	// Offset (0x6C),
	// uint16_t BTMP,
	// uint8_t EVTN
	// Offset (0x72),
	// uint8_t PRCL,
	// uint8_t PRC0,
	// uint8_t PRC1,
	// uint8_t PRCM,
	// uint8_t PRIN,
	// uint8_t PSTE,
	// uint8_t PCAD,
	// uint8_t PEWL,
	// uint8_t PWRL,
	// uint8_t PECD,
	// uint8_t PEHI,
	// uint8_t PECI,
	// uint8_t PEPL,
	// uint8_t PEPM,
	// uint8_t PWFC,
	// uint8_t PECC,
	// uint8_t PDT0,
	// uint8_t PDT1,
	// uint8_t PDT2,
	// uint8_t PDT3,
	// uint8_t PRFC,
	// uint8_t PRS0,
	// uint8_t PRS1,
	// uint8_t PRS2,
	// uint8_t PRS3,
	// uint8_t PRS4,
	// uint8_t PRCS,
	// uint8_t PEC0,
	// uint8_t PEC1,
	// uint8_t PEC2,
	// uint8_t PEC3,
	// uint8_t CMDR,
	// uint8_t CVRT,
	// uint8_t GTVR,
	// uint8_t FANT,
	// uint8_t SKNT,
	// uint8_t AMBT,
	// uint8_t MCRT,
	// uint8_t DIM0,
	// uint8_t DIM1,
	// uint8_t PMAX,
	// uint8_t PPDT,
	// uint8_t PECH,
	// uint8_t PMDT,
	// uint8_t TSD0,
	// uint8_t TSD1,
	// uint8_t TSD2,
	// uint8_t TSD3,
	// uint16_t CPUP,
	// uint16_t MCHP,
	// uint16_t SYSP,
	// uint16_t CPAP,
	// uint16_t MCAP,
	// uint16_t SYAP,
	// uint16_t CFSP,
	// uint16_t CPUE,
	// Offset (0xC6),
	// Offset (0xC7),
	// VGAT,   8,
	// OEM1,   8,
	// OEM2,   8,
	// OEM3,   16,
	// OEM4,   8,
	// Offset (0xCE),
	// DUT1,   8,
	// DUT2,   8,
	// RPM1,   16,
	// RPM2,   16,
	// RPM4,   16,
	// Offset (0xD7),
	// DTHL,   8,
	// DTBP,   8,
	// AIRP,   8,
	// WINF,   8,
	// RINF,   8,
	// Offset (0xDD),
	// INF2,   8,
	// MUTE,   1,
	// Offset (0xE0),
	// RPM3,   16,
	// ECKS,   8,
	// Offset (0xE4),
	// 	,   4,
	// XTUF,   1,
	// EP12,   1,
	// Offset (0xE5),
	// INF3,   8,
	// Offset (0xE7),
	// GFOF,   8,
	// Offset (0xE9),
	// KPCR,   1,
	// Offset (0xEA),
	// Offset (0xF0),
	// PL1T,   16,
	// PL2T,   16,
	// TAUT,   8,
	// Offset (0xF8),
	// FCMD,   8,
	// FDAT,   8,
	// FBUF,   8,
	// FBF1,   8,
	// FBF2,   8,
	// FBF3,   8
} acpi_ram_t;

extern uint8_t sci_extra;

uint8_t ecos = 0;

static uint8_t fcmd = 0;
static uint8_t fdat = 0;
static uint8_t fbuf[4] = { 0, 0, 0, 0 };

void fcommand(void) {
    switch (fcmd) {
        // Keyboard backlight
        case 0xCA:
            switch (fdat) {
                // Set white LED brightness
                case 0x00:
                    kbled_set(fbuf[0]);
                    break;
                // Get white LED brightness
                case 0x01:
                    fbuf[0] = kbled_get();
                    break;
                // Set LED color
                case 0x03:
                    kbled_set_color(
                        ((uint32_t)fbuf[0]) |
                        ((uint32_t)fbuf[1] << 16) |
                        ((uint32_t)fbuf[2] << 8)
                    );
                    break;
                // Set LED brightness
                case 0x06:
                    kbled_set(fbuf[0]);
                    break;
            }
            break;
    }
}

uint8_t acpi_read(uint8_t addr) {
    uint8_t data = 0;

    #define ACPI_8(K, V) \
    case (K): \
        data = (uint8_t)(V); \
        break

    #define ACPI_16(K, V) \
        ACPI_8(K, V); \
        ACPI_8((K) + 1, (V) >> 8)

    #define ACPI_32(K, V) \
        ACPI_16(K, V); \
        ACPI_16((K) + 2, (V) >> 16)

    switch (addr) {
        // Lid state and other flags
        case 0x03:
            if (gpio_get(&LID_SW_N)) {
                // Lid is open
                data |= 1 << 0;
            }
            if (lid_wake) {
                data |= 1 << 2;
            }
            break;

        // Handle AC adapter and battery present
        case 0x10:
            if (!gpio_get(&ACIN_N)) {
                // AC adapter connected
                data |= 1 << 0;
            }
            data |= battery_present ? (1 << 2) : 0;
            break;

        ACPI_16(0x16, battery_design_capacity); /* BDC0: 0x16 - 0x17 */

        /* Unused: 0x18 - 0x19 */

        ACPI_16(0x1A, battery_full_capacity);   /* BFC0: 0x1A - 0x1B */

        /* Unused: 0x1C - 0x1D */

        ACPI_16(0x22, battery_design_voltage);  /* BDV0: 0x22 - 0x23 */

        /* Unused: 0x24 - 0x25 */

        case 0x26:
            // If AC adapter connected
            if (!gpio_get(&ACIN_N)) {
                // And battery is not fully charged
                if (battery_current != 0) {
                    // Battery is charging
                    data |= 1 << 1;
                }
            }
            break;

        /* Unused: 0x27 - 0x29 */

        ACPI_16(0x2A, battery_current);             /* BPR0? */

        /* Unused: 0x2C - 0x2D */

        ACPI_16(0x2E, battery_remaining_capacity);  /* BRC0: */

        /* Unused: 0x30 - 0x31 */

        ACPI_16(0x32, battery_voltage);             /* BPV0 */

        /* Unused: 0x34 - 0x35 */

        ACPI_16(0x36, battery_temp);                /* BTP0 */

        ACPI_8(0x4E, battery_device.str[0]);        /* BMO0 */
        ACPI_8(0x4F, battery_device.str[1]);
        ACPI_8(0x50, battery_device.str[2]);
        ACPI_8(0x51, battery_device.str[3]);
        ACPI_8(0x52, battery_device.str[4]);
        ACPI_8(0x53, battery_device.str[5]);
        ACPI_8(0x54, battery_device.str[6]);
        ACPI_8(0x55, battery_device.str[7]);

        ACPI_16(0x5E, battery_serial);              /* BSN0 */

        ACPI_8(0x62, battery_type.str[0]);          /* BTY0 */
        ACPI_8(0x63, battery_type.str[1]);
        ACPI_8(0x64, battery_type.str[2]);
        ACPI_8(0x65, battery_type.str[3]);

        /* Unused: 0x66 - 0x67 */

        ACPI_8(0x68, ecos);

        ACPI_8(0xCC, sci_extra);

#if HAVE_LED_AIRPLANE_N
        // Airplane mode LED
        case 0xD9:
            if (!gpio_get(&LED_AIRPLANE_N)) {
                data |= (1 << 6);
            }
            break;
#endif // HAVE_LED_AIRPLANE_N

        // Set size of flash (from old firmware)
        ACPI_8 (0xE5, 0x80);

        ACPI_8 (0xF8, fcmd);
        ACPI_8 (0xF9, fdat);
        ACPI_8 (0xFA, fbuf[0]);
        ACPI_8 (0xFB, fbuf[1]);
        ACPI_8 (0xFC, fbuf[2]);
        ACPI_8 (0xFD, fbuf[3]);
    }

    DEBUG("acpi_read %02X = %02X\n", addr, data);
    return data;
}


void acpi_write(uint8_t addr, uint8_t data) {
    DEBUG("acpi_write %02X = %02X\n", addr, data);

    switch (addr) {
        // Lid state and other flags
        case 0x03:
            lid_wake = (bool)(data & (1 << 2));
            break;

        case 0x68:
            ecos = data;
            break;

#if HAVE_LED_AIRPLANE_N
        // Airplane mode LED
        case 0xD9:
            gpio_set(&LED_AIRPLANE_N, !(bool)(data & (1 << 6)));
            break;
#endif

        case 0xF8:
            fcmd = data;
            fcommand();
            break;
        case 0xF9:
            fdat = data;
            break;
        case 0xFA:
            fbuf[0] = data;
            break;
        case 0xFB:
            fbuf[1] = data;
            break;
        case 0xFC:
            fbuf[2] = data;
            break;
        case 0xFD:
            fbuf[3] = data;
            break;
    }
}
