idf_component_register(SRCS "Button.cpp"
        INCLUDE_DIRS .
        REQUIRES Task driver
        PRIV_REQUIRES CriticalSection
        )
