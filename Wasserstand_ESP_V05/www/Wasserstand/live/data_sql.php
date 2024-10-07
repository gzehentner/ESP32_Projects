<!DOCTYPE html>
<html>
<body>


<?php

echo "<br>======================================" . PHP_EOL;
echo "<br>Data from ESP" . PHP_EOL;
echo "<br>====================================== <br>" . PHP_EOL;

// select table: live-system or development
$table = "WasserstandAllLive";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Accessing form data
    $board     = $_POST['board'];
    $levelAct  = $_POST['levelAct'];
    $debug     = $_POST['debug_level_switches'];
    $AHH       = $_POST['AHH'];
    $AH        = $_POST['AH'];
    $AL        = $_POST['AL'];
    $pump1_op  = $_POST['pump1_op'];
    $pump2_op  = $_POST['pump2_op'];
    $epochtime = $_POST['epochTime'];


    // =================================================================
    echo "<br> write data to database" . PHP_EOL;
    // =================================================================

    $link = mysqli_connect("localhost", "zehentner", "ZdgzqyAwnJ4M9aHH", "zehentner_wasserstand");

    if (!$link) {
        echo "Fehler: es konnte keine Verbindung zur Datenbank aufgebaut werden" . PHP_EOL;
        echo "Debug-Fehlernummer: " . mysqli_connect_errno() . PHP_EOL;
        echo "Debug-Fehlermeldung: " . mysqli_connect_error() . PHP_EOL;
        exit;

    } else {
        echo "Datenbank-Verbindung aufgebaut";
    }       

    echo "\n<br>Host-Informationen: " . mysqli_get_host_info($link) . PHP_EOL;
    
    $sql = "INSERT INTO $table (myIndex, Wasserstand, Relais_AHH, Relais_AH, Relais_AL, Pumpe1, Pumpe2, DatumUhrzeit, epochtime) VALUES (NULL, $levelAct, $AHH, $AH, $AL, $pump1_op, $pump2_op, current_timestamp(), $epochtime)";
    if ($link->query($sql) === TRUE) {
        echo "New record created successfully" . PHP_EOL;
        } else {
        echo "Error: " . $sql . PHP_EOL . $link->error;
        }

    mysqli_close($link);
    
    
    
    // =================================================================
    echo "<br> done" . PHP_EOL;
    // =================================================================
};

?>

</body>
</html>
