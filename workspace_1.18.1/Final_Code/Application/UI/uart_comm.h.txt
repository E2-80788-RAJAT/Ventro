#ifndef __UART_COMM_H
#define __UART_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

void UART_Init(void);
void UART_SendString(const char *str);
char UART_ReceiveChar(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_COMM_H */
