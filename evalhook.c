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
  | Author: Stefan Esser                                                 |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_compile.h"
#include "php_evalhook.h"

zend_module_entry evalhook_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"evalhook",
    NULL,
	PHP_MINIT(evalhook),
	PHP_MSHUTDOWN(evalhook),
	NULL,
	NULL,
	PHP_MINFO(evalhook),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_EVALHOOK
ZEND_GET_MODULE(evalhook)
#endif

static zend_op_array *(*orig_compile_string)(zval *source_string, char *filename TSRMLS_DC);
static zend_bool evalhook_hooked = 0;

static zend_op_array *evalhook_compile_string(zval *source_string, char *filename TSRMLS_DC)
{
	int c, len, yes;
	char *copy;
	
	/* Ignore non string eval() */
	if (Z_TYPE_P(source_string) != IS_STRING) {
		return orig_compile_string(source_string, filename TSRMLS_CC);
	}
	
	len  = Z_STRLEN_P(source_string);
	copy = estrndup(Z_STRVAL_P(source_string), len);
	if (len > strlen(copy)) {
		for (c=0; c<len; c++) if (copy[c] == 0) copy[c] == '?';
	}
	
	printf("Script tries to evaluate the following string.\n");
	printf("----\n");
	printf("%s\n", copy);
	printf("----\nDo you want to allow execution? [y/N]\n");
	
	yes = 0;
	while (1) {
		c = getchar();
		if (c == '\n') break;
		if (c == 'y' || c == 'Y') {
			yes = 1;
		}
    }

	if (yes) {
		return orig_compile_string(source_string, filename TSRMLS_CC);
	}
	
	zend_error(E_ERROR, "evalhook: script abort due to disallowed eval()");
}


PHP_MINIT_FUNCTION(evalhook)
{
	if (evalhook_hooked == 0) {
		evalhook_hooked = 1;
		orig_compile_string = zend_compile_string;
		zend_compile_string = evalhook_compile_string;
	}
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(evalhook)
{
	if (evalhook_hooked == 1) {
		evalhook_hooked = 0;
		zend_compile_string = orig_compile_string;
	}
	return SUCCESS;
}

PHP_MINFO_FUNCTION(evalhook)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "evalhook support", "enabled");
	php_info_print_table_end();
}
