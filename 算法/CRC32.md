# CRC (Cyclic Redundancy Check) 
循环冗余校验，是一种常见的检测数据一致性的算法

常用于网络/文件...等数据的一致性/完整性校验

在计算速度和空间占用上有一个良好的平衡，是一种非常主流的一致性校验算法

CRC计算中的一些概念
| 参数            | 说明                                 |
| --------------- | ------------------------------------ |
| Length          | CRC的长度(按bit算，如8,16,32)        |
| Name            | CRC的名字，让人一看就知道这是哪种CRC |
| Polinomial      | 多项式，通过该多项式来计算CRC        |
| InitialValue    | CRC的初始值                          |
| FinalXorValue   | CRC结果做异或运算的值                |
| InputReflected  | 指示输出是否需要翻转                 |
| OutputReflected | 指示输出是否需要翻转                 |


# CRC计算过程
```text
    1100001010 = Quotient (nobody cares about the quotient)
       _______________
10011 ) 11010110110000 = Augmented message (1101011011 + 0000)
 =Poly  10011,,.,,..|.
        -----,,.,,|...
         10011,.,,|.|.
         10011,.,,|...
         -----,.,,|.|.
              10110...
              10011.|.
              -----...
               010100.
                10011.
                -----.
                 01110
                 00000
                 -----
                  1110 = Remainder = THE CHECKSUM!!!!
```
这里的“除法” 用的是异或运算
例如 两个二进制数  1011 “除以”1100 --> 0111

```
从最开始的除以指定多项式，得到余数然后继续对余数除以多项式，循环往复，直到余数的有效位数小于指定多项式的有效位数,此时的余数就是最终的校验码,所谓的CRC8/CRC32后面的数字就是最终的校验码的长度,指定的多项式除数叫做“生成多项式”是固定的，而且有效位数位比最终的校验码多一位

（ps:这里的有效位数指的是最高次+1，例如 1001最高次为3,它的有效位数为3+1->4)
```

crc8的计算代码
```cpp
const uint8_t poly = 0x07;
uint8_t crc8(uint8_t *data, size_t length){
    uint8_t crc = 0x00;
    for (size_t i = 0; i < length; i++){
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++){
            if (crc & 0x80) crc = (crc << 1) ^ poly;
            else crc <<= 1;
        }
    }
    return crc;
}

```

# 标准CRC32
CRC有很多标准这里主要介绍应用最广泛的 “标准CRC32”
标准 CRC-32（也称 CRC-32/ISO-HDLC、CRC-32/ADCCP）

| 参数                     | 值                                                   |
| ------------------------ | ---------------------------------------------------- |
| 生成多项式（Polynomial） | 0x04C11DB7（正向）常以反向形式 0xEDB88320 用于查表法 |
| 初始值（Initial Value）  | 0xFFFFFFFF                                           |
| 输入反转（RefIn）        | true（每个字节 bit-reversed）                        |
| 输出反转（RefOut）       | true（最终结果 bit-reversed）                        |
| 最终异或值（XorOut）     | 0xFFFFFFFF                                           |

