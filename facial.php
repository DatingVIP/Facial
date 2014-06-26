<?php
$cascade = "cascades/default.xml";
$image   = "images/token.jpg";
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

imagejpeg($copy, "images/token-face.jpg");
?>
