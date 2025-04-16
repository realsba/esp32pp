idf_component_register(
    SRCS "Button.cpp"
    INCLUDE_DIRS .
    REQUIRES driver Task
    PRIV_REQUIRES CriticalSection
)
