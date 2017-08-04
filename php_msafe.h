/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifndef PHP_MSAFE_H
#define PHP_MSAFE_H

extern zend_module_entry msafe_module_entry;
#define phpext_msafe_ptr &msafe_module_entry

#ifdef PHP_WIN32
#	define PHP_MSAFE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_MSAFE_API __attribute__ ((visibility("default")))
#else
#	define PHP_MSAFE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4) 
#  define MSAFE_OP1_CONSTANT_PTR(n) (&(n)->op1.u.constant)
#else
#  define MSAFE_OP1_CONSTANT_PTR(n) ((n)->op1.zv)
#endif

PHP_MINIT_FUNCTION(msafe);
PHP_MSHUTDOWN_FUNCTION(msafe);
PHP_RINIT_FUNCTION(msafe);
PHP_RSHUTDOWN_FUNCTION(msafe);
PHP_MINFO_FUNCTION(msafe);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
**/
ZEND_BEGIN_MODULE_GLOBALS(msafe)
	zend_bool  msafe_enabled;
  char *log_path;
  zend_bool  msafe_disable_found;
ZEND_END_MODULE_GLOBALS(msafe)


/* In every utility function you add that needs to use variables 
   in php_msafe_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as MSAFE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define MSAFE_G(v) TSRMG(msafe_globals_id, zend_msafe_globals *, v)
#else
#define MSAFE_G(v) (msafe_globals.v)
#endif

#endif	/* PHP_MSAFE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
