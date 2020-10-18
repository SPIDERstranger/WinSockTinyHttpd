# TinyHttpdForWindows

参考TinyHttpd实现，使用了一些C++的语法。

http接收到客户端连接时，使用了线程池来复刻原版中的创建pthread。

## 基本逻辑：

接收到http请求时，对请求报文进行接收，切分，获取url

通过url查找htdoc下是否存在文件，并发送文件。