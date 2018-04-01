本文件包含两个类，主要处理windows程序异常情况

代码说明
  1. 提供线程安全日之类 Logger
  2. windows程序异常检测，生成dump文件和错误定位报告日志
代码结构
  CLogger类  格式化日志输出类
  CMiniDumper windows程序异常检测，生成dump文件，并输出错误日志
