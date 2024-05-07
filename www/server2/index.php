<?php

echo "<h1>". getenv("CONTENT_TYPE") . "</h1>";

echo getenv("SCRIPT_FILENAME") . "\n";
$_SESSION["pass"] = "password";
echo "<h1>" . $_SESSION["pass"] . "</h1>";
?>