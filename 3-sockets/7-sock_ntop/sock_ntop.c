#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

/* sock_ntop() takes a pointer to a socket address structure, looks inside
 * the structure, and calls the appropriate function to return the
 * presentation format of the address.
 * sa points to a socket address structure whose length is salen. The
 * function uses its own static buffer to hold the result and a pointer to
 * this buffer is the return value.
 * The presentation format is the dotted-decimal form of an IPv4 address or
 * the hex string form of an IPv6 address surrounded by brackets, followed
 * by a terminator (we use a colon, similar to URL syntax), followed by the
 * decimal port number, followed by a null character. Hence, the buffer size
 * must be at least INET_ADDRSTRLEN plus 6 bytes for IPv4 (16 + 6= 22), or
 * INET6_ADDRSTRLEN plus 8 bytes for IPv6 (46 + 8 = 54).
 */
static char *sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char portstr[8];
    /* 声明为static变量,是为了让函数返回后,下面的str[]还存在于静态存储
     * 区中.若是声明为局部变量,则函数返回后,str[]的内容可能会被覆盖.
     */
    static char str[128];  /* Unix domain is largest */

    switch (sa->sa_family) {
        case AF_INET: {
            /* 实际调试发现,gcc中,在label之后(例如case, goto语句等)直接
             * 声明变量时,编译就会报错.报错信息如下:
             * error: a label can only be part of a statement and a
             * declaration is not a statement
             * 有两个方法可以修改这个报错.(1)在label后面加上大括号.(2)
             * 不用大括号也可以,只要label后面的第一条语句不是变量声明
             * 语句就可以.
             */
            struct sockaddr_in *sin = (struct sockaddr_in *)sa;

            if (inet_ntop(AF_INET,&sin->sin_addr,str,sizeof(str)) == NULL) {
                return NULL;
            }
            /* 这里应该是有一个潜规则.如果传入的结构体填充port number,那么
             * sin_port成员会被置为0.实际上,ntohs()函数没有出错返回值,即使
             * 返回0,也不表示ntohs()函数执行出错.
             */
            if (ntohs(sin->sin_port) != 0) {
                snprintf(portstr, sizeof(portstr), ":%d",
                        ntohs(sin->sin_port));
                strcat(str, portstr);
            }
            return str;
        }
        default:
            snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, "
                    "len: %d", sa->sa_family, salen);
            return str;
    }

    return NULL;
}

int main(void)
{
    char *str;
    struct sockaddr_in in;

    in.sin_family = AF_INET;
    in.sin_port = htons(12345);
    if (inet_pton(AF_INET, "192.168.1.88", &in.sin_addr) <= 0) {
        perror("inet_pton: AF_INET error");
        return 1;
    }

    if ((str = sock_ntop((struct sockaddr *)&in, sizeof(in))) == NULL) {
        perror("sock_ntop error");
        return 1;
    }
    printf("after sock_ntop: str = %s\n", str);

    return 0;
}
