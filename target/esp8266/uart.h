#ifndef ESP8266_UART_H
#define ESP8266_UART_H

#define UART_RX_BUFFER_SIZE 1024
#define UART_TX_BUFFER_SIZE 1024

typedef void (*uart_rx_handler_t)(char);

typedef struct _uart uart_t;

uart_t* uart0_init(int baud_rate, uart_rx_handler_t rx_handler);
void uart0_uninit(uart_t* uart);
void uart0_transmit(const char* buf, size_t size);	// may block on TX fifo 

#endif//ESP8266_UART_H
