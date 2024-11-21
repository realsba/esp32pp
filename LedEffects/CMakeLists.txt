idf_component_register(
    SRCS
        "ActionSequence.cpp"
        "LedController.cpp"
    INCLUDE_DIRS .
    REQUIRES driver asio esp_driver_ledc
)
