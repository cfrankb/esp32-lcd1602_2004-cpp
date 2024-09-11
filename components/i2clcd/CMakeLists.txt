idf_component_register(SRCS "i2clcd.cpp" "lcdapi.cpp"
INCLUDE_DIRS "include"
REQUIRES esp_driver_i2c esp_driver_gpio)
