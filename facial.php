<?php
/**
* Scan a directory for images creating test images ...
**/

/** usage: facial.php [path=images] [cascade=cascades/default.xml] **/

$path    = @realpath($argv[1] ?: "images");
$cascade = @cascade($argv[2]  ?: "cascades/default.xml");
$draw    = (bool) @$argv[3];

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
	$size    = getimagesize($image);
	
	$faces   = faces($cascade, $image, $size[0], $size[1]);

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
