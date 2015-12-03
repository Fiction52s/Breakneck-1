<html>
<head>
<title>My first PHP Page</title>
</head>
<body>

<?php 

$target_file = "map online.brknk"
// Check if file already exists
if (file_exists($target_file)) {
    echo "Sorry, file already exists.";
} 
?>

</body>
</html>