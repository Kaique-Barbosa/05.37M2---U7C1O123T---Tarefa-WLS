cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == NÃO MODIFIQUE AS LINHAS ABAIXO para que a Extensão VS Code do Raspberry Pi Pico funcione corretamente ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Tipo de placa")

# Importa o SDK do Raspberry Pi Pico (deve ser antes da definição do projeto)
include(pico_sdk_import.cmake)

# Define o nome do projeto
project(atividadeFinal C CXX ASM)

# Inicializa o SDK do Raspberry Pi Pico
pico_sdk_init()

# Adiciona o executável principal
add_executable(atividadeFinal
    atividadeFinal.c
    inc/ssd1306.c
)

# Garante que a pasta "generated" exista e gera o cabeçalho PIO para a WS2812
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(atividadeFinal ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Define o nome do programa e a versão
pico_set_program_name(atividadeFinal "atividadeFinal")
pico_set_program_version(atividadeFinal "0.1")

# Habilita a saída via UART/USB
pico_enable_stdio_uart(atividadeFinal 1)
pico_enable_stdio_usb(atividadeFinal 1)

# Adiciona as bibliotecas padrão ao projeto
target_link_libraries(atividadeFinal
    pico_stdlib
    hardware_i2c
    hardware_gpio
    hardware_pwm
    hardware_pio  # Necessário para o controle da WS2812
)

# Adiciona os diretórios de inclusão ao projeto
target_include_directories(atividadeFinal PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    inc
)

# Gera o arquivo UF2 para facilitar o upload para a placa
pico_add_extra_outputs(atividadeFinal)
