set(srcs "main.c"
        "board.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS  "."
                    PRIV_REQUIRES nvs_flash driver spi_flash esp_idf_lib_helpers shtc3)
