#include <board/kbled.h>
#include <common/config.h>
#include <common/macro.h>
#include <ec/dac.h>

#define KBLED_DAC 2
#define KBLED_DACDAT DACDAT2

static uint8_t __code levels[] = {
    0x00,
    0x80,
    0x90,
    0xA8,
    0xC0,
    0xFF
};

void kbled_set_cfg(config_t* cfg);

config_t kbled_cfg_start_value = {
    .config_id = {'K', 'B', 'B', 'L'},
    .config_short = "Keyboard Backlight Level",
    .config_desc = "Power-on keyboard backlight level",
    .value = {
        .min_value = 0,
        .max_value = ARRAY_SIZE(levels) - 1,
        .value = 0 /* Default value */
    },
    .set_callback = &kbled_set_cfg,
};

void kbled_init(void) {
    config_register(&kbled_cfg_start_value);

    // Enable DAC used for KBLIGHT_ADJ
    DACPDREG &= ~(1 << KBLED_DAC);
    kbled_reset();
}

void kbled_reset(void) {
    kbled_set(kbled_cfg_start_value.value.value);
}

void kbled_set_cfg(config_t* cfg) {
    kbled_set(cfg->value.value);
}

uint8_t kbled_get(void) {
    uint8_t level;
    uint8_t raw = KBLED_DACDAT;
    for (level = 0; level < ARRAY_SIZE(levels); level++) {
        if (raw <= levels[level]) {
            return level;
        }
    }
    return 0;
}

void kbled_set(uint8_t level) {
    uint8_t raw = 0;
    if (level < ARRAY_SIZE(levels)) {
        raw = levels[level];
    }
    KBLED_DACDAT = raw;
}

void kbled_set_color(uint32_t color) { /*Fix unused variable*/ color = color; }
