R"(
<?php

if (isset($_POST['submit'])) {
    $_SESSION['time_steps'] = $_POST['time_steps'];
} 
?>

<?php

// set default values
if (isset($_SESSION['time_steps'])){
        $t_steps = intval($_SESSION['time_steps']);
} else {
    $t_steps = 2;                       // zwei Schritte
}

?>

<!DOCTYPE html>
<html lang='en'>
    <head>
        <title>Wasserstand-Messung</title>
        <meta http-equiv="content-type" content="text/html; charset=utf-8">
        <meta name="viewport" content="width=device-width">
        <link rel='stylesheet' type='text/css' href='/f.css'>
        <script src='j.js'></script>
    </head>
    <body>
        <header><h1>Wasserstand - Board Develop</h1>
            <nav> <a href="/">[Home]</a> 
                <a href="filtered.htm">[Value History]</a>
                <a href="longterm_graph.htm">[Longterm Graph]</a>
                <a href="graph.htm">[Shorterm Graph]</a> 
                
            </nav>
        </header>
        <main>
            <article><h2>Wasserstand Zehentner Teisendorf</h2>
                <p>Line Graph -- Shortterm values<br> </p> 
                <br>filterCnt= 6<br><br>  Zeit: 22:19:34   Wasserstand aktuell: 0</article>
                <article>
                      
                        <form action="/testR.htm" method="post">
                        <!-- <form action="/set_time_steps.htm" method="post"> -->
                        <label for="time_steps">Zeitschritte:</label>

                        <input type="number" id="time_steps" name="time_steps" min="1" value="<?php echo isset($_SESSION['time_steps']) ? $_SESSION['time_steps'] : 2; ?>" required>
                        <br>
                        <input type="submit" name="submit" value="Absenden">
                    </form>
                </article>
         
            </main>
            <footer><p><p>Actual Date and Time: 2024-12-10 -- 22:19:34<br>
                <span id='min'>0</span> minutes 
                <span id='sec'>55</span> seconds since startup | Version 6.9 | IP: 192.168.178.171 | Dec  8 2024 18:06:38</p>
            </footer>
        </body>
        </html>

)";