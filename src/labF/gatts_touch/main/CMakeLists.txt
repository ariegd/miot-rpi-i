idf_component_register(SRCS "gatts_touch.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES gatts_comp 
                                             touch_pad_comp)
