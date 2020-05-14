PROJECT(anal_mic_rel)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
ENABLE_LANGUAGE(ASM)

FIND_PACKAGE(CMSIS REQUIRED)
FIND_PACKAGE(STM32HAL REQUIRED)

INCLUDE_DIRECTORIES(
        Inc
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
)

#Сюда суем все файлы, кроме system_stm32fx.c

file(GLOB PROJECT_SOURCES "Src/*.c")

#LIST(REMOVE_ITEM PROJECT_SOURCES system_stm32f4xx.c)
#MESSAGE("${PROJECT_SOURCES}")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

#SET(PROJECT_SOURCES
#        Src/adc.c
#        Src/gpio.c
#        Src/main.c
#        Src/stm32f4xx_hal_msp.c
#        Src/stm32f4xx_it.c
#        Src/sys.c
#        Src/syscalls.c
#        Src/sys
#        Src/tim.c
#        Src/usart.c
#        Src/mic_direction.c
#        )

SET(STM32_LINKER_SCRIPT ${CMSIS_LINKER_SCRIPT})

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES} ${CMSIS_SOURCES} ${STM32HAL_SOURCES})

STM32_SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME})
STM32_ADD_HEX_BIN_TARGETS(${CMAKE_PROJECT_NAME})
STM32_PRINT_SIZE_OF_TARGETS(${CMAKE_PROJECT_NAME})
