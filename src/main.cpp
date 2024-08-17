#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include <Arduino.h>
#include <TFT_eSPI.h>

void* my_malloc(const char* what, std::size_t size) {
    (void) what;
    Serial.print("malloc ");
    Serial.print(what);
    Serial.print(":  ");
    Serial.println(size);
    return malloc(size);
}

void* my_realloc(const char* what, void* ptr, std::size_t new_size) {
    (void) what;
    Serial.print("realloc ");
    Serial.print(what);
    Serial.print(":  ");
    Serial.println(new_size);
    return realloc(ptr, new_size);
}

#define PL_MPEG_IMPLEMENTATION
#define PLM_BUFFER_DEFAULT_SIZE (32 * 1024)
#define PLM_MALLOC(what, sz) my_malloc(what, sz)
#define PLM_FREE(p) free(p)
#define PLM_REALLOC(what, p, sz) my_realloc(what, p, sz)
#include "pl_mpeg.h"

#include "buttonvideo.h"

constexpr int WIDTH = 128;
constexpr int HEIGHT = 128;

constexpr int MS_PER_FRAME = 33;

uint32_t pwm_slice_num;

static void pwm_setup() {
    gpio_set_function(TFT_BL, GPIO_FUNC_PWM);
    pwm_slice_num = pwm_gpio_to_slice_num(TFT_BL);
    pwm_set_wrap(pwm_slice_num, 100);
    pwm_set_chan_level(pwm_slice_num, PWM_CHAN_B, 1);
    pwm_set_clkdiv(pwm_slice_num, 50);
    pwm_set_enabled(pwm_slice_num, true);
}

static void set_bl_pwn(uint8_t level) {
    pwm_set_chan_level(pwm_slice_num, PWM_CHAN_B, level);
}

static bool buttonHeld() {
    return false;
}

TFT_eSPI tft = TFT_eSPI();

int16_t yuv_to_rgb(int y, int cr, int cb) {
    // luma (y) is scaled to 16-bit
	y = (max(0, y - 16)) * 76309;
    // cr and cb get converted to -128..127
	cr -= 128;
    cb -= 128;

    int r = (cr * 104597);
    int g = (cb * 25674 + cr * 53278);
    int b = (cb * 132201);
    r = plm_clamp((y + r) >> 16);
	g = plm_clamp((y - g) >> 16);
	b = plm_clamp((y + b) >> 16);
    return tft.color565(r, g, b);
}

void show_color_frame(plm_frame_t *frame) {
    tft.startWrite();
    int width = frame->width;
    int height = max(HEIGHT, frame->height);
    tft.setAddrWindow(0, (HEIGHT - height) / 2, width, height);
    int y_index = 0;
    int c_index = 0;
    for (int y = height; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            tft.pushColor(yuv_to_rgb(
                frame->y.data[y_index],
                frame->cr.data[c_index],
                frame->cb.data[c_index]));
            ++y_index;
            if ((x & 1) == 1) ++c_index;
        }
        if ((y & 1) == 0) c_index -= width / 2;
    }
    tft.endWrite();
}

void play_color_video(uint8_t const* data, size_t len, bool loop) {
    plm_t *plm =
        plm_create_with_memory(
                const_cast<uint8_t*>(data),
                len,
                false, false);

    plm_set_audio_enabled(plm, false);
    plm_set_loop(plm, loop);

    auto last_time = millis();

    // Decode forever until power is removed
    while (true) {
        // pause when user button held
        while (buttonHeld()) {}

        auto *frame = plm_decode_video(plm);
        if (frame) show_color_frame(frame);

        auto now = millis();
        if (now - last_time < MS_PER_FRAME) {
            delay(MS_PER_FRAME - (now - last_time));
        }
        last_time = now;

        // if not looping, NULL frame means end
        if (!loop && !frame) break;
    }
    plm_destroy(plm);
}

inline int16_t convertPixel(int16_t luma) {
    return tft.color565(luma, luma, luma);
}

void show_static_frame() {
    tft.startWrite();
    tft.setAddrWindow(0, 0, WIDTH, HEIGHT);
    int i = 0;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            uint32_t luma = random(256);
            tft.pushColor(convertPixel(luma));
        }
    }
    tft.endWrite();
}

void play_static(int msDuration) {
    auto now = millis();
    auto last_time = now;
    auto endTime = now + msDuration;
    while ((now = millis()) < endTime
        || buttonHeld()) {
        show_static_frame();
        if (now - last_time < MS_PER_FRAME) {
            delay(MS_PER_FRAME - (now - last_time));
        }
        last_time = now;
    }
    tft.fillScreen(TFT_BLACK);
}

bool demo_mode = false;

void setup() {
    // pwm_setup();
    // set_bl_pwn(20);

    Serial.begin(19200);
    Serial.println("starting on Pico");

    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
}

void loop() {
    play_color_video(ST_SNW_Intro_mpg, ST_SNW_Intro_mpg_len, false);
    play_static(200);
}

