<?php
$image   = "images/sepia.jpg";
$size    = getimagesize($image);
$cascade = cascade("cascades/default.xml");

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

imagejpeg($copy, "images/sepia-face.jpg");
?>
