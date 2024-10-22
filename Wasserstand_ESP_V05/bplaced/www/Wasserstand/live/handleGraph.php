<!DOCTYPE html>
<html lang='en'>
<!-- 
    =======================================================================================================
    Header Code including the navigation
    =======================================================================================================
    -->

<head>
    <title>Wasserstand-Messung</title>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width">
    <link rel='stylesheet' type='text/css' href='../f.css'>
    <script src='j.js'> </script>
</head>

<header>
    <h1>Wasserstand - Live System</h1>
    <nav>
        <a href="/">[Home]</a>
        <a href="list_level.php">[Value History]</a>
        <a href="handleGraph.php">[Wasserstand Diagramm]</a>
    </nav>
</header>

<?php
//=====================================================================================================
// connect data base
// ====================================================================================================
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
    // echo "Datenbank-Verbindung aufgebaut<br>";
}

//=====================================================================================================
// get latest element (=actual waterlevel)
$sql = "SELECT * FROM `WasserstandAllLive` order by epochtime DESC LIMIT 1";
$result = $link->query($sql);
$latestrow = $result->fetch_assoc();
$levelAct = $latestrow["Wasserstand"];           // we need the last value of water level

?>


<article>
    <h2>Wasserstand Zehentner Teisendorf</h2>
    <br>Wasserstand aktuell: <?php echo $levelAct ?>;
    <br><br>
<!-- 
    ========================================================
    - display form to enter number of days to be displayed 
    - and extract result
    ========================================================
    -->
    <form action="handleGraph.php" method="post">
        <label for="days">Enter number of days for graph:</label>
        <input type="number" id="days" name="days" min="1" value=2 required>
        <input type="submit" value="Submit">
    </form>
</article>

<?php
$days = intval($_POST['days']);
?>

<?php

//=====================================================================================================
// adjustable graph
// beginning
//================================================================
$end_time = time();
$start_time = (time() - (60 * 60 * 24 * $days));

$sql = "SELECT * FROM `WasserstandAllLive` WHERE epochtime 
                                                BETWEEN $start_time
                                                AND $end_time";


$result = $link->query($sql);
$myNumRows_two = $result->num_rows;

if ($result->num_rows > 0) {
    // output data of each row
    $firstrun = 1;
    while ($row = $result->fetch_assoc()) {
        if ($firstrun == 1) {
            $firstrun = 0;
            $xVal_two = "[ \"" . date('d.m - H:i', $row["epochtime"]) . "\"";
            $yVal_two = "[ " . $row["Wasserstand"];
        } else {
            $xVal_two = $xVal_two . ", \""  .  date('d.m - H:i', $row["epochtime"]) . "\"";
            $yVal_two = $yVal_two . ", " . $levelAct;
        }
    }
    $xVal_two = $xVal_two . "]";
    $yVal_two = $yVal_two . "]";
} else {
    echo "0 results";
}
//=====================================================================================================
// === END adjustable
//==========
//=====================================================================================================
mysqli_close($link);
//=====================================================================================================
// END: get data from db
// ====================================================================================================
?>

<body>

    <main>


        <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.js"></script>

        <body>
            <canvas id="Last two days" style="border:2px solid #000000; width:100%;max-width:700px">
            </canvas>


            <script>
                // get variables from PHP
                var xValues = <?php echo $xVal_two; ?>;
                var yValues = <?php echo $yVal_two; ?>;
                var myNumRows_h = <?php echo $myNumRows_two ?>;

                const graphYlevelWarn = [];
                generateWarnData("170", 0, myNumRows_h, 1);
                const graphYlevelErro = [];
                generateErroData("185", 0, myNumRows_h, 1);

                new Chart("Last two days", {
                    type: "line",
                    data: {
                        labels: xValues,
                        datasets: [{
                                data: yValues,
                                fill: false,
                                lineTension: 0,
                                pointRadius: 0,
                                backgroundColor: "rgba(0,0,255,1.0)",
                                borderColor: "rgba(0,0,255,0.5)",
                                label: "Wasserstand [cm]"
                            },
                            {
                                data: graphYlevelWarn,
                                fill: false,
                                lineTension: 0,
                                pointRadius: 0,
                                backgroundColor: "rgba(0,255,0,0)",
                                borderColor: "rgba(0,255,0,0.3)",
                                label: "Warnschwelle: 170 cm"
                            },
                            {
                                data: graphYlevelErro,
                                fill: false,
                                lineTension: 0,
                                pointRadius: 0,
                                backgroundColor: "rgba(255,0,0,0)",
                                borderColor: "rgba(255,0,0,0.3)",
                                label: "Alarmschwelle: 185 cm"
                            }
                        ]
                    },
                    options: {
                        title: {
                            display: true,
                            text: "Wasserstand - vergangene <?php echo $days ?> Tage"
                        },
                        legend: {
                            display: true,
                            text: "Wasserstand "
                        },
                        scales: {
                            yAxes: [{
                                ticks: {
                                    min: 100,
                                    max: 200
                                }
                            }]
                        },
                        interaction: {
                            mode: 'x'
                        }
                    }
                });

                function generateWarnData(value, i1, i2, step = 1) {
                    for (let x = i1; x <= i2; x += step) {
                        ;
                        graphYlevelWarn.push(eval(value));
                    }
                }

                function generateErroData(value, i1, i2, step = 1) {
                    for (let x = i1; x <= i2; x += step) {
                        ;
                        graphYlevelErro.push(eval(value));
                    }
                }
            </script>
        </body>
    </main>

    <footer>
        <p>Actual Date and Time: </p>

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