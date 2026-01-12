idf_component_register(SRCS "gattc_wifi.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES gattc_comp
                                            wifim_comp)
