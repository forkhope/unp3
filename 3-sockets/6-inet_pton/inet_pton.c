#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

/* Simple version of inet_pton() that supports only IPv4.
 * #include <arpa/inet.h>
 * int inet_aton(const char *cp, struct in_addr *inp);
 *      Returns: 1 if string was valid, 0 on error
 * inet_aton() converts the Internet host address cp from the IPv4 numbers-
 * and-dots notation into binary form (in network byte order) and stores it
 * in the structure that inp points to. inet_aton() returns nonzero if the
 * address is valid, zero if not.
 * 这个函数不再建议使用,使用 inet_pton() 函数来替代它.
 */
int inet_pton_l(int family, const char *src, void *dst)
{
    if (family == AF_INET) {
        struct in_addr in_val;

        if (inet_aton(src, &in_val)) {
            memcpy(dst, &in_val, sizeof(in_val));
            return 1;
        }
        return 0;
    }
    errno = EAFNOSUPPORT;
    return -1;
}

/* Simple version of inet_ntop() that supports only IPv4.
 * #include <arpa/inet.h>
 * char *inet_ntoa(struct in_addr in);
 *      Returns: pointer to dotted-decimal string
 * The inet_ntoa() function converts the Internet host address in, given
 * in network byte order, to a string in IPv4 dotted-decimal notation. The
 * string is returned in a statically allocated buffer, which subsequent
 * calss will overwrite.
 * 这个函数不再建议使用,使用 inet_ntop() 函数来替代它.
 */
const char *inet_ntop_l(int family, const void *src,
        char *dst, socklen_t len)
{
    const u_char *p = (const u_char *)src;

    if (family == AF_INET) {
        char temp[INET_ADDRSTRLEN];

        /* 这里没有调用inet_ntoa()函数,而是自行做了转换.网络字节序是大端
         * 模式,低地址存放高位字节,高地址存放低位字节.下面将低地址的值放到
         * IPv4地址的前面,将高地址的值放到IPv4地址的后面.
         * 实际调试发现,将IP地址"192.168.1.88"转换为数字值后得到0x5801a8c0,
         * 将0x5801a8c0传递到这个函数来,则可以发现,低位字节0xc0正好对应十
         * 进制值192.0xa8正好对应十进制值168,0x01当然是对应1,0x58对应88,
         * 按照下面的方式组合起来,正好是"192.168.1.88",结果正确.
         */
        snprintf(temp, sizeof(temp), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        if (strlen(temp) >= len) {
            errno = ENOSPC;
            return NULL;
        }
        strcpy(dst, temp);
        return dst;
    }
    errno = EAFNOSUPPORT;
    return NULL;
}

/* The inet_pton() and inet_ntop() functions are new with IPv6 and work
 * with both IPv4 and IPv6 addresses. The letters "p" and "n" stand for
 * presentation and numeric. The presentation format for an address is
 * often an ASCII string and the numeric format is the binary value that
 * goes into a socket address structure.
 * #include <arpa/inet.h>
 * int inet_pton(int af, const char *src, void *dst);
 *  Returns: 1 if OK,0 if input not a valid presentation format,-1 on error
 * This function converts the character string src into a network address
 * structure in the af address family, then copies the network address
 * structure to dst. The af argument must be either AF_INET or AF_INET6.
 * If successful, the return value is 1. If the input string is not a valid
 * presentation format for the specified family, 0 is returned.
 *
 * AF_INET
 *      src points to a character string containing an IPv4 network address
 *      in dotted-decimal format, "ddd.ddd.ddd.ddd", where ddd is a decimal
 *      number of up to three digits in the range 0 to 255. The address is
 *      converted to a struct in_addr and copied to dst, which must be
 *      sizeof(struct in_addr) (4) bytes (32 bits) long.
 *
 * const char *inet_ntop(int af, const void *src,
 *      char *dst, socklen_t size);
 *      Returns: pointer to result if OK, NULL on error
 * This function converts the network address structure src in the af
 * address family into a character string. The resulting string is copied
 * to the buffer pointed to by dst, which must be a non-NULL pointer. The
 * caller specifies the number of bytes available in this buffer in the
 * argument size.
 * The size argument is the size of the destination, to prevent the function
 * from overflowing the caller's buffer. To help specify this size, the
 * following two definitions are defined by including the <netinet/in.h>:
 * #define INET_ADDRSTRLEN      16  // for IPv4 dotted-decimal
 * #define INET6_ADDRSTRLEN     46  // for IPv6 hex string
 * If size is too small to hold the resulting presentation format,
 * including the terminating null, a null pointer is returned and errno is
 * set to ENOSPC.
 * The dst argument to inet_ntop() cannot be a null pointer. The caller must
 * allocate memory for the destination and specify its size. On success,
 * this pointer is the return value of the function.
 *
 * !!!NOTE!!! 书中提到 INET_ADDRSTRLEN 和 INET6_ADDRSTRLEN 宏是定义在
 * <netinet/in.h>中.根据POSIX在线手册对<netinet/in.h>头文件的说明,确实如此.
 * (pubs.opengroup.org/onlinepubs/9699919799/basedefs/netinet_in.h.html)
 * 但是POSIX对<arpa/inet.h>头文件的说明中,也提到: The <arpa/inet.h> header
 * shall define the INET_ADDRSTRLEN and INET6_ADDRSTRLEN macros as
 * described in <netinet/in.h>.所以,上面只包含<arpa/inet.h>头文件,也能引入
 * INET_ADDRSTRLEN 和 INET6_ADDRSTRLEN 宏的声明.
 *
 * AF_INET
 *      src points to a struct in_addr (in network byte order) which is
 *      converted to an IPv4 network address in the dotted-decimal format,
 *      "ddd.ddd.ddd.ddd". The buffer dst must be at least INET_ADDRSTRLEN
 *      bytes long.
 *
 * The family argument for both functions is either AF_INET or AF_INET6. If
 * family is not supported, both functions return an error with errno set
 * to EAFNOSUPPORT.
 */
int main(void)
{
    const char *ip_addr = "192.168.1.88";
    struct in_addr numeric_addr;
    char str_addr[INET_ADDRSTRLEN];

    /* 将 "192.168.1.88" 转换为网络字节序的数字值,得到的结果是0x5801a8c0.
     * 根据这个结果,有这样的猜测:十进制192转换为十六进制是0xc0,168转换后是
     * 0xa8,1转换后是0x01,88转换后是0x58,去掉中间的'.',小端模式下得到的值
     * 是0xc0a80158.将这个值转换为网络字节序,得到0x5801a8c0.这个值正好是
     * inet_pton()函数转换"192.168.1.88"所得到的数字值.由于192是高位,大端
     * 模式下,得到的值就是0x5801a8c0,结果也是符合的.
     */
    if (inet_pton(AF_INET, ip_addr, &numeric_addr) <= 0) {
        perror("inet_pton: AF_INET error");
        return 1;
    }
    printf("after inet_pton: numeric_addr = %#x\n", numeric_addr.s_addr);
    printf("------------------------------------------------\n");

    if (inet_pton_l(AF_INET, ip_addr, &numeric_addr) <= 0) {
        perror("inet_pton_l: AF_INET error");
        return 1;
    }
    printf("after inet_pton_l: numeric_addr = %#x\n", numeric_addr.s_addr);
    printf("------------------------------------------------\n");

    /* 书中在调用inet_ntop()函数时,第二个参数一般都是传递struct in_addr
     * 结构体的指针.如下所示.struct in_addr结构体有一个s_addr成员,实际上,
     * 传递这个成员的指针到inet_ntop()函数也是可以的.参见后面的例子.
     */
    if (inet_ntop(AF_INET, &numeric_addr, str_addr,
                sizeof(str_addr) - 1) == NULL) {
        perror("inet_ntop: AF_INET error");
        return 1;
    }
    printf("after inet_ntop: str_addr = %s\n", str_addr);
    printf("------------------------------------------------\n");

    /* 使用struct in_addr结构体的s_addr成员指针作为inet_ntop()函数的
     * 第二个参数也是可以的,能获取到正确的结果.
     */
    if (inet_ntop(AF_INET, &numeric_addr.s_addr, str_addr,
                sizeof(str_addr) - 1) == NULL) {
        perror("inet_ntop: s_addr: AF_INET error");
        return 1;
    }
    printf("after inet_ntop: s_addr: str_addr = %s\n", str_addr);
    printf("------------------------------------------------\n");

    if (inet_ntop_l(AF_INET, &numeric_addr, str_addr,
                sizeof(str_addr) - 1) == NULL) {
        perror("inet_ntop_l: AF_INET error");
        return 1;
    }
    printf("after inet_ntop_l: str_addr = %s\n", str_addr);

    return 0;
}
