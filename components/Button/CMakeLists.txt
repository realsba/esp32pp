idf_component_register(
    SRCS "Button.cpp"
    INCLUDE_DIRS .
    REQUIRES esp_driver_gpio Task
    PRIV_REQUIRES CriticalSection
)
