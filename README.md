### Dependencies

Download the latest nanopb library and place it in `<WICED-SDK-PATH>/libraries/nanopb/`

Create and add the following to a file called `nanopb.mk` within that directory

    NAME := Lib_nanopb
    $(NAME)_SOURCES := pb_common.c *.c
    GLOBAL_INCLUDES := .

### Development


To compile and download to the board:

    ./make EMW3162-FreeRTOS-LwIP-hermes run