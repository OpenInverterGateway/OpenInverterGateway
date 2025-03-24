#pragma once

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html lang="en">

<!-- Rui Santos - Complete project details at https://RandomNerdTutorials.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files.
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software. -->

<head>
    <meta charset='utf-8'>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Growatt Inverter</title>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/4.2.1/chart.umd.min.js" integrity="sha512-GCiwmzA0bNGVsp1otzTJ4LWQT2jjGJENLGyLlerlzckNI30moi2EQT0AfRI7fLYYYDKR+7hnuh35r3y1uJzugw==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>

    <style>
        body {
            min-width: 310px;
            max-width: 800px;
            height: 400px;
            margin: 0 auto;
            font-family: Arial;
        }

        h2 {
            font-size: 2.5rem;
            text-align: center;
        }

        div {
            font-size: 1rem;
            padding: 10px 0px 10px 0px;
        }

        .linkButtonBar {
            text-align: center;
            padding: 20px 0px 20px 0px;
        }

        .linkButton {
            -webkit-border-radius: 4px;
            -moz-border-radius: 4px;
            border-radius: 4px;
            border: solid 1px #91ca5f;
            text-shadow: 0 -1px 0 rgba(0, 0, 0, 0.4);
            -webkit-box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            -moz-box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            background: #6eb92b;
            color: #FFF;
            padding: 8px 12px;
            text-decoration: none;
            display: inline-block;
            margin: 2px;
            font-size: .8rem;
            text-align: center;
        }

        .yellow {
            border: solid 1px rgb(202, 189, 95);
            text-shadow: 0 -1px 0 rgba(0, 0, 0, 0.4);
            -webkit-box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            -moz-box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            background:rgb(185, 178, 43);
        }

        .red {
            border: solid 1px rgb(202, 95, 95);
            text-shadow: 0 -1px 0 rgba(0, 0, 0, 0.4);
            -webkit-box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            -moz-box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.4), 0 1px 1px rgba(0, 0, 0, 0.2);
            background:rgb(185, 43, 43);
        }
    </style>
</head>

<body>
    <h2>Growatt Inverter</h2>

    <div><canvas id="powerChart"></canvas></div>

    <div class="linkButtonBar">
        <a href="./status" class="linkButton">Json</a>
        <a href="./uiStatus" class="linkButton">UI Json</a>
        <a href="./metrics" class="linkButton">Metrics</a>
        <a href="./debug" class="linkButton">Log</a>
        <a onClick="return confirm('Starting config AP will disconnect you from the device. Are you sure?');" href="./startAp" class="linkButton yellow">Start Config AP</a>
        <a onClick="return confirm('This will reboot the Wifi Stick. Are you sure?');" href="./reboot" class="linkButton yellow">Reboot</a>
        <a href="./postCommunicationModbus" class="linkButton red">RW Modbus</a>
    </div>

    <div id="DataContainer"></div>

<script>
    let initialised = false;
    const powerchartelement = document.getElementById('powerChart');

    const CHART_COLORS = {
        red: 'rgb(255, 99, 132)',
        orange: 'rgb(255, 159, 64)',
        yellow: 'rgb(255, 205, 86)',
        green: 'rgb(75, 192, 192)',
        blue: 'rgb(54, 162, 235)',
        purple: 'rgb(153, 102, 255)',
        grey: 'rgb(201, 203, 207)'
    };

    const NAMED_COLORS = [
        CHART_COLORS.red,
        CHART_COLORS.orange,
        CHART_COLORS.yellow,
        CHART_COLORS.green,
        CHART_COLORS.blue,
        CHART_COLORS.purple,
        CHART_COLORS.grey,
    ];

    function namedColor(index) {
        return NAMED_COLORS[index % NAMED_COLORS.length];
    }

    const powerchartData = {
        labels: [],
        datasets: [],
    };

    let powerchart = new Chart(powerchartelement, {
        type: 'line',
        data: powerchartData,
        options: {
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                // add data fields to the main page
                var obj = JSON.parse(this.responseText);
                // Add Date to chart x Axis
                let date = new Date();
                powerchartData.labels.push(date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds());
                if (initialised == false) {
                    initialised = true;
                    // clear data view container just in case
                    container = document.getElementById("DataContainer");
                    container.innerHTML = "";
                    // Add Data Labels to chart
                    for (var key in obj) {
                        if (obj[key][2] == true) {
                            const newDataset = {
                                label: key,
                                data: [obj[key][0]],
                                fill: false,
                                borderColor: namedColor(powerchart.data.datasets.length),
                                tension: 0.1
                            };
                            powerchartData.datasets.push(newDataset);
                            powerchart.update();
                        }
                        // init dataview
                        var element = document.createElement("p");
                        element.innerHTML = "<a href=\"/value/" + key + "\">" + key + "</a>: " +
                                            obj[key][0] + "&#8239;" + obj[key][1];
                        element.setAttribute("id", key);
                        container.appendChild(element);
                    }
                } else {
                    // Update Data in chart
                    for (var key in obj) {
                        // find dataset in array
                        if (obj[key][2] == true) {
                            for (var dset in powerchartData.datasets) {
                                let leb = powerchartData.datasets[dset].label;
                                if (leb == key) {
                                    powerchartData.datasets[dset].data.push(obj[key][0]);
                                }
                            }
                        }
                        // update data view
                        var element = document.getElementById(key);
                        element.innerHTML = "<a href=\"/value/" + key + "\">" + key + "</a>: " +
                                            obj[key][0] + "&#8239;" + obj[key][1];
                        powerchart.update();
                    }
                }
            }
        }
        xhttp.open("GET", "./uiStatus", true);
        xhttp.send();
    }, 5000);
</script>
</body>
</html>
)=====";

const char SendPostSite_page[] PROGMEM = R"=====(
<!DOCTYPE HTML><html>
<!-- Rui Santos - Complete project details at https://RandomNerdTutorials.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files.
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software. -->
<head>
  <meta charset='utf-8'>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Growatt Inverter</title>
</head>
<body>
  <h2>Growatt Post Communication Modbus</h2>
  <form action="/postCommunicationModbus_p" method="POST">
    <input type="text" name="reg" placeholder="Register ID"></br>
    <input type="text" name="val" placeholder="Input Value (16bit only!)"></br>
    <select name="type">
      <option value="16b" selected>16b</option>
      <option value="32b">32b</option>
    </select></br>
    <select name="operation">
      <option value="R" selected>Read</option>
      <option value="W">Write</option>
    </select></br>
    <select name="registerType">
      <option value="I" selected>Input Register</option>
      <option value="H">Holding Register</option>
    </select></br>
    <input type="submit" value="go">
  </form>
  <a href=".">back</a>
</body>
</html>
)=====";
