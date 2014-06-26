<?php
/**
* Scan a directory for images creating test images ...
**/

/** usage: facial.php [path=images] [cascade=cascades/default.xml] **/

$path    = @realpath($argv[1] ?: "images");
$cascade = @cascade($argv[2] ?: "cascades/default.xml");

foreach (glob("{$path}/*.jpg") as $image) {
	if (preg_match("~/face-~", $image)) {
		continue;	
	}
	
	$base    = basename($image);
	$size    = getimagesize($image);
	$copy    = imagecreatefromjpeg($image);
	$green   = imagecolorallocate($copy, 132, 135, 28);

	foreach (faces($cascade, $image, $size[0], $size[1]) as $face) {
		imagerectangle($copy, 
			$face["x"],
			$face["y"],
			$face["x"] + $face["width"],
			$face["y"] + $face["height"],
			$green);
	}

	imagejpeg($copy, "{$path}/face-{$base}");
}
?>
