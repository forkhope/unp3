#include <stdio.h>
#include <arpa/inet.h>

void print_hex_byte(uint32_t value, unsigned size)
{
    int i;
    char *p;

    printf("-----打印值 '%#x' 的各个字节如下----\n", value);
    for (i = 0, p = (char *)&value; i < size; ++i) {
        printf("byte %d: address: %p, value: %#x\n", i, &p[i], p[i]&0xff);
    }
    putchar('\n');
}

/* We must deal with these byte ordering differences as network programmers
 * because networking protocols must specify a network byte order. For
 * example, in a TCP segment, there is a 16-bit port number and a 32-bit
 * IPv4 address. The sending protocol stack and the receiving protocol stack
 * must agree on the order in which the bytes of these multibyte fields will
 * be transmitted. The Internet protocols use big-endian byte ordering for
 * these multibyte integers. We use the following four functions to convert
 * between these two byte orders.
 * #include <netinet/in.h>
 * uint16_t htons(uint16_t host16bitvalue);
 * uint32_t htonl(uint32_t host32bitvalue);
 *          Both return: value in network byte order
 * uint16_t ntohs(uint16_t net16bitvalue);
 * uint16_t ntohl(uint32_t net32bitvalue);
 *          Both return: value in host byte order
 * 书中注明,这四个函数的头文件是<netinet/in.h>,但是在Linux系统中,man htonl
 * 显示这四个函数的头文件是<arpa/inet.h>,且在"CONFORMING TO"小节中提到,"Some
 * systems require the inclusion of <netinet/in.h> instead of <arpa/inet.h>
 * 查看POSIX在线网址对 <netinet/in.h> 头文件的说明,里面提到: The htonl(),
 * htons(), ntohl(), and ntohs() functions shall be available as described
 * in <arpa/inet.h>. Inclusion of the <netinet/in.h> header may also make
 * visible all symbols from <arpa/inet.h>.也就是包含<netinet/in.h>头文件可
 * 能也会包含到<arpa/inet.h>.
 * (pubs.opengroup.org/onlinepubs/9699919799/basedefs/netinet_in.h.html)
 * 实际调试发现,Linux系统就是这种情况.
 *
 * In the names of these functions, h stands for host, n stands for network,
 * s stands for short, and l stands for long. The terms "short" and "long"
 * are historical artifacts from the Digital VAX implementation of 4.2BSD.
 * We should instead think of s as a 16-bit value (such as a TCP or UDP port
 * number) and l as a 32-bit value (such as an IPv4 address).
 *
 * We have not yet defined the term "byte". We use the term to mean an 8-bit
 * quantity since almost all current computer systems use 8-bit bytes. Most
 * Internet standards use the term octet instead of byte to mean an 8-bit
 * quantity. This started in the early days of TCP/IP because much of the
 * early work was done on systems such as the DEC-10, which did not use
 * 8-bit bytes.
 */
int main(void)
{
    /* uint32_t, uint16_t 等数据类型是在<stdint.h>头文件中声明.<inttypes.h>
     * 头文件会包含<stdint.h>头文件,<arpa/inet.h>头文件会按照<inettypes.h>
     * 头文件的要求来定义uint32_t, uint16_t等数据类型.<arpa/inet.h>头文件在
     * POSIX在线网址中有所描述.里面提到:
     * (pubs.opengroup.org/onlinepubs/9699919799/basedefs/arpa_inet.h.html)
     * The <arpa/inet.h> header shall define the uint32_t* and uint16_t
     * types as described in <inttypes.h>.所以上面包含了<arpa/inet.h>头文件
     * 之后,不再需要包含其他头文件来引入uint32_t的声明.
     */
    uint32_t hl = 0x1234abcd, nl;
    uint16_t hs = 0x9876, ns;

    printf("主机字节序: 0x1234abcd 的值为: %#x\n", hl);
    print_hex_byte(hl, sizeof(hl));

    nl = htonl(hl);
    /* 将 0x1234abcd 转换为网络字节序后,下面两个语句打印出来的结果如下:
     *      将 0x1234abcd 转换为网络字节序得到的值为: 0xcdab3412
     *      byte 0: address: 0x7fff6fd9c598, value: 0x12
     *      byte 1: address: 0x7fff6fd9c599, value: 0x34
     *      byte 2: address: 0x7fff6fd9c59a, value: 0xab
     *      byte 3: address: 0x7fff6fd9c59b, value: 0xcd
     * 可以看到,转换后的值为0xcdab3412.当逐个字节打印该值时,还是会先打印
     * 低位字节0x12,最后打印高位字节0xcd.也就是说,相对于0xcdab3412来说,
     * 还是按照主机字节序来存放.只不过实际上我们想要传输的值是0x1234abcd,
     * 而转换后,变成了传输0xcdab3412,传输的值变了,而不是该值的存放方式变了.
     * 可能有一种想法是执行htonl()函数后,将转换后的值打印出来,会发现低地址
     * 存放高位字节,例如上面应该先打印0xcd,最后再打印0x12.实际上,这种想法
     * 是错误的.执行htonl()函数后,得到的值还是按照主机字节序来存放.如果本
     * 机是小端模式,那么就是低地址存放低位字节.小端模式下,htonl()函数只是
     * 会返回另外一个值,如果对端也是小端模式,则对端接收到该值后,将这个值再
     * 转换为小端字节序,就得到原来的值.
     */
    printf("将 0x1234abcd 转换为网络字节序得到的值为: %#x\n", nl);
    print_hex_byte(nl, sizeof(nl));

    printf("主机字节序: 0x9876 的值为: %#x\n", hs);
    print_hex_byte(hs, sizeof(hs));

    ns = htons(hs);
    printf("将 0x9876 转换为网络字节序得到的值为: %#x\n", ns);
    print_hex_byte(ns, sizeof(ns));

    hl = ntohl(nl);
    printf("将网络字节序 '%#x' 转换为主机字节序的值为: %#x\n", nl, hl);

    hs = ntohs(ns);
    printf("将网络字节序 '%#x' 转换为主机字节序的值为: %#x\n", ns, hs);

    return 0;
}
