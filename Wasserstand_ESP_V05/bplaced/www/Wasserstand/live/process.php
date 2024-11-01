<?php
// Get the number of days from the form
$days = intval($_POST['days']);

// Calculate the start and end times
$endTime = time();
$startTime = $endTime - ($days * 24 * 60 * 60);

// Query the database
$conn = new mysqli('hostname', 'username', 'password', 'database');
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT * FROM `WasserstandAllLive` WHERE epochtime BETWEEN $startTime AND $endTime";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    // Output data of each row
    while($row = $result->fetch_assoc()) {
        echo "id: " . $row["id"]. " - Data: " . $row["data"]. "<br>";
    }
} else {
    echo "0 results";
}
$conn->close();
?>
