idf_component_register(
    SRCS "DHT.cpp"
    INCLUDE_DIRS .
    REQUIRES esp_driver_rmt esp_driver_gpio Task
)
