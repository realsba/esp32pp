idf_component_register(
    SRCS "PwmController.cpp"
    INCLUDE_DIRS .
    REQUIRES esp_driver_ledc
)
