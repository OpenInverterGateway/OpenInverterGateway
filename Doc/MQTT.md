## MQTT and Homeassistant configuration


This will put the inverter on the energy dashboard.

```yaml
mqtt:
  sensor:
    - state_topic: "energy/solar"
      unique_id: "growatt_wr_total_production"
      name: "Growatt.TotalGenerateEnergy"
      unit_of_measurement: "kWh"
      value_template: "{{ float(value_json.TotalGenerateEnergy) | round(1) }}"
      device_class: energy
      state_class: total_increasing
      json_attributes_topic: "energy/solar"
      last_reset_topic: "energy/solar"
      last_reset_value_template: "1970-01-01T00:00:00+00:00"
      payload_available: "1"
      availability_mode: latest
      availability_topic: "energy/solar"
      availability_template: "{{ value_json.InverterStatus }}"
```


To extract the current AC Power you have to add a sensor template.

```yaml
template:
  - sensor:
      - name: "Growatt inverter AC Power"
        unit_of_measurement: "W"
        state: "{{ float(state_attr('sensor.growatt_inverter', 'OutputPower')) }}"
```

To send commands you can use the MQTT Publish service. Note that the datetime commands are currently only available on the 1.24 modbus version.

```yaml
service: mqtt.publish
data:
  qos: "1"
  topic: energy/solar/command/datetime/get
  payload_template: |
    {
      "correlationId": "ha-datetime-get"
    }
```

```yaml
service: mqtt.publish
data:
  qos: "1"
  topic: energy/solar/command/datetime/set
  payload_template: |
    {
      "correlationId": "ha-datetime-set",
      "value": "{{ now().strftime('%Y-%m-%d %H:%M:%S') }}"
    }
```

To receive responses to commands you can use templates.

```yaml
- trigger:
    platform: mqtt
    topic: energy/solar/result
    value_template: "{{ value_json.correlationId }}"
    payload: "ha-datetime-get"
  sensor:
    name: Growatt - Inverter date/time
    state: "{{ trigger.payload_json.value }}"
    attributes:
      success: "{{ trigger.payload_json.success }}"
      message: "{{ trigger.payload_json.message }}"
```

```yaml
- trigger:
    platform: mqtt
    topic: energy/solar/result
    value_template: "{{ value_json.correlationId }}"
    payload: "ha-datetime-set"
  binary_sensor:
    name: Growatt - Inverter set date/time
    state: "{{ trigger.payload_json.success }}"
    attributes:
      message: "{{ trigger.payload_json.message }}"
```

You could create an automation triggered by a binary sensor on the success JSON attribute if you want to be notified when a command has failed for example.

The following mqtt commands are defined for protocol 1.24:

    datetime/get
    datetime/set
    batteryfirst/get
    batteryfirst/set/powerrate
    batteryfirst/set/stopsoc
    batteryfirst/set/acchargeenabled
    batteryfirst/set/timeslot
    gridfirst/get
    gridfirst/set/powerrate
    gridfirst/set/stopsoc
    gridfirst/set/timeslot
