# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/zodd/esp/esp-idf/components/bootloader/subproject"
  "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader"
  "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader-prefix"
  "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader-prefix/tmp"
  "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader-prefix/src/bootloader-stamp"
  "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader-prefix/src"
  "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/zodd/Documentos/@Documentos/Master2025/2025_Master_Universitario_IoT/2-RP1/laboratorio/miot-rpi-i/src/lab1/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
