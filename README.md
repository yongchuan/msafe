msafe
=====

通过劫持Zend底层编译opcode，可以分析php执行的代码，从而达到还原部分混淆加密的php源码，并且可以根据自定义规则，审计代码安全，查看危险调用。

Update:
===
	1.增加PHP7支持
	2.增加system()，passthru(),exec(),shell_exec(),proc_open(),popen()支持
	3.增加msafe.log_path自定义存储路径
	4.增加masfe.msafe.disable_found 设置当检测到上述方法或者eval时是否执行

编译和安装
===
  	cd ***/msafe
  
  	phpize
  
	./configure --with-msafe
	
	make
	
	make install
	
	编译完成msafe.so加入php.ini
	
	extension=msafe.so
	
	msafe.enable_msafe = 1 或 0
	msafe.log_path=/tmp/log.log 自定义记录路径

使用
===
	1.当包含以上函数的代码执行后，会在log_path指定的文件中记录执行文件的路径，行数，参数，时间等，以判断是否为可疑文件。
	2.当利用eval,正则/e修饰符混淆加密的文件执行时，会记录所执行的真实源码。
感谢：
===
	1.参考借鉴了鸟哥taint https://github.com/laruence/taint
