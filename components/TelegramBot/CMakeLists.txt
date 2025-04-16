idf_component_register(
    SRCS "TelegramBot.cpp"
    INCLUDE_DIRS .
    REQUIRES esp_http_client esp-tls
)
