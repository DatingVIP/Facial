/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
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

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_facial.h"

#include <opencv/cv.h>

/* {{{ proto array faces(string cascade, string file, int width, int height)
   return coordinates of faces found in file */
PHP_FUNCTION(faces)
{
	zval *zfile = NULL,
	     *zcascade = NULL;
	long zwidth = 0L,
	     zheight = 0L;
	CvHaarClassifierCascade *cascade;
	CvMemStorage *storage;
	IplImage *img;
	CvSeq *faces;
	int face;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzll", &zcascade, &zfile, &zwidth, &zheight) == FAILURE) {
		return;
	}
	
	if (!zfile || Z_TYPE_P(zfile) != IS_STRING) {
		return;
	}

	if (!zcascade || Z_TYPE_P(zcascade) != IS_STRING) {
		return;
	}

	cascade = (CvHaarClassifierCascade*) cvLoad(Z_STRVAL_P(zcascade), 0, 0, 0 );
	
	if (!cascade) {
		return;
	}

	img = cvLoadImage(Z_STRVAL_P(zfile), -1);

	if (!img) {
		return;
	}
	
	storage = cvCreateMemStorage(0);

	if (!storage) {
		return;
	}
	
	faces = cvHaarDetectObjects(img, cascade, storage, 1.1, 3, 0, cvSize(100,100), cvSize(zwidth, zheight));

	if (!faces->total) {
		return;
	}

	array_init(return_value);

	for (face = 0; face < faces->total; face++) {
		CvRect *rect = (CvRect*) cvGetSeqElem(faces, face);
		zval *child;

		MAKE_STD_ZVAL(child);
		array_init(child);
		
		add_assoc_long(child, "x", rect->x);
		add_assoc_long(child, "y", rect->y);
		add_assoc_long(child, "width", rect->width);
		add_assoc_long(child, "height", rect->height);

		add_next_index_zval(return_value, child);
	}
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(facial)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(facial)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(facial)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(facial)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(facial)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "facial support", "enabled");
	php_info_print_table_end();
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(php_arginfo_faces, 0, 0, 1)
	ZEND_ARG_INFO(0, image)
	
ZEND_END_ARG_INFO()

/* {{{ facial_functions[]
 *
 * Every user visible function must have an entry in facial_functions[].
 */
const zend_function_entry facial_functions[] = {
	PHP_FE(faces,	php_arginfo_faces)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in facial_functions[] */
};
/* }}} */

/* {{{ facial_module_entry
 */
zend_module_entry facial_module_entry = {
	STANDARD_MODULE_HEADER,
	"facial",
	facial_functions,
	PHP_MINIT(facial),
	PHP_MSHUTDOWN(facial),
	PHP_RINIT(facial),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(facial),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(facial),
	PHP_FACIAL_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FACIAL
ZEND_GET_MODULE(facial)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
