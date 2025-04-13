idf_component_register(
    SRCS
        "ActionSequence.cpp"
        "LedController.cpp"
    INCLUDE_DIRS .
    REQUIRES driver esp_driver_ledc asio sock_utils
)
