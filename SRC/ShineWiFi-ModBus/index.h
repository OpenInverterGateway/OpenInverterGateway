const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE HTML><html>
<!-- Rui Santos - Complete project details at https://RandomNerdTutorials.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files.
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software. -->
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://code.highcharts.com/highcharts.js"></script>
  <style>
    body {
      min-width: 310px;
      max-width: 800px;
      height: 400px;
      margin: 0 auto;
    }
    h2 {
      font-family: Arial;
      font-size: 2.5rem;
      text-align: center;
    }
  </style>
</head>
<body>
  <h2>Growatt Inverter</h2>
  <div id="chart-power" class="container"></div>

  <div id="DataCointainer"> </div>

  <a href="./firmware">Firmware update</a> -
  <a href="./status">Json</a> -
  <a href="./debug">Log</a> -
  <a href="./StartAp">Start config access point</a> -
  <a href="./postCommunicationModbus">RW Modbus</a>

</body>
<script>

const d = new Date();
let diff = d.getTimezoneOffset();
let initialised = false;
let nameToId = {};

Highcharts.setOptions({
time: {
timezoneOffset: diff
}
});

var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-power' },
  title: { text: 'Inverter Data' },
  series: [],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
  },
  xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  credits: {
    enabled: false
  },
  legend: {
    align: 'left',
    verticalAlign: 'top',
    borderWidth: 0,
  },
  tooltip: {
    shared: true,
    crosshairs: true,
  }
});

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {

      // add data fields to the main page
      var obj = JSON.parse(this.responseText);

      // init the UI if not already done
      if (initialised == false) {
        var i = 0;

        // clear data container just in case
        container = document.getElementById("DataCointainer");
        container.innerHTML = "";

        for (var key in obj) {
          // init chart
          if (obj[key][2] == true) {
            chartT.addSeries({
              name: key + " [" + obj[key][1] + "]",
              data: []
            });
            nameToId[key] = i;
            i++;
          }
          // init data container
          var element = document.createElement("p");
          element.innerHTML = key + ": " + obj[key][0] + " " + obj[key][1];
          element.setAttribute("id", key);
          container.appendChild(element);
        }
        initialised = true;
      } else {
        let x = (new Date()).getTime();
        for (var key in obj) {
          // update site data
          var element = document.getElementById(key);
          element.innerHTML = key + ": " + obj[key][0] + " " + obj[key][1];
          // update chart data
          if (obj[key][2] == true) {
            if (chartT.series[nameToId[key]].data.length <= 50) {
              chartT.series[nameToId[key]].addPoint([x, obj[key][0]], true, false, true);
            } else {
              chartT.series[nameToId[key]].addPoint([x, obj[key][0]], true, true, true);
            }
          }
        }
      }
    };
  }
  xhttp.open("GET", "./uistatus", true);
  xhttp.send();
}, 5000 ) ;

</script>



</html>


)=====";
