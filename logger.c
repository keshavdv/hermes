/*
 * Copyright 2015, Keshav Varma
 * All Rights Reserved.
 */

#include "wiced.h"
#include <stdio.h>
#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "protocol/messages.pb.h"
#include <inttypes.h>
#include "cqueue.h"
#include "device_config.h"

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

volatile uint32_t sample = 0;
static device_state state = IDLE;
static wiced_thread_t      tcp_thread, poll_thread;
static wiced_bool_t        quit = WICED_FALSE;
static wiced_queue_t    sample_queue;
static wiced_time_t     sys_time;
static volatile tcp_server_handle_t tcp_server_handle;
static uint64_t nt_time;
static const wiced_ip_setting_t ap_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};
static Queue* dataBuffer;

/******************************************************
 *               Function Definitions
 ******************************************************/
volatile int led_on = 0;
volatile datum* tmpDatum;

void TIM2_irq()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        /* Blink led */
        if(led_on) {
            wiced_gpio_output_high(WICED_LED1);
            led_on = !led_on;
        } else {
            wiced_gpio_output_low(WICED_LED1);
            led_on = !led_on;
        }

        tmpDatum = (datum*) queue_enqueue(dataBuffer);
        if (tmpDatum != NULL) {
            tmpDatum->ts = sample;
            tmpDatum->d0 = wiced_gpio_input_get(WICED_GPIO_14);
            wiced_adc_take_sample(WICED_ADC_4, &tmpDatum->a0);
        } else {
            // wiced_gpio_output_high(WICED_LED1);
        }
        sample++;
    }
}

void InitializeTimer()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 120000-1;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 100-1;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void EnableTimerInterrupt()
{
    WPRINT_APP_INFO(("Starting to sample!\n"));
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void DisableTimerInterrupt()
{
    WPRINT_APP_INFO(("Stop sampling!\n"));
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvicStructure);
}

bool encode_state(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    SensorValue sv = SensorValue_init_zero;
    datum *data = (datum*) *arg;

    sv.id = 1;
    sv.value = data->d0;
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    if (!pb_encode_submessage(stream, SensorValue_fields, &sv))
        return false;

    sv.id = 2;
    sv.value = data->a0;
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    if (!pb_encode_submessage(stream, SensorValue_fields, &sv))
        return false;

    return true;
}

bool encode_payload(pb_ostream_t *stream, const pb_field_t messagetype[], const void *message)
{
    const pb_field_t *field;
    for (field = Payload_fields; field->tag != 0; field++)
    {
        if (field->ptr == messagetype)
        {
            /* This is our field, encode the message using it. */
            if (!pb_encode_tag_for_field(stream, field))
                return false;

            return pb_encode_submessage(stream, messagetype, message);
        }
    }

    /* Didn't find the field for messagetype */
    return false;
}

void application_start(void)
{
    /* Initialize the device */
    WPRINT_APP_INFO(("Starting data logger...\n"));

    wiced_init();
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

    wiced_rtos_delay_milliseconds(1000);

    /* Initialize analog pins */
    wiced_adc_init( WICED_ADC_4, 5 ); // PC2

    wiced_init_nanosecond_clock();
    wiced_reset_nanosecond_clock();
    dataBuffer = queue_initialize(sizeof(datum), 2000);

    /* Configure sample timer */
    WPRINT_APP_INFO(("Initializing sample timer...\n"));
    InitializeTimer();

    /* Continuously send buffered data */
    datum *pending;
    wiced_packet_t* tx_packet;
    char*           tx_data;
    uint16_t        available_data_length;

    while(WICED_TRUE) {
        int err = queue_dequeue(dataBuffer, (void**)&pending);
        if(err == 0) {
            WPRINT_APP_INFO(("Sending... (%d->%d->%d)\n", pending->ts, pending->d0, pending->a0));
            bool status;

            /* Send echo back */
            if (wiced_packet_create_tcp(&tcp_server_handle.socket, TCP_PACKET_MAX_DATA_LENGTH, &tx_packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
            {
                WPRINT_APP_INFO(("TCP packet creation failed\n"));
                continue;
            }

            Payload message = Payload_init_zero;
            pb_ostream_t stream = pb_ostream_from_buffer(tx_data + 4, available_data_length);

            message.type = Payload_MsgType_SensorState;
            SensorState ss = SensorState_init_zero;
            ss.timestamp = pending->ts;

            ss.messages.funcs.encode = &encode_state;
            ss.messages.arg = pending;
            encode_payload(&stream, SensorState_fields, &ss);

            if (!status)
            {
                printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
                continue;
            }

            status = pb_encode(&stream, Payload_fields, &message);
            int pkt_len = stream.bytes_written;

            if (!status)
            {
                printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
                continue;
            }

            /* Add length prefix */
            uint32_t message_length = pkt_len + sizeof(uint32_t);
            WPRINT_APP_INFO(("length %d\n", message_length));
            tx_data[0] = (message_length >> 24) & 0xFF;
            tx_data[1] = (message_length >> 16) & 0xFF;
            tx_data[2] = (message_length >> 8) & 0xFF;
            tx_data[3] = (message_length) & 0xFF;

            /* Set the end of the data portion */
            wiced_packet_set_data_end(tx_packet, (uint8_t*)tx_data + message_length);

            /* Send the TCP packet */
            if (wiced_tcp_send_packet(&tcp_server_handle.socket, tx_packet) != WICED_SUCCESS)
            {
                WPRINT_APP_INFO(("TCP packet send failed\n"));

                /* Delete packet, since the send failed */
                wiced_packet_delete(tx_packet);
                tcp_server_handle.quit=WICED_TRUE;
                continue;
            }
            WPRINT_APP_INFO(("Sent data.\n"));
        } else {
            wiced_rtos_delay_milliseconds(2);
        }
    }
}

static wiced_result_t tcp_server_process(  tcp_server_handle_t* server, wiced_packet_t* rx_packet )
{
    char*           request;
    uint16_t        request_length;
    uint16_t        available_data_length;

    wiced_packet_get_data( rx_packet, 0, (uint8_t**) &request, &request_length, &available_data_length );

    /* Allocate space for the decoded message. */
    Payload message = Payload_init_zero;

    pb_istream_t stream = pb_istream_from_buffer(request, request_length);
    bool status = pb_decode(&stream, Payload_fields, &message);

    /* Check for decoding errors */
    if (!status)
    {
        WPRINT_APP_INFO(("Decoding failed: %s\n", PB_GET_ERROR(&stream)));
        return WICED_ERROR;
    }

    switch(state) {

        case IDLE:
            if(message.type == Payload_MsgType_DeviceControlRequest) {

                // Handle requests
                switch(message.request.action) {
                    case DeviceControlRequest_Action_START:
                        sample = 0;
                        EnableTimerInterrupt();
                        break;
                    case DeviceControlRequest_Action_STOP:
                        DisableTimerInterrupt();
                        break;
                    case DeviceControlRequest_Action_RESET:
                        break;
                    case DeviceControlRequest_Action_SET_CONFIG:
                        break;
                    case DeviceControlRequest_Action_GET_CONFIG:
                        break;
                }
            }

            break;

        case POLL:
            // Only respond to stop request if we are currently sampling
            if(message.type == Payload_MsgType_DeviceControlRequest) {
                if(message.request.action == DeviceControlRequest_Action_STOP) {
                    DisableTimerInterrupt();
                }
            }
            break;
    }


    return WICED_SUCCESS;
}

static void tcp_server_thread_main(uint32_t arg)
{
    uint8_t buffer[1024];
    size_t message_length;
    bool status;

    tcp_server_handle_t* server = (tcp_server_handle_t*) arg;
    WPRINT_APP_INFO(("Waiting for connection...\n"));
    while ( quit != WICED_TRUE ) {
        wiced_packet_t* temp_packet = NULL;

        /* Wait for a connection */
        wiced_result_t result = wiced_tcp_accept(&server->socket);
        result = wiced_tcp_enable_keepalive(&server->socket, TCP_SERVER_KEEP_ALIVE_INTERVAL, TCP_SERVER_KEEP_ALIVE_PROBES, TCP_SERVER_KEEP_ALIVE_TIME );
        if( result != WICED_SUCCESS )
        {
            WPRINT_APP_INFO(("Keep alive initialization failed \n"));
        }

        if (result == WICED_SUCCESS) {
            /* Wait for start message. Receive the query from the TCP client */
            while (wiced_tcp_receive(&server->socket, &temp_packet, WICED_WAIT_FOREVER) == WICED_SUCCESS) {

                /* Process the client request */
                tcp_server_process(server, temp_packet);
                wiced_packet_delete(temp_packet);

            }
            /* Send failed or connection has been lost, close the existing connection and */
            /* get ready to accept the next one */
            WPRINT_APP_INFO(("Closing connection...\n"));
            wiced_tcp_disconnect( &server->socket );
            DisableTimerInterrupt();
        }
    }
    WPRINT_APP_INFO(("Disconnect\n"));
    wiced_tcp_disconnect( &server->socket );
    WICED_END_OF_CURRENT_THREAD();
}

