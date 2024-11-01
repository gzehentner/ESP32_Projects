<!DOCTYPE html>
<html lang='en'>
    <head><title>Wasserstand-Messung</title>
        <meta http-equiv="content-type" content="text/html; charset=utf-8">
        <meta name="viewport" content="width=device-width">
        <link rel='stylesheet' type='text/css' href='../f.css'>
        <script src='j.js'> </script>
    </head>
    <header><h1>Wasserstand - Board Develop</h1>

        <nav> 
            <a href="/">[Home]</a> 
            <a href="list_level.php">[Value History]</a>
            <a href="handleGraph.php">[Wasserstand Diagramm]</a>  
        </nav>
    </header>
<?php

// select table: live-system or development
$table = "WasserstandAllLive";

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
                     <footer><p>Actual Date and Time:  </p>

<p id="datetime"></p>

<script>
    function updateDateTime() {
        var now = new Date();
        var date = now.toLocaleDateString();
        var time = now.toLocaleTimeString();
        document.getElementById('datetime').innerHTML = date + " " + time;
    }

    setInterval(updateDateTime, 1000); // Update every second
    window.onload = updateDateTime; // Initial call to display immediately
</script>
</p>
</footer>
</body>
</html>
