msafe
=====

通过劫持Zend底层编译opcode，可以分析php执行的代码，从而达到还原一切混淆加密的php源码，并且可以根据自定义规则，审计代码安全。

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
