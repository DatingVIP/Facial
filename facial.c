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

zend_object_handlers php_facial_detector_handlers;
zend_object_handlers php_facial_cascade_handlers;
zend_object_handlers php_facial_image_handlers;

zend_class_entry *Cascade_ce;
zend_class_entry *Facial_ce;
zend_class_entry *Image_ce;

/* {{{ */
typedef struct _php_facial_cascade_t {
	zend_object              std;
	zend_object_handle       h;
	CvHaarClassifierCascade  *c;	
} php_facial_cascade_t; /* }}} */

/* {{{ */
typedef struct _php_facial_detector_t {
	zend_object              std;
	zend_object_handle       h;
	php_facial_cascade_t    *cascade;
	CvSize                   min;
} php_facial_detector_t; /* }}} */

/* {{{ */
typedef struct _php_facial_image_t {
	zend_object              std;
	zend_object_handle       h;
	IplImage                *img;	
} php_facial_image_t; /* }}} */

/* {{{ */
static inline void php_facial_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	zend_objects_destroy_object(zobject, handle TSRMLS_CC);
} /* }}} */

/* {{{ */
static inline void php_facial_detector_free(void *zobject TSRMLS_DC) {
	php_facial_detector_t *pfacial =
		(php_facial_detector_t *) zobject;
	zend_object_std_dtor(&pfacial->std TSRMLS_CC);
	zend_objects_store_del_ref_by_handle(pfacial->cascade->h TSRMLS_CC);
	efree(pfacial);
} /* }}} */

/* {{{ */
static inline zend_object_value php_facial_detector_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;

	php_facial_detector_t *pfacial =
		(php_facial_detector_t*) ecalloc(1, sizeof(php_facial_detector_t));

	zend_object_std_init(&pfacial->std, ce TSRMLS_CC);
	object_properties_init(&pfacial->std, ce);

	pfacial->h = zend_objects_store_put(
		pfacial,
		php_facial_destroy,
		php_facial_detector_free, NULL TSRMLS_CC);

	value.handle = pfacial->h;
	value.handlers = &php_facial_detector_handlers;

	return value;
} /* }}} */

/* {{{ */
static inline void php_facial_cascade_free(void *zobject TSRMLS_DC) {
	php_facial_cascade_t *pcascade =
		(php_facial_cascade_t *) zobject;
	zend_object_std_dtor(&pcascade->std TSRMLS_CC);
	cvRelease((void**)&pcascade->c);
	efree(pcascade);
} /* }}} */

/* {{{ */
static inline zend_object_value php_facial_cascade_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;

	php_facial_cascade_t *pcascade =
		(php_facial_cascade_t*) ecalloc(1, sizeof(php_facial_detector_t));

	zend_object_std_init(&pcascade->std, ce TSRMLS_CC);
	object_properties_init(&pcascade->std, ce);

	pcascade->h = zend_objects_store_put(
		pcascade,
		php_facial_destroy,
		php_facial_cascade_free, NULL TSRMLS_CC);

	value.handle = pcascade->h;
	value.handlers = &php_facial_cascade_handlers;

	return value;
} /* }}} */

/* {{{ */
static inline void php_facial_image_free(void *zobject TSRMLS_DC) {
	php_facial_image_t *pimage =
		(php_facial_image_t *) zobject;
	zend_object_std_dtor(&pimage->std TSRMLS_CC);
	cvReleaseImage(&pimage->img);
	efree(pimage);
} /* }}} */

/* {{{ */
static inline zend_object_value php_facial_image_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;

	php_facial_image_t *pimage =
		(php_facial_image_t*) ecalloc(1, sizeof(php_facial_image_t));

	zend_object_std_init(&pimage->std, ce TSRMLS_CC);
	object_properties_init(&pimage->std, ce);

	pimage->h = zend_objects_store_put(
		pimage,
		php_facial_destroy,
		php_facial_image_free, NULL TSRMLS_CC);

	value.handle = pimage->h;
	value.handlers = &php_facial_image_handlers;

	return value;
} /* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(facial)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Facial support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ proto Cascade::__construct(string file) */
PHP_METHOD(Cascade, __construct)  {
	zval *zcascade = NULL;
	php_facial_cascade_t *pcascade = zend_object_store_get_object(getThis() TSRMLS_CC);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zcascade) != SUCCESS) {
		return;
	}
	
	if (!zcascade || Z_TYPE_P(zcascade) != IS_STRING) {
		return;
	}

	pcascade->c = (CvHaarClassifierCascade*) cvLoad(Z_STRVAL_P(zcascade), 0, 0, 0);
} /* }}} */

/* {{{ proto Detector::__construct(Cascade cascade [, array min = [0, 0]])
	Construct a facial object using specified cascade, able to find objects with minimum dimensions of min[0] * min[1] */
PHP_METHOD(Detector,  __construct)  {
	zval *zcascade = NULL,
	     *zsize = NULL;
	php_facial_detector_t *pfacial = zend_object_store_get_object(getThis() TSRMLS_CC);
		
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|z", &zcascade, Cascade_ce, &zsize) != SUCCESS) {
		return;
	}

	pfacial->cascade = (php_facial_cascade_t*) zend_object_store_get_object(zcascade TSRMLS_CC);

	zend_objects_store_add_ref_by_handle(pfacial->cascade->h TSRMLS_CC);
	
	if (zsize) {
		switch (Z_TYPE_P(zsize)) {
			case IS_LONG: pfacial->min = cvSize(Z_LVAL_P(zsize), Z_LVAL_P(zsize)); break;

			case IS_ARRAY: if (zend_hash_num_elements(Z_ARRVAL_P(zsize)) >= 2) {
				zval **min[2];

				zend_hash_index_find(Z_ARRVAL_P(zsize), 0, (void**)&min[0]);
				zend_hash_index_find(Z_ARRVAL_P(zsize), 1, (void**)&min[1]);
				
				pfacial->min = cvSize
					(Z_LVAL_PP(min[0]), Z_LVAL_PP(min[1]));					
				break;
			}

			default: {
				pfacial->min = cvSize(0, 0);
			}
		}
	} else pfacial->min = cvSize(0, 0);
} /* }}} */

/* {{{ proto array Detector::detect(Image image [, array max])
   return coordinates of faces found in file with maximum dimensions of max[0] * max[1]
   if max is not provided, image dimensions are used */
PHP_METHOD(Detector, detect)
{	
	zval *zimage = NULL,
	     *zsize = NULL,
	     **zsizes[2] = {NULL, NULL};
	php_facial_detector_t *pdetector = zend_object_store_get_object(getThis() TSRMLS_CC);
	php_facial_image_t    *pimage = NULL;
	CvMemStorage *storage;
	CvSeq *faces;
	CvSize max;
	int face;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|a", &zimage, Image_ce, &zsize) == FAILURE) {
		return;
	}
	
	pimage = zend_object_store_get_object(zimage TSRMLS_CC);	
		
	storage = cvCreateMemStorage(0);

	if (!storage) {
		return;
	}

	if (zsize && Z_TYPE_P(zsize) != IS_NULL) {
		if (zend_hash_index_find(Z_ARRVAL_P(zsize), 0, (void**)&zsizes[0]) != SUCCESS ||
		    zend_hash_index_find(Z_ARRVAL_P(zsize), 1, (void**)&zsizes[1]) != SUCCESS) {
		    return;
		}

		if ((!zsizes[0] || Z_TYPE_PP(zsizes[0]) != IS_LONG) ||
		    (!zsizes[1] || Z_TYPE_PP(zsizes[1]) != IS_LONG)) {
		    return;
		}

		max = cvSize(Z_LVAL_PP(zsizes[0]), Z_LVAL_PP(zsizes[1]));
	} else max = cvSize(pimage->img->width, pimage->img->height);

	faces = cvHaarDetectObjects(
		pimage->img,
		pdetector->cascade->c, 
		storage, 
		1.1, 2, 0, 
		pdetector->min, max);

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
	
	cvRelease((void**)&faces);
	cvReleaseMemStorage(&storage);
}
/* }}} */

/* {{{ */
PHP_METHOD(Image, __construct) {
	zval *zimage = NULL;
	php_facial_image_t *pimage = zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zimage) != SUCCESS) {
		return;
	}	
	
	if (!zimage || Z_TYPE_P(zimage) != IS_STRING) {
		return;	
	}
	
	pimage->img = cvLoadImage(Z_STRVAL_P(zimage), -1);
} /* }}} */

/* {{{ proto int Image::getWidth(void) */
PHP_METHOD(Image, getWidth) {
	php_facial_image_t *pimage = zend_object_store_get_object(getThis() TSRMLS_CC);
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;	
	}
	
	RETURN_LONG(pimage->img->width);
} /* }}} */

/* {{{ proto int Image::getHeight(void) */
PHP_METHOD(Image, getHeight) {
	php_facial_image_t *pimage = zend_object_store_get_object(getThis() TSRMLS_CC);
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;	
	}
	
	RETURN_LONG(pimage->img->height);
} /* }}} */

/* {{{ */
zend_function_entry php_facial_image_methods[] = {
	PHP_ME(Image, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Image, getWidth,    NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Image, getHeight,   NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

/* {{{ */
zend_function_entry php_facial_cascade_methods[] = {
	PHP_ME(Cascade, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

/* {{{ */
zend_function_entry php_facial_detector_methods[] = {
	PHP_ME(Detector, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Detector, detect,      NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

/* {{{ */
PHP_MINIT_FUNCTION(facial) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "DatingVIP\\Facial", "Detector", php_facial_detector_methods);	
	Facial_ce = zend_register_internal_class(&ce TSRMLS_CC);
	Facial_ce->create_object = php_facial_detector_create;

	INIT_NS_CLASS_ENTRY(ce, "DatingVIP\\Facial", "Cascade", php_facial_cascade_methods);
	Cascade_ce = zend_register_internal_class(&ce TSRMLS_CC);
	Cascade_ce->create_object = php_facial_cascade_create;

	INIT_NS_CLASS_ENTRY(ce, "DatingVIP\\Facial", "Image", php_facial_image_methods);
	Image_ce = zend_register_internal_class(&ce TSRMLS_CC);
	Image_ce->create_object = php_facial_image_create;

	memcpy(
		&php_facial_detector_handlers,
		zend_get_std_object_handlers(),
		sizeof(php_facial_detector_handlers));
	memcpy(
		&php_facial_cascade_handlers,
		zend_get_std_object_handlers(),
		sizeof(php_facial_cascade_handlers));
	memcpy(
		&php_facial_image_handlers,
		zend_get_std_object_handlers(),
		sizeof(php_facial_image_handlers));				

	return SUCCESS;
} /* }}} */

/* {{{ facial_module_entry
 */
zend_module_entry facial_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_FACIAL_EXTNAME,
	NULL,
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
