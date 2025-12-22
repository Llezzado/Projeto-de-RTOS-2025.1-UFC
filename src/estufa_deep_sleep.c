#include <stdio.h>
#include <hardware/gpio.h>
#include <hardware/sleep.h>
#include <pico/stdlib.h>
#include <dth11.h>
#include <wifi.h>>

#define SLEEP_DURATION 300000
#define WAKE_PIN 2
#define DHT_PIN 4

DHT dht(DHT_PIN, DHT11);

void setup() {
    stdio_init_all();

    gpio_init(WAKE_PIN);
    gpio_set_dir(WAKE_PIN, GPIO_IN);
    gpio_pull_up(WAKE_PIN);

    dht.begin();
    Wifi.mode(WIFI_OFF);
}

void read_sensors() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        printf("Failed to read from DHT sensor!\n");
        return
    } else {
        printf("Temperature: %.2f °C, Humidity: %.2f %%\n", temperature, humidity);
    }
}

void enter_deep_sleep() {
    sleep_run_xosc();
    sleep_goto_dormant_until_edge_high(WAKE_PIN);
}

int main() {
    setup();

    while (true) {
        read_sensors();
        enter_deep_sleep();
    }

    return 0;
}