# CMakeLists for ESP32 Tamagotchi Emulator

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(espgotchi)

# SPIFFS用のファイルシステムイメージを作成
spiffs_create_partition_image(storage ${CMAKE_SOURCE_DIR}/spiffs)
