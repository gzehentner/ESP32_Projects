<?php

// select table: live-system or development
$table = WasserstandAllDevelop;

// Create connection
$link = mysqli_connect("localhost", "zehentner", "ZdgzqyAwnJ4M9aHH", "zehentner_wasserstand");

if (!$link) {
    echo "Fehler: es konnte keine Verbindung zur Datenbank aufgebaut werden" . PHP_EOL;
    echo "Debug-Fehlernummer: " . mysqli_connect_errno() . PHP_EOL;
    echo "Debug-Fehlermeldung: " . mysqli_connect_error() . PHP_EOL;
    exit;
} else {
    echo "Datenbank-Verbindung aufgebaut<br>";
}


$sql = "SELECT myIndex, Wasserstand, DatumUhrzeit FROM $table";
$result = $link->query($sql);

if ($result->num_rows > 0) {
    // output data of each row
    while($row = $result->fetch_assoc()) {
    echo "id: " . $row["myIndex"]. " Zeit: " . $row["DatumUhrzeit"] . " - Stand: " . $row["Wasserstand"]. "<br>";
    }
} else {
    echo "0 results";
}

mysqli_close($link);
?>