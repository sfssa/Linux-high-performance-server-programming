# 第一篇
## 第一章：TCP/IP协议族
`/etc/services`：可以查看机器支持的所有协议，包括端口号、支持的协议
`arp` / `arp -a`：查看ARP缓存
`sudo arp -d ip地址`：删除对应IP地址的缓存项
`sudo arp - s ip地址 mac地址`：添加对应IP地址的缓存项
`/etc/resolv.conf`：存放DNS服务器的IP地址
`host -t A 域名`：查看域名的IP和别名
