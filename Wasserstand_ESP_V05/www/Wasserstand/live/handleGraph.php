
<!DOCTYPE html>
<html lang='en'>
    <head><title>Wasserstand-Messung</title>
        <meta http-equiv="content-type" content="text/html; charset=utf-8">
        <meta name="viewport" content="width=device-width">
        <link rel='stylesheet' type='text/css' href='/f.css'><script src='j.js'></script></head>

        <?php
                    //=====================================================================================================
                    // get data from db
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
                        echo "Datenbank-Verbindung aufgebaut<br>";
                    }


                    $sql = "SELECT myIndex, Wasserstand, DatumUhrzeit FROM $table";
                    $result = $link->query($sql);

                    if ($result->num_rows > 0) {
                        // output data of each row
                        $firstrun = 1;
                        while($row = $result->fetch_assoc()) {
                            if ($firstrun == 1) {
                                $firstrun = 0;
                                $xVal = "[ \"". $row["DatumUhrzeit"] . "\"";
                                $yVal = "[ ". $row["Wasserstand"] ;
                            } else {
                                $xVal = $xVal . ", \"" . $row["DatumUhrzeit"] . "\"";
                                $levelAct = $row["Wasserstand"];           // we need the last value of water level
                                $yVal = $yVal . ", " . $levelAct ;
                            }
                        }
                        $xVal = $xVal . "]";
                        $yVal = $yVal . "]";
                        
                    } else {
                        echo "0 results";
                    }

                    mysqli_close($link);
                    //=====================================================================================================
                    // END: get data from db
                    // ====================================================================================================
                ?>
            <body><header><h1>Wasserstand - Board Develop</h1>

            <nav> <a href="/">[Home]</a> <a href="Wasserstand/filtered.htm">[Value History]</a><a href="handleGraph.htm">[Wasserstand Diagramm]</a>  </nav></header>
            
            <main><article><h2>Wasserstand Zehentner Teisendorf</h2>
                <p>Line Graph<br></p> <br>
                Wasserstand aktuell: <?php echo $levelAct;?>;
                </article>

                <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.js"></script>
                <body><canvas id="Longterm_chart" style="width:100%;max-width:700px"></canvas>


                <script>

                    var xValues = <?php echo $xVal;?>;
                    var yValues = <?php echo $yVal;?>;
                    const graphYlevelWarn = [];generateWarnData("170",0,100,1);
                    const graphYlevelErro = [];generateErroData("185",0,100,1);
                    
                    new Chart("Longterm_chart", { type: "line", data: {   labels: xValues,   
                    datasets: [{     fill: false,     lineTension: 1,     pointRadius: 2,     backgroundColor: "rgba(0,0,255,1.0)",     borderColor: "rgba(0,0,255,0.5)",     label: "Wasserstand [cm]",     data: yValues     },
                    {     fill: false,     lineTension: 0,     pointRadius: 0,     backgroundColor: "rgba(0,255,0,0)",     borderColor: "rgba(0,255,0,0.3)",     label: "Warnschwelle: 170 cm",     data: graphYlevelWarn     },
                    {     fill: false,     lineTension: 0,     pointRadius: 0,     backgroundColor: "rgba(255,0,0,0)",     borderColor: "rgba(255,0,0,0.3)",     label: "Alarmschwelle: 185 cm",     data: graphYlevelErro   }] },
                     options: {   title: {   display: false,   text: "Wasserstand in cm"   },   legend: {display: true, text: "Wasserstand "},   
                     scales: {     yAxes: [{ticks: {min: 100, max:200}}]   } } }); 
                     function generateWarnData(value, i1, i2, step = 1) 
                     {     for (let x = i1; x <= i2; x += step) {       ;       graphYlevelWarn.push(eval(value));     }   } 
                     function generateErroData(value, i1, i2, step = 1) {     for (let x = i1; x <= i2; x += step) 
                     {       ;       graphYlevelErro.push(eval(value));     }   }
                     </script>

                     </body>
                    </main>
                    
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
