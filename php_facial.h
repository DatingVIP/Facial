/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: krakjoe                                                      |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_FACIAL_H
#define PHP_FACIAL_H

extern zend_module_entry facial_module_entry;
#define phpext_facial_ptr &facial_module_entry

#define PHP_FACIAL_VERSION "0.1.0"
#define PHP_FACIAL_EXTNAME "facial"
 
#ifdef PHP_WIN32
#	define PHP_FACIAL_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_FACIAL_API __attribute__ ((visibility("default")))
#else
#	define PHP_FACIAL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#endif	/* PHP_FACIAL_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
