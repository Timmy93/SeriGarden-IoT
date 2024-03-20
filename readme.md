# SeriGarden

An IoT watering system that allow to water and monitor up to 4 plant per time. Server and Frontend can be set up inside your home and there is no need to communciate over internet (fully communicate over MQTT in the local network)

## Hardware requirements

### Main components

1x NodeMCU (tested Lolin v3) - Used to control plant
4x Water pump - Get water for plants
4x Capacitive Soil Moisture Sensor v2.0 - Measure soil humidity
1x Relay (4 ports) - Controls water pump
4x Inline Rectifier Diode (1N4007) - Used to control multilple sensor using 1 analog port

### Other suggestion

Stripboard to assemble components
Pin connectors to assemble easily components
Several meters of cable (26AWG - black, red and yellow in my case)
Several meters of heat-shrink tubing for a better cable management (2.4mm in my case)
1x Water box - A simple container filled with water. It's where water pump will be immersed

## Construction instruction

### Step 1 - Set up stripboard

- Solder the 4 diode (same pin for head, different for tail)
- Connect 4 pin for the 4 analog input
- Connect the 3 entry pin for analog, positive and negative

### Step 2 - Set up the the 4 humidity sensor

1. Cut 12 sensor wires *(4 red, 4 black, 4 yellow)* and 4 heat shrink tubes
2. Insert each group of 3 wires (rby) in a heat shrink tube
3. On one side, attach the female sensor connector to each group of wire
4. On the other side:

- Join the black cables together
- Attach the yellow cables to a female connector *(order is not important)* and isolate using small heat shirnking tube (insert **before** joining cables)
- Attach the red cables to a single female connector and isolate using small heat shirnking tube (insert **before** joining cables)

### Step 3 - Set up water pump

1. Cut 8 pump wires *(4 red, 4 black)* and 4 heat shrink tubes
2. Insert each group of 2 wires (rby) in a heat shrink tube and connect cables to the pump
3. Connect the black wires together to the board negative pole
4. Connect the red wires to each central port of relay (connect the 4° pump to the 3° relay due to known problems with nodemcu 1.0)
5. Connect wires from positive pole to the relay port on the left (pin of relay board are over the relay)


### How to assemble?

Pin D0, D1, D2 are used for the watering plant
Pin D5, D6, D7, D8 are used for monitoring plant

- Plant #1: connect pump to D0 and sensor to D5
- Plant #2: connect pump to D1 and sensor to D6
- Plant #3: connect pump to D2 and sensor to D7
- Plant #4: connect pump to D2 and sensor to D8

## Changelog:

v1.0 (30/07/2023): First release
Main feature

- Control 4 water pump and 4 humidity sensor using 1 NodeMCU (some problems with the 4° pump connected to D3)
- Receive watering control from server
- Communication over MQTT
- Plant ID is set up directly on MQTT

v2.0 (20/03/2024): Main design update

- Manage plant association directly from server
- Adding modular design to substitute failed components
- Known problem: the 4° pump is controlled together with the 3° due to a [problem](https://arduino.stackexchange.com/questions/95770/nodemcu-1-0-port-d3-relay-and-pump-does-not-boot) at boot
