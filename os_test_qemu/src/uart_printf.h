#if !defined(_UART_PRINTF_)
#define _UART_PRINTF_

int printf(const char *format, ...);
int raw_printf(const char *format, ...);
// #define LOGD(...) raw_printf(__VA_ARGS__)
#define LOGD(...) while(0)

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

void printContextSnapshot();

#if defined(__cplusplus)
}
#endif // __cplusplus




#endif // _UART_PRINTF_
