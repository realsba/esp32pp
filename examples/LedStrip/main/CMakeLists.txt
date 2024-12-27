idf_component_register(
    SRCS "main.cpp"
    REQUIRES nvs_flash esp_netif esp_event LedEffects LedStrip
    INCLUDE_DIRS .
)
