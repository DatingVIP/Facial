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
  | Author: krakjoe                                                      |
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

static int le_cascade;

/* {{{ proto resource cascade(string filename) */
PHP_FUNCTION(cascade) 
{
	zval *zcascade = NULL;
	CvHaarClassifierCascade *cascade;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zcascade) != SUCCESS) {
		return;
	}

	if (!zcascade || Z_TYPE_P(zcascade) != IS_STRING) {
		return;
	}

	cascade = (CvHaarClassifierCascade*) cvLoad(Z_STRVAL_P(zcascade), 0, 0, 0);

	if (!cascade) {
		return;
	}
	
	ZEND_REGISTER_RESOURCE(return_value, cascade, le_cascade);	
} /* }}} */

/* {{{ proto array faces(resource cascade, string file, int width, int height)
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzll", &zcascade, &zfile, &zwidth, &zheight) == FAILURE) {
		return;
	}
	
	if (!zfile || Z_TYPE_P(zfile) != IS_STRING) {
		return;
	}

	if (!zcascade || Z_TYPE_P(zcascade) != IS_RESOURCE) {
		return;
	}

	ZEND_FETCH_RESOURCE(cascade, CvHaarClassifierCascade*, &zcascade, -1, "cascade", le_cascade);
	
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
	
	faces = cvHaarDetectObjects(img, cascade, storage, 1.1, 2, 0, cvSize(60,60), cvSize(zwidth, zheight));

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
	
	cvReleaseImage(&img);
	cvRelease(&faces);
	cvReleaseMemStorage(&storage);
}
/* }}} */

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

ZEND_BEGIN_ARG_INFO_EX(php_arginfo_cascade, 0, 0, 1)
	ZEND_ARG_INFO(0, cascade)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_arginfo_faces, 0, 0, 4)
	ZEND_ARG_INFO(0, cascade)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

/* {{{ facial_functions[]
 */
const zend_function_entry facial_functions[] = {
	PHP_FE(cascade,	php_arginfo_cascade)
	PHP_FE(faces,	php_arginfo_faces)
	PHP_FE_END
};
/* }}} */

/* {{{ */
static void php_facial_cascade_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
    cvRelease(&rsrc->ptr);
} /* }}} */

/* {{{ */
PHP_MINIT_FUNCTION(facial) {
	
	le_cascade = zend_register_list_destructors_ex(
		php_facial_cascade_dtor, NULL, "cascade", module_number);

	return SUCCESS;
} /* }}} */

/* {{{ facial_module_entry
 */
zend_module_entry facial_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_FACIAL_EXTNAME,
	facial_functions,
	PHP_MINIT(facial),
	NULL,
	NULL,
	NULL,
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
