idf_component_register(SRCS "gatt_touch_server.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES gatt_server_comp 
                                             touch_pad_comp)
