<?php
/**
* Scan a directory for images creating test images ...
**/

/** usage: facial.php [path=images] [cascade=cascades/default.xml] **/

use DatingVIP\Facial\Cascade;
use DatingVIP\Facial\Detector;
use DatingVIP\Facial\Image;

$path    = realpath(isset($argv[1]) ? $argv[1] : "images");
$cascade = new Cascade(isset($argv[2])  ? $argv[2] : "cascades/default.xml");
$draw    = (bool) @$argv[3];

var_dump($cascade);

function find($path) {
	$files = [];
	foreach (scandir($path) as $file) {
		if ($file == "." || $file == "..") {
			continue;
		}
		
		$next = "{$path}/{$file}";
		if (is_dir($next)) {
			$files = array_merge
				($files, find($next));
		} else $files[] = $next;
	}
	return $files;
}

foreach (find(realpath($path)) as $image) {
	if (preg_match("~/face-~", $image)) {
		continue;	
	}
	
	$base    = basename($image);

	$img     = new Image($image);
	$detector  = new Detector($cascade);
	$faces   = $detector->detect($img);	

	if (!$faces) {
		$fail++;
		printf("[F]: no faces found in %s\n", $image);
		continue;
	} else $pass++;

	printf("[P]: %d faces found in %s\n", count($faces), $image);

	if (!$draw) {
		continue;
	}
	
	$copy    = imagecreatefromjpeg($image);
	$green   = imagecolorallocate($copy, 132, 135, 28);

	foreach ($faces as $face) {
		imagerectangle($copy, 
			$face["x"],
			$face["y"],
			$face["x"] + $face["width"],
			$face["y"] + $face["height"],
			$green);
	}

	imagejpeg($copy, "{$path}/face-{$base}");
}

printf(
	"%d failures, %d detections, %.3f fails\n",
	$fail, $pass, (($fail + $pass) / 100) * $fail);
?>
