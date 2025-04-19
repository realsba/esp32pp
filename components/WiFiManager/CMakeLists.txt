idf_component_register(
    SRCS "WiFiManager.cpp"
    INCLUDE_DIRS .
    REQUIRES esp_wifi asio sock_utils
)
