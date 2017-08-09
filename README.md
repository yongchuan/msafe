msafe
=====

通过劫持Zend底层编译opcode，可以分析php执行的代码，从而达到还原部分混淆加密的php源码，检测网站木马加入日志，禁用危险方法，禁用特定目录PHP文件的执行。

Update: (2017-08-05)
===
	1.增加PHP7支持
	2.增加system()，passthru(),exec(),shell_exec(),proc_open(),popen()支持
	3.增加msafe.log_path自定义检测到危险函数时日志存储路径
	4.增加msafe.disable_found 设置当检测到上述方法或者eval时是否执行
	5.增加msafe.disable_path 设置禁止执行php的目录，如 upload 等

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
	msafe.disable_found=1 禁止执行检测到的方法和eval()
	msafe.disable_path=/aaa,/bbb
使用
===
	1.当包含以上函数的代码执行后，会在log_path指定的文件中记录执行文件的路径，行数，参数，时间等，以判断是否为可疑木马文件。
	2.当利用eval,正则/e修饰符混淆加密的文件执行时，会记录所执行的真实源码。
	3.设置禁用危险函数后，上述函数不会执行。
	4.设置禁止执行PHP的目录后，该目录下的php文件不会执行。
感谢：
===
	1.参考借鉴了鸟哥taint https://github.com/laruence/taint
