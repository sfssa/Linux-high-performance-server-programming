#include <iostream>

/* 大端字节序更符合人类的习惯
 *
 * 大端字节序：整数高位字节存储到低位地址；低位字节存储到高位地址。
 * 小端字节序：整数高位字节存储到高位地址；低位字节存储到低位地址。
 *
 * 两台使用不同字节序的主机进行通信时会出现问题，解决办法是：
 * 发送端总是将数据转换成大端字节序后才发送；接收方以大端字节序的方式来接受数据。
 *
 * 网络中以大端字节序发送和接收，因此大端字节序又被称为网络字节序。
 * 现代计算机多数用小端字节序，因此小端字节序又被称为主机字节序。
 */

void byteOrder(){
    union test{
        short value;
        char unionBytes[sizeof(char)];
    };

    test t;
    t.value = 0x0102;
    if(t.unionBytes[0] == 1 && t.unionBytes[1] == 2){
        std::cout << "big endian" << std::endl;
    }else if(t.unionBytes[0] == 2 && t.unionBytes[1] == 1){
        std::cout << "small endian" << std::endl;
    }else{
        std::cout << "unknow..." << std::endl;
    }
}

int main()
{
    byteOrder();
    return 0;
}

