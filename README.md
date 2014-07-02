Facial
======

An API to OpenCV used to detect faces in images, more generally a bridge to OpenCV objdetect.

```php
<?php
/**
* This is the entire API at present, more will be exposed as it becomes necessary
**/
namespace DatingVIP\Facial {
	class Image {
		public function __construct(string file);
		public function getWidth();
		public function getHeight();
	}
	
	class Cascade {
		public function __construct(string file);
	}
	
	class Detector {
		public function __construct(Cascade cascade [, array min = [0, 0]]);
		public function detect(Image image [, array max]);
	}
}

namespace {
	use DatingVIP\Facial\Image;
	use DatingVIP\Facial\Cascade;
	use DatingVIP\Facial\Detector;
	
	$cascade = new Cascade("cascades/default.xml");

	$detector = new Detector($cascade);

	$image = new Image("test.jpg");	

	var_dump($detector->detect($image));
}
?>
