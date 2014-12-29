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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_msafe.h"

ZEND_DECLARE_MODULE_GLOBALS(msafe)


/* True global resources - no need for thread safety here */
static int le_msafe;

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("msafe.enable_msafe", "1", PHP_INI_ALL, OnUpdateLongGEZero, msafe_enabled, zend_msafe_globals, msafe_globals)
PHP_INI_END();

static zend_op_array* (*old_compile_string)(zval *source_string, char *filename TSRMLS_DC);
static zend_op_array* m_compile_string(zval *source_string, char *filename TSRMLS_DC);

char logfile[32] = "/tmp/check.log";

void web_log(char *file_name, char *function_name, int lineno)
{
        FILE *fh;
        fh = fopen(logfile, "ab+");
        fprintf(fh, "[filename]: %s\n[function]: %s\n[linenume]: %d\n\n", file_name, function_name, lineno);
        fclose(fh);
}

static zend_op_array *m_compile_string(zval *source_string, char *filename TSRMLS_DC)
{
	char *exec_string;
	zend_op_array *op_array;
	
	op_array = old_compile_string(source_string, filename TSRMLS_CC);

	//eval string, 这里可以增加对source_string其他的过滤逻辑
	if(op_array != NULL && strstr(op_array->filename, "eval()'d code")) {
		convert_to_string(source_string);
		exec_string = estrndup(Z_STRVAL_P(source_string), Z_STRLEN_P(source_string));
		char *file_name = zend_get_executed_filename(TSRMLS_C);
		char function_name[8] = "eval";
		int lineno = zend_get_executed_lineno(TSRMLS_C);
		web_log(file_name, function_name, lineno);
	}
	return op_array;
}

int m_hook_execute()
{ 			
	old_compile_string = zend_compile_string;
	zend_compile_string = m_compile_string;
	return 1;
}

int m_unhook_execute()
{	
	zend_compile_string = old_compile_string;    
	return 1;
}
/* {{{ msafe_functions[]
 *
 * Every user visible function must have an entry in msafe_functions[].
 */
const zend_function_entry msafe_functions[] = {
	{NULL, NULL, NULL}	/* Must be the last line in msafe_functions[] */
};
/* }}} */

/* {{{ msafe_module_entry
 */
zend_module_entry msafe_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"msafe",
	msafe_functions,
	PHP_MINIT(msafe),
	PHP_MSHUTDOWN(msafe),
	PHP_RINIT(msafe),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(msafe),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(msafe),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MSAFE
ZEND_GET_MODULE(msafe)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("msafe.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_msafe_globals, msafe_globals)
    STD_PHP_INI_ENTRY("msafe.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_msafe_globals, msafe_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_msafe_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_msafe_init_globals(zend_msafe_globals *msafe_globals)
{
	msafe_globals->global_value = 0;
	msafe_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(msafe)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(msafe)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(msafe)
{
	if (MSAFE_G(msafe_enabled) == 1) {
		m_hook_execute();
	}
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(msafe)
{
	if (MSAFE_G(msafe_enabled) == 1) {
		m_unhook_execute();
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(msafe)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "msafe support", "enabled");
	php_info_print_table_header(2, "Author", "charles(charles.m1256@gmail.com)");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
