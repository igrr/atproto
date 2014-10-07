#include "ets_sys.h"
#include "mem.h"
#include "driver/uart_register.h"
#include "uart.h"

#define UART_TX_FIFO_SIZE 0x7f

struct _uart
{
    int  baud_rate;
    uart_rx_handler_t rx_handler;
};

void ICACHE_FLASH_ATTR uart0_rx_handler(uart_t* uart)
{
    while(true)
    {
        int rx_count = (READ_PERI_REG(UART_STATUS(0)) >> UART_RXFIFO_CNT_S) & UART_RXFIFO_CNT;
        if (!rx_count)
            break;

        for(int cnt = 0; cnt < rx_count; ++cnt)
        {
            char c = READ_PERI_REG(UART_FIFO(0)) & 0xFF;
            (*uart->rx_handler)(c);
        }
    }
}

void ICACHE_FLASH_ATTR uart0_wait_for_tx_fifo(size_t size_needed)
{
    while (true)
    {
        size_t tx_count = (READ_PERI_REG(UART_STATUS(0)) >> UART_TXFIFO_CNT_S) & UART_TXFIFO_CNT;
        if (tx_count < (UART_TX_FIFO_SIZE - size_needed))
            break;
    }
}

void ICACHE_FLASH_ATTR uart0_transmit(const char* buf, size_t size)
{
    while (size)
    {
        size_t part_size = (size > UART_TX_FIFO_SIZE) ? UART_TX_FIFO_SIZE : size;

        uart0_wait_for_tx_fifo(part_size);
        for(;part_size;--part_size, ++buf)
            WRITE_PERI_REG(UART_FIFO(0), *buf);
    }
}

void ICACHE_FLASH_ATTR uart0_flush(uart_t* uart)
{
    SET_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST | UART_TXFIFO_RST);
}

void ICACHE_FLASH_ATTR uart0_interrupt_enable(uart_t* uart)
{
    WRITE_PERI_REG(UART_INT_CLR(0), 0x1ff);
    ETS_UART_INTR_ATTACH(&uart0_rx_handler, uart);
    SET_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA);
    ETS_UART_INTR_ENABLE();
}

void ICACHE_FLASH_ATTR uart0_interrupt_disable(uart_t* uart)
{
    SET_PERI_REG_MASK(UART_INT_ENA(0), 0);
    ETS_UART_INTR_DISABLE();
}

uart_t* ICACHE_FLASH_ATTR uart0_init(int baud_rate, uart_rx_handler_t rx_handler)
{
    uart_t* uart = (uart_t*) os_malloc(sizeof(uart_t));
    uart->baud_rate = baud_rate;
    uart->rx_handler = rx_handler;

    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

    uart_div_modify(0, UART_CLK_FREQ / (uart->baud_rate));  // baud rate
    WRITE_PERI_REG(UART_CONF0(0), 0x3 << UART_BIT_NUM_S);   // 8n1

    uart0_flush(uart);
    uart0_interrupt_enable(uart);
    
    WRITE_PERI_REG(UART_CONF1(0), ((0x01 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S));

    return uart;
}


void ICACHE_FLASH_ATTR uart0_uninit(uart_t* uart)
{
    uart0_interrupt_disable(uart);
    // TODO: revert pin functions
    os_free(uart);
}
