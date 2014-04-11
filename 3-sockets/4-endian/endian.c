#include <stdio.h>

union u {
    short sh;
    char ch[sizeof(short)];
};

/* Consider a 16-bit integer that is made up of 2 bytes. There are two ways
 * to store the two bytes in memeory: with the low-order byte at the
 * starting address, known as little-endian byte order, or with the high-
 * order byte at the starting address, known as big-endian byte order.
 * 即,Little-endian是低位字节存放在内存的低地址端,高位字节存放在高地址端.
 * Big-endian是高位字节存放在内存的低地址端,低位字节存放在内存的高地址端.
 *
 * 两个字节的整型数0x0102在Little-endian和Big-endian模式中的存放方式如下:
 *      ---------------------------------------------------------
 *      | 内存地址 | Little-endian存放内容 | Big-endian存放内容 |
 *      |----------|-----------------------|--------------------|
 *      |  低地址  |         0x02          |       0x01         |
 *      |  高地址  |         0x01          |       0x02         |
 *      ---------------------------------------------------------
 *
 * The terms "little-endian" and "big-endian" indicate which end of the
 * multibyte value, the little end or the big end, is stored at the
 * startnig address of the value.
 */
int main(void)
{
    union u test;
    /* union u test = 0x0102; 写成这样会报错: invalid initializer.
     * union u test = {0x0102}; 写成不会报错,能正常编译并运行.
     * 但最好还是写成下面的形式,为test.sh赋值,而不是为test赋值.
     * 从中可知,为union变量赋值时,要直接引用它的成员,而不是它本身.
     * 如果要直接为union变量赋值,则要用大括号括起来,且所赋值的类型
     * 要求和union变量第一个成员的类型一致,否则编译可能报警.例如:
     * union u test = {0x0102};
     */
    test.sh = 0x0102;

    if (sizeof(short) == 2) {
        /* 数组的第一个元素位于内存低地址,第二个元素位于内存高地址.
         * 当数组第一个元素(也就是内存低地址)存放short类型值的高位
         * 字节,数组第二个元素存放低位字节时,按照上面的图例,这属于
         * 大端模式.相应的,当数组第一个元素(也就是内存低地址)存放
         * short类型值的低位字节,数组第二个元素存放高位字节时,这就
         * 属于小端模式.
         */
        if (test.ch[0] == 0x01 && test.ch[1] == 0x02)
            printf("The host order byte is Big-endian\n");
        else if (test.ch[0] == 0x02 && test.ch[1] == 0x01)
            printf("The host order byte is Little-endian\n");
        else
            printf("unknown\n");
    }
    else
        printf("sizeof(short) = %lu\n", sizeof(short));

    return 0;
}
