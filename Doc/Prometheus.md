# Prometheus Configuration

To scrape the metrics of your Growatt inverter using [Prometheus](https://prometheus.io/), it is necessary to set up a Prometheus server. If you are not familiar with this technology, please refer to the [getting started tutorial](https://prometheus.io/docs/prometheus/latest/getting_started/).

A possible configuration for the `prometheus.yml` file used to scrape the metrics every second is as follows:

```yaml
global:
  scrape_interval: 1m

scrape_configs:
  - job_name: 'growatt'
    static_configs:
      - targets: ['<ip>:80']
    metrics_path: /metrics
```

You can check the names of the metrics available by inspecting the endpoint `http://<ip>/metrics`. You should get a response similar to the following:

```
growatt_inverter_status{mac="<mac>"} 5
growatt_input_power{mac="<mac>"} 2520
growatt_pv1_voltage{mac="<mac>"} 261.5
[...]
```

For instance, a metric name is `growatt_inverter_status`. You can query Prometheus using the [PromQL language](https://prometheus.io/docs/prometheus/latest/querying/basics/). A basic query consists of the metric name. You can check if your metrics are being collected by querying Prometheus using a metric name.

## Grafana dashboard

It is quite common to use [Grafana](https://grafana.com/oss/grafana/) to build dashboards using Prometheus as a data source. You will need to install Grafana and configure it to query Prometheus. Then, it is possible to build a dashboard. If your Modbus version is v1.24, you can refer to [this dashboard](https://grafana.com/grafana/dashboards/20646) as a possible example. It may work without modifications or not, depending on the model of your inverter and the available metrics.
