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
#include <time.h>

ZEND_DECLARE_MODULE_GLOBALS(msafe)


/* True global resources - no need for thread safety here */
static int le_msafe;

PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("msafe.enable_msafe", "0", PHP_INI_ALL, OnUpdateBool, msafe_enabled, zend_msafe_globals, msafe_globals)
	STD_PHP_INI_BOOLEAN("msafe.disable_found", "0", PHP_INI_ALL, OnUpdateBool, msafe_disable_found, zend_msafe_globals, msafe_globals)
	STD_PHP_INI_ENTRY("msafe.log_path", "/tmp/check.log", PHP_INI_ALL, OnUpdateString, log_path, zend_msafe_globals, msafe_globals)
	STD_PHP_INI_ENTRY("msafe.disable_path", "", PHP_INI_SYSTEM, OnUpdateString, disable_path, zend_msafe_globals, msafe_globals)
PHP_INI_END()

static zend_op_array* (*old_compile_string)(zval *source_string, char *filename TSRMLS_DC);
static zend_op_array* m_compile_string(zval *source_string, char *filename TSRMLS_DC);

static zend_op_array* (*old_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);
static zend_op_array* my_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC);

zval *msafe_disable_path;

void web_log(const char *file_name, const char *log_string, const char *type, int lineno)
{
	FILE *fp;
	fp = fopen(MSAFE_G(log_path), "ab+");
	if (fp) {
		time_t rawtime;
	    struct tm *timeinfo;
	    char time_buf[20];
	    time(&rawtime);
	    timeinfo = localtime (&rawtime);
	    strftime(time_buf,sizeof(time_buf),"%Y-%m-%d %H:%M:%S",timeinfo);
		fprintf(fp, "[filename]: %s\n[%s]: %s\n[linenume]: %d\n[time]:%s\n\n", file_name, type, log_string, lineno, time_buf);
		fclose(fp);
	}
}

static zend_op_array *m_compile_string(zval *source_string, char *filename TSRMLS_DC)
{
	zend_op_array *op_array;
	
	op_array = old_compile_string(source_string, filename TSRMLS_CC);
	const char *file_name = zend_get_executed_filename(TSRMLS_C);

	if (op_array == NULL) {
		return op_array;
	}

	#if (PHP_MAJOR_VERSION == 7) 
		char *exec = op_array->filename->val;
	#else
		const char *exec = op_array->filename;
	#endif

	if(op_array != NULL && exec != NULL && strstr(exec, "eval()'d code")) {
		char function_name[8] = "eval";
		int lineno = zend_get_executed_lineno(TSRMLS_C);
		web_log(file_name, function_name, "function_name", lineno);
		web_log(file_name, Z_STRVAL_P(source_string), "source", lineno);
		if (MSAFE_G(msafe_disable_found)) {
			return NULL;
		}
	}
	return op_array;
}

static zend_op_array *my_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{
	zend_op_array *op_array;
	if (strlen(MSAFE_G(disable_path)) != 0) {
#if (PHP_MAJOR_VERSION == 5) 
		zval *function_name;
		zval *retval_ptr;
		zval *params[2];
		zval *separator, *str;

		MAKE_STD_ZVAL(function_name);
		MAKE_STD_ZVAL(retval_ptr);
		MAKE_STD_ZVAL(separator);
		MAKE_STD_ZVAL(str);
		ZVAL_STRING(separator, "," ,1);
		ZVAL_STRING(str, MSAFE_G(disable_path), 1);
		ZVAL_STRING(function_name, "explode" ,1);

		params[0] = separator;
		params[1] = str;

		int found = 0;

		if (call_user_function(EG(function_table), NULL, function_name, retval_ptr, 2, params TSRMLS_CC) == SUCCESS) {
			HashTable *ht;
			ht = Z_ARRVAL_P(retval_ptr);

			HashPosition pointer;
			zval **data;  
			for(zend_hash_internal_pointer_reset_ex(ht, &pointer);  
	        	zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) == SUCCESS;  
	        	zend_hash_move_forward_ex(ht, &pointer)) {  
				char *tmp = Z_STRVAL_PP(data);
				int len = Z_STRLEN_PP(data);
				if (file_handle->opened_path && strncmp(tmp, file_handle->opened_path, len) == 0) {
					found = 1;
					break;
				}
	    	} 
		}
		
		zval_ptr_dtor(&function_name); 
		zval_ptr_dtor(&retval_ptr); 
		zval_ptr_dtor(&separator); 
		zval_ptr_dtor(&str); 

		if (found == 1) {
			php_error_docref(NULL, E_ERROR, "%s", "Can't execute");
			return NULL;
		}
#else
		zval function_name = {{0}};

		ZVAL_STRING(&function_name, "explode");

		zval separator, str;
		ZVAL_STRING(&separator, ",");
		ZVAL_STRING(&str, MSAFE_G(disable_path));

		zval params[2];
		params[0] = separator;
		params[1] = str;

		zval retval_ptr;
		
		int found = 0;

		if (call_user_function(EG(function_table), NULL, &function_name, &retval_ptr, 2, params TSRMLS_CC) == SUCCESS) {
			zval *value;
			zend_string *key;

			ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(&retval_ptr), key, value) {
				char *tmp = Z_STRVAL_P(value);
				int len = Z_STRLEN_P(value);
				if (file_handle->opened_path && strncmp(tmp, file_handle->opened_path->val, len) == 0) {
					found = 1;
					break;
				}
			} ZEND_HASH_FOREACH_END();
		}

		zval_ptr_dtor(&function_name); 
		zval_ptr_dtor(&retval_ptr); 
		zval_ptr_dtor(&separator); 
		zval_ptr_dtor(&str); 
		if (found == 1) {
			php_error_docref(NULL, E_ERROR, "%s", "Can't execute");
			return NULL;
		}
#endif
    }
	op_array = old_compile_file(file_handle, type TSRMLS_CC);
	return op_array;
}

#if (PHP_MAJOR_VERSION == 7) 
	static int msafe_fcall_handler(zend_execute_data *execute_data) {
		zend_execute_data *call = execute_data->call;
		zend_function *fbc = call->func;
		zend_string *fname_str = fbc->common.function_name;

		int arg_count = ZEND_CALL_NUM_ARGS(call);

		char *fname = fname_str->val;
		int len = sizeof(fname);
#else 
	static int msafe_fcall_handler(ZEND_OPCODE_HANDLER_ARGS) {
		const zend_op *opline = execute_data->opline;
		zval *fname_zval = MSAFE_OP1_CONSTANT_PTR(opline);
		int arg_count = opline->extended_value;

		#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 3)
			void **p = EG(argument_stack)->top;
		#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
			void **p = EG(argument_stack)->top;
		#else
			void **p = EG(argument_stack).top_element;
		#endif

		char *fname = Z_STRVAL_P(fname_zval);
		int len = Z_STRLEN_P(fname_zval);

		#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 3)
			zend_function *fbc = EG(function_state_ptr)->function;
		#else
			zend_function *fbc = EG(current_execute_data)->function_state.function;
		#endif
#endif

	if (fbc->common.scope == NULL) {
		if (strncmp("system", fname, len) == 0
			|| strncmp("passthru", fname, len) == 0
			|| strncmp("exec", fname, len) == 0
			|| strncmp("shell_exec", fname, len) == 0
			|| strncmp("proc_open", fname, len) == 0 
			|| strncmp("popen", fname, len) == 0 ) {

			int lineno = zend_get_executed_lineno(TSRMLS_C);
			const char *file_name = zend_get_executed_filename(TSRMLS_C);
			web_log(file_name, fname, "function_name", lineno);

			if (arg_count) {
				#if (PHP_MAJOR_VERSION == 7) 
					zval *cmd = ZEND_CALL_ARG(call, arg_count);
				#else	
					zval *cmd;
					cmd = *((zval **) (p - arg_count));
				#endif
				if (IS_STRING == Z_TYPE_P(cmd)) {
					web_log(file_name, Z_STRVAL_P(cmd), "argv", lineno);
				}
			}
			if (MSAFE_G(msafe_disable_found)) {
				return ZEND_USER_OPCODE_DISPATCH_TO;
			}
		}
	}

	return ZEND_USER_OPCODE_DISPATCH;
} 

static void msafe_register_handlers() {
	zend_set_user_opcode_handler(ZEND_DO_FCALL, msafe_fcall_handler);
	#if (PHP_MAJOR_VERSION == 7)
	zend_set_user_opcode_handler(ZEND_DO_ICALL, msafe_fcall_handler);
	zend_set_user_opcode_handler(ZEND_DO_FCALL_BY_NAME, msafe_fcall_handler);
	#endif
} 

static void m_hook_execute()
{ 			
	old_compile_string = zend_compile_string;
	zend_compile_string = m_compile_string;

	old_compile_file = zend_compile_file; 
    zend_compile_file = my_compile_file;
}

static void m_unhook_execute()
{	
	zend_compile_string = old_compile_string; 
	zend_compile_file = old_compile_file;
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
	"0.2", /* Replace with version number for your extension */
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
		msafe_register_handlers(TSRMLS_C);
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

	DISPLAY_INI_ENTRIES();
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
