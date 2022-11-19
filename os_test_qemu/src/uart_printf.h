#if !defined(_UART_PRINTF_)
#define _UART_PRINTF_

int printf(const char *format, ...);

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

void printContextSnapshot();

#if defined(__cplusplus)
}
#endif // __cplusplus




#endif // _UART_PRINTF_
