#include "ActionSequence.hpp"
#include "LedStrip.hpp"

#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>

#include <random>

using namespace std::chrono_literals;
using namespace esp32pp;

struct RGB {
    uint8_t g {0};
    uint8_t r {0};
    uint8_t b {0};
};

LedStrip ledStrip;

constexpr auto PIXELS_SIZE = 12;
std::array<RGB, PIXELS_SIZE> pixels;

void fadeOutPixel(RGB& pixel)
{
    pixel.r = pixel.r > 1 ? pixel.r - 1 : 0;
    pixel.g = pixel.g > 1 ? pixel.g - 1 : 0;
    pixel.b = pixel.b > 1 ? pixel.b - 1 : 0;
}

int getRandomNumber(int N)
{
    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, N);
    return distribution(generator);
}

const std::array<RGB, 32> basicColors = {
    RGB {32, 0, 0},  // Red
    RGB {0, 32, 0},  // Green
    RGB {0, 0, 32},  // Blue
    RGB {32, 32, 0}, // Yellow
    RGB {0, 32, 32}, // Cyan
    RGB {32, 0, 32}, // Magenta
    RGB {32, 32, 32},// White
    RGB {16, 16, 0}, // Olive
    RGB {16, 0, 16}, // Purple
    RGB {0, 16, 16}, // Teal
    RGB {32, 16, 0}, // Orange
    RGB {16, 32, 0}, // Lime
    RGB {0, 32, 16}, // Spring Green
    RGB {0, 16, 32}, // Sky Blue
    RGB {16, 0, 32}, // Violet
    RGB {32, 0, 16}, // Rose
    RGB {16, 16, 16},// Grey
    RGB {24, 0, 0},  // Dark Red
    RGB {0, 24, 0},  // Dark Green
    RGB {0, 0, 24},  // Dark Blue
    RGB {24, 24, 0}, // Dark Yellow
    RGB {0, 24, 24}, // Dark Cyan
    RGB {24, 0, 24}, // Dark Magenta
    RGB {24, 24, 24},// Light Grey
    RGB {32, 8, 0},  // Amber
    RGB {8, 32, 0},  // Chartreuse
    RGB {0, 32, 8},  // Mint
    RGB {0, 8, 32},  // Azure
    RGB {8, 0, 32},  // Indigo
    RGB {32, 0, 8},  // Crimson
    RGB {16, 8, 4}   // Brown
};

extern "C" void app_main()
{
    auto err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    asio::io_context ioc;
    ledStrip.setup(5, WS2812Config {});

    ActionSequence actions(ioc);
    ActionSequence fade(ioc);

    int pixelIndex1 = 0;
    int pixelIndex2 = PIXELS_SIZE / 2;
    int colorIndex1 = 0;
    int colorIndex2 = 0;

    actions.addAction(
        [&]
        {
            if (++pixelIndex1 >= PIXELS_SIZE) {
                pixelIndex1 = 0;
                colorIndex1 = getRandomNumber(basicColors.size() - 1);
            }
            if (++pixelIndex2 >= PIXELS_SIZE) {
                pixelIndex2 = 0;
                colorIndex2 = getRandomNumber(basicColors.size() - 1);
            }

            pixels[pixelIndex1] = basicColors[colorIndex1];
            pixels[pixelIndex2] = basicColors[colorIndex2];
            ledStrip.transmit(pixels.data(), pixels.size() * 3);
        },
        150ms
    );
    actions.setInfiniteRepeat();
    actions.start();

    fade.addAction(
        [&]
        {
            for (auto& pixel : pixels) {
                fadeOutPixel(pixel);
            }
            ledStrip.transmit(pixels.data(), pixels.size() * 3);
        },
        20ms
    );
    fade.setInfiniteRepeat();
    fade.start();

    ioc.run();
}
