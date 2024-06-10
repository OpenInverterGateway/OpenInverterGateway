# Homeassistant configuration

To make use of MQTT protocol that sends data from Growatt datalogger you need a MQTT broker and MQTT integration with this broker in Home Assistant.

If you don't have your own MQTT broker, than just use existing add-on in HomeAssistant. Here how you do it.

1. Settings -> Add-ons go to Add On store and look for `Mosquitto broker`, install it. default settings will suffice.
2. Start the Mosquitto add-on.
3. Go to Settings -> Devices & services. In Integrations it will automatically suggest `MQTT` integration, create it with default settings.

Now let's create a user for mqtt broker to use from Growatt datalogger.

1. Go to Settings -> People -> Users. Press `Add User`, username: `growatt`, password: `your password`, mark it as `local only`.

Time to configure MQTT settings on Growatt datalogger. If it's already in your network go to it's IP address and press `Start config access point`,
or press AP button on the datalogger (see specific info for your logger how to enable AP mode). Connect to `Growatt` network, go to <http://192.168.4.1>

Choose `Setup` and set the mqtt parameters:

- mqtt server ip: ip of your home assistant instance
- mqtt port: leave default (1883)
- mqtt user: growatt (Or whatever user you have created)
- mqtt password: \<password of that user>

Save the settings.

Disconnect from AP, it should eventually turn off AP and reconnect to your home wifi and start sending data. You can check if it is successful going to ip-address of your Growatt device and checking logs.  

## MQTT inside Homeassistant configuration

This will put the inverter on the energy dashboard. Add it to your `/config/configuration.yaml`. (You have SSH Add-on for Home assistant installed and can SSH to it).

Different protocol versions send different packages thus might need different configuration.

To find which properties are available checkout name of the registers in relevant cpp file.

E.g. for protocol 3.05 in [Growatt305.cpp](/SRC/ShineWiFi-ModBus/Growatt305.cpp)

Here are some examples:

- [Version for protocol 1.24](#version-for-protocol-124)
- [Version for protocol 3.05](#version-for-protocol-305)

### Version for protocol 1.24

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
    power/get/activerate
    power/set/activerate

### Version for protocol 3.05

```yaml
mqtt:
  sensor:
    - state_topic: "energy/solar"
      unique_id: "growatt_total_production"
      name: "Growatt. Energy Total"
      unit_of_measurement: "kWh"
      value_template: "{{ float(value_json.EnergyTotal) | round(1) }}"
      device_class: energy
      state_class: total_increasing
      json_attributes_topic: "energy/solar"
      payload_available: "1"
      availability_mode: latest
      availability_topic: "energy/solar"
      availability_template: "{{ value_json.InverterStatus }}"

    - name: "Growatt Status"
      unique_id: "growatt_status"
      state_topic: "energy/solar"
      value_template: "{{ value_json.Status }}"

    - name: "Growatt DC Voltage"
      unique_id: "growatt_dc_voltage"
      unit_of_measurement: "V"
      device_class: "voltage"
      state_topic: "energy/solar"
      value_template: "{{ value_json.DcVoltage }}"
      
    - name: "Growatt AC Frequency"
      unique_id: "growatt_ac_frequency"
      unit_of_measurement: "Hz"
      state_class: "measurement"
      state_topic: "energy/solar"
      device_class: "frequency"
      value_template: "{{ value_json.AcFrequency }}"  

    - name: "Growatt AC Voltage"
      unique_id: "growatt_ac_voltage"
      unit_of_measurement: "V"
      state_class: "measurement"
      device_class: "voltage"
      state_topic: "energy/solar"
      value_template: "{{ value_json.AcVoltage }}"        
  
    - name: "Growatt AC Power"
      unique_id: "growatt_ac_power"
      unit_of_measurement: "W"
      state_class: "measurement"
      device_class: "power"
      state_topic: "energy/solar"
      value_template: "{{ value_json.AcPower }}"              
      
    - name: "Growatt Energy Today"
      unique_id: "growatt_energy_today"
      unit_of_measurement: "kWh"
      state_class: "total_increasing"
      device_class: "energy"
      state_topic: "energy/solar"
      value_template: "{{ value_json.EnergyToday }}"  
      
    - name: "Growatt Temperature"
      unique_id: "growatt_temperature"
      unit_of_measurement: "°C"
      state_class: "measurement"
      device_class: "temperature"
      state_topic: "energy/solar"
      value_template: "{{ value_json.Temperature }}"
```
## Direct Access

Using mosquitto_sub/mosquitto_pub directly from the cli.

Subscribe to mqtt events from the stick:

    mosquitto_sub -h <ip> -u <mqttuser> -P <mqttpw> -t "energytest/" -v

Get battery first state:

    mosquitto_pub -h <ip> -u <mqttuser> -P <mqttpw> -m "{\"correlationId\": \"ha-batteryfirst-get\"}" -t energytest/solar/command/batteryfirst/get

Example how to limit output power in percent via mosquitto_pub from cli:

    mosquitto_pub -h <mqttip> -u <mqttuser> -P <mqttpw> \
    -t "<base-topic>/command/power/set/activeRate" -m "{ \"value\": 50 }"