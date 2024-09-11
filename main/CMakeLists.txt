idf_component_register(
    SRCS 
    "esp32_test.cpp"
    #"i2clcd.cpp"
    #"lcdapi.cpp"
    INCLUDE_DIRS "." "include" 
    REQUIRES i2clcd
    )
