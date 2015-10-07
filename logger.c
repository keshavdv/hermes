/*
 * Copyright 2015, Keshav Varma
 * All Rights Reserved.
 */

#include "wiced.h"
#include <stdio.h>
#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "messages.pb.h"
#include <inttypes.h>

/******************************************************
 *                      Macros
 ******************************************************/

#define TCP_PACKET_MAX_DATA_LENGTH          (30)
#define TCP_SERVER_LISTEN_PORT              (5000)
#define TCP_SERVER_THREAD_PRIORITY          (WICED_DEFAULT_LIBRARY_PRIORITY)
/* Stack size should cater for printf calls */
#define TCP_SERVER_STACK_SIZE               (6200)
#define TCP_SERVER_COMMAND_MAX_SIZE         (10)
#define TCP_PACKET_MAX_DATA_LENGTH          (30)

/* Enable this define to demonstrate tcp keep alive procedure */
/* #define TCP_KEEPALIVE_ENABLED */

/* Keepalive will be sent every 2 seconds */
#define TCP_SERVER_KEEP_ALIVE_INTERVAL      (2)
/* Retry 10 times */
#define TCP_SERVER_KEEP_ALIVE_PROBES        (5)
/* Initiate keepalive check after 5 seconds of silence on a tcp socket */
#define TCP_SERVER_KEEP_ALIVE_TIME          (5)
#define TCP_SILENCE_DELAY                   (30)


/******************************************************
 *                    Constants
 ******************************************************/

static int POLL_DELAY = 1000;
/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct
{
    wiced_bool_t quit;
    wiced_tcp_socket_t socket;
} tcp_server_handle_t;
/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static void tcp_server_thread_main(uint32_t arg);
static void poll_thread_main(uint32_t arg);
static wiced_result_t tcp_server_process(  tcp_server_handle_t* server, wiced_packet_t* rx_packet );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static uint32_t sample = 0;
static wiced_thread_t      tcp_thread, poll_thread;
static wiced_bool_t        quit = WICED_FALSE;
static wiced_queue_t    sample_queue;
static wiced_time_t     sys_time;
static tcp_server_handle_t tcp_server_handle;
static uint64_t nt_time;
static const wiced_ip_setting_t ap_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};

/******************************************************
 *               Function Definitions
 ******************************************************/
void poll_callback();

volatile int led_on = 0;

void TIM2_irq()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        if(led_on) {
            wiced_gpio_output_high(WICED_LED1);
            led_on = !led_on;
        } else {
            wiced_gpio_output_low(WICED_LED1);
            led_on = !led_on;
        }

    }
}

void InitializeTimer()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 40000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 500;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void EnableTimerInterrupt()
{
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void application_start(void)
{
    /* Initialize the device */
    WPRINT_APP_INFO(("Starting data logger...\n"));

    wiced_init();
    // wiced_gpio_output_high(WICED_LED1);
    wiced_network_up(WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ap_ip_settings);

     /* Create a TCP server socket */
    if (wiced_tcp_create_socket(&tcp_server_handle.socket, WICED_AP_INTERFACE) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP socket creation failed\n"));
    }

    if (wiced_tcp_listen( &tcp_server_handle.socket, TCP_SERVER_LISTEN_PORT ) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP server socket initialization failed\n"));
        wiced_tcp_delete_socket(&tcp_server_handle.socket);
        return;
    }

    /* Start a tcp server thread */
    WPRINT_APP_INFO(("Creating tcp server on port %d\n", TCP_SERVER_LISTEN_PORT));
    wiced_rtos_create_thread(&tcp_thread, TCP_SERVER_THREAD_PRIORITY, "TCP server", tcp_server_thread_main, TCP_SERVER_STACK_SIZE, &tcp_server_handle);

    // wiced_rtos_delay_milliseconds(2000);

    /* Start polling inputs */

    wiced_init_nanosecond_clock();
    wiced_reset_nanosecond_clock();
    WPRINT_APP_INFO(("Init timer\n"));

    wiced_timer_t timer;
    if(wiced_rtos_init_timer(&timer, 10000, poll_callback, NULL) != WICED_SUCCESS) {
        WPRINT_APP_INFO(("Timer initialization failed\n"));
    }
    WPRINT_APP_INFO(("start timer\n"));
    if(wiced_rtos_start_timer(&timer) != WICED_SUCCESS) {
        WPRINT_APP_INFO(("Timer failed to start\n"));
    } else {
        WPRINT_APP_INFO(("Timer started\n"));
    }
    InitializeTimer();
    EnableTimerInterrupt();
    while(WICED_TRUE) {
        // wiced_gpio_output_high(WICED_LED1);
        // nt_time = wiced_get_nanosecond_clock_value();
        // WPRINT_APP_INFO(("Sampling: #%d\n", sample));
        // WPRINT_APP_INFO(("Time is: %d\n", sys_time));
        wiced_rtos_delay_milliseconds(POLL_DELAY);
        // wiced_gpio_output_low(WICED_LED1);
        wiced_rtos_delay_milliseconds(POLL_DELAY);
    }
}

void poll_callback(){
    nt_time = wiced_get_nanosecond_clock_value();
    wiced_time_get_time(&sys_time);
    WPRINT_APP_INFO(("Time is %d, %llu\n", sys_time, nt_time));
}

static wiced_result_t tcp_server_process(  tcp_server_handle_t* server, wiced_packet_t* rx_packet )
{
    char*           request;
    uint16_t        request_length;
    uint16_t        available_data_length;
    wiced_packet_t* tx_packet;
    char*           tx_data;

    wiced_packet_get_data( rx_packet, 0, (uint8_t**) &request, &request_length, &available_data_length );

    /* Null terminate the received string */
    request[request_length] = '\x0';
    WPRINT_APP_INFO(("Received data: %s \n", request));

    /* Send echo back */
    if (wiced_packet_create_tcp(&server->socket, TCP_PACKET_MAX_DATA_LENGTH, &tx_packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet creation failed\n"));
        return WICED_ERROR;
    }

    /* Write the message into tx_data"  */
    tx_data[request_length] = '\x0';
    memcpy(tx_data, request, request_length);

    /* Set the end of the data portion */
    wiced_packet_set_data_end(tx_packet, (uint8_t*)tx_data + request_length);

    /* Send the TCP packet */
    if (wiced_tcp_send_packet(&server->socket, tx_packet) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet send failed\n"));

        /* Delete packet, since the send failed */
        wiced_packet_delete(tx_packet);

        server->quit=WICED_TRUE;
        return WICED_ERROR;
    }
    WPRINT_APP_INFO(("Echo data: %s\n", tx_data));

    return WICED_SUCCESS;
}

static void tcp_server_thread_main(uint32_t arg)
{
    uint8_t buffer[1024];
    size_t message_length;
    bool status;

    tcp_server_handle_t* server = (tcp_server_handle_t*) arg;
    WPRINT_APP_INFO(("Waiting for connection.\n"));
    while ( quit != WICED_TRUE ) {
        wiced_packet_t* temp_packet = NULL;

        /* Wait for a connection */
        wiced_result_t result = wiced_tcp_accept(&server->socket);
        if (result == WICED_SUCCESS) {

            /* Wait for start message. Receive the query from the TCP client */
            if (wiced_tcp_receive(&server->socket, &temp_packet, WICED_WAIT_FOREVER) == WICED_SUCCESS) {

                /* Process the client request */
                tcp_server_process(server, temp_packet);
                wiced_packet_delete(temp_packet);

            } else {
                /* Send failed or connection has been lost, close the existing connection and */
                /* get ready to accept the next one */
                wiced_tcp_disconnect( &server->socket );
            }
        }
    }
    WPRINT_APP_INFO(("Disconnect\n"));
    wiced_tcp_disconnect( &server->socket );
    WICED_END_OF_CURRENT_THREAD();
}

