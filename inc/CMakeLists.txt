add_executable(app
        main.c
        led.c
)

target_link_libraries(
    app
    pico_stdlib 
    lib 
    pico_stdio_usb
)

pico_add_extra_outputs(app)
#