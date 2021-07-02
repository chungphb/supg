# Semtech UDP Packet Generator

**SUPG** (**S**emtech **U**DP **P**acket **G**enerator) is a lightweight version of [ChirpStack Simulator](https://github.com/chungphb/chirpstack-simulator) used to simulate LoRaWAN® gateways and devices. Similar to **ChirpStack Simulator**, **SUPG** allows ChirpStack users to test their setups and to observe how ChirpStack really works in action.

## Building

### How to build

As for many C++ projects, **SUPG** can be built using the CMake build system. The minimum required version of CMake is 3.12. To build **SUPG**, use the following commands:

```bash
$ bash scripts/pre-build.sh
$ bash scripts/build.sh
```

The binary file will be located at `bin/supg`.

### Note

Running the bash script `scripts/pre-build.sh` before building is required because **cryptopp** is not fully supporting CMake as of now.

## Configuration

### How to generate a new configuration file

A sample TOML configuration file will be automatically generated along the binary file at `bin/supg.toml`. You can also generate your own configuration file using the `--generate-config-file ` command-line option:

```bash
./bin/supg --generate-config-file [OUTPUT_FILE]
```

where `OUTPUT_FILE` is the path to the output file.

For example: `./bin/supg --generate-config-file ./bin/my_supg.toml`

### How to configure

| Name                                             | Type   | Meaning                                                      | Requirements                     |
| ------------------------------------------------ | :----- | ------------------------------------------------------------ | -------------------------------- |
| `general.log_level`                              | int    | Log level (trace = 6, debug = 5, info = 4, warn = 3, error = 2, critical = 1, off = 0). | >= 0, <= 6                       |
| `network_server.host`                            | string | IP of ChirpStack Gateway Bridge's UDP listener.              | Not empty                        |
| `network_server.port`                            | int    | Port of ChirpStack Gateway Bridge's UDP listener.            | > 0                              |
| `simulator.duration`                             | int    | The time running the simulation (s).                         | > 0                              |
| `simulator.device.first_eui`                     | string | EUI of the first configured device.                          | Length = 16                      |
| `simulator.device.first_address`                 | string | Address of the first configured device.                      | Length = 8                       |
| `simulator.device.first_network_session_key`     | string | Network session key of the first configured device.          | Length = 32                      |
| `simulator.device.first_application_session_key` | string | Application session key of the first configured device.      | Length = 32                      |
| `simulator.device.count`                         | int    | Number of devices.                                           | > 0, <= 1000                     |
| `simulator.device.uplink_interval`               | int    | The time between two consecutive uplink transmissions (s).    | > 0, < `simulator.duration`      |
| `simulator.device.f_port`                        | int    | FPort.                                                       | > 0                              |
| `simulator.device.payload`                       | string | Uplink payload.                                              | Not empty                        |
| `simulator.device.frequency`                     | int    | Frequency (Hz).                                              | > 0                              |
| `simulator.device.bandwidth`                     | int    | Bandwidth (Hz).                                              | > 0                              |
| `simulator.device.spreading_factor`              | int    | Spreading-factor.                                            | > 0                              |
| `simulator.device.f_cnt`                         | int    | Uplink frame counter.                                        | >= 0                             |
| `simulator.gateway.first_id`                     | string | ID of the first configured gateway.                          | Length = 16                      |
| `simulator.gateway.min_count`                    | int    | Minimum number of receiving gateways.                        | > 0                              |
| `simulator.gateway.max_count`                    | int    | Maximum number of receiving gateways.                        | >= `simulator.gateway.min_count` |

### Example

```toml
[general]
log_level = 4

[network_server]
host = "localhost"
port = 1700

[simulator]
duration = 60

[simulator.device]
first_eui = "0000000000000000"
first_address = "00000000"
first_network_session_key = "00000000000000000000000000000000"
first_application_session_key = "00000000000000000000000000000000"
count = 1
uplink_interval = 10
f_port = 10
payload = "test_packet_1234"
frequency = 868100000
bandwidth = 125
spreading_factor = 12
f_cnt = 0

[simulator.gateway]
first_id = "0000000000000000"
min_count = 1
max_count = 1
```

### Note

* Unlike **ChirpStack Simulator**, **SUPG** does not use [ChirpStack Client](https://github.com/chungphb/chirpstack-client) to set up gateways and devices before or tear them down after the simulation (to avoid installing **gRPC**). Therefore, everything relating to the simulation must be manually handled. A fixed set of gateways and devices will be a lot easier to manage than a random one.
* The `k`-th gateway of the configured gateway set will have:
    * *Gateway ID:* `simulator.gateway.first_id  + k`
* The `k`-th device of the configured device set will have:
    * *Device EUI:* `simulator.device.first_eui  + k`
    * *Device address:*** `simulator.device.first_address + k`
    * *Network session key:* `simulator.device.first_network_session_key + k `
    * *Application session key:* `simulator.device.first_application_session_key + k`
* As running the simulator multiple times on a fixed set of devices might result in ChirpStack refusing to process uplink frames due to conflicted frame counters, the `simulator.device.f_cnt` configuration parameter should be modified (usually increased) each time you run the simulator to avoid recreating old devices.

## Running

### How to set up ChirpStack before running

As mentioned above, because **SUPG** does not set up ChirpStack while running, you have to create/update everything beforehand.

- To set up gateways, all the configured gateways must be created.
- To set up devices, a service-profile, a device-profile, an application and all the configured devices must be created. You also need to activate these devices using their associated device address, network session key and application session key.

### How to run

To start **SUPG**, use the following command:

```bash
./bin/supg --config [CONFIG_FILE]
```

where `CONFIG_FILE` is the path to the configuration file.

For example: `./bin/supg --config ./bin/my_supg.toml`

### What happens during the simulation

After the simulation starts, devices will periodically send uplink frames (and handle downlink frames, if any) to the configured network server through the gateways assigned to them.

The simulation will be gracefully stopped after a `simulator.duration` interval.

### How to stop

**SUPG** will be terminated by any of the following conditions:

* After a `simulator.duration` interval.
* When an exception is thrown.
* When receiving a SIGINT signal.

When the simulation stops, all the gateways and devices (as well as the service-profile, the device-profile and the application) created earlier should be deleted.

### Note

* When creating a new device-profile, set:
    * *LoRaWAN MAC version:* 1.0.3
    * *LoRaWAN Regional Parameters revision:* B
    * *ADR algorithm:* Default ADR algorithm
    * *Uplink interval:* 60
* To send downlink payloads to a simulated device, you can enqueue device queue items to its queue using **ChirpStack Application Server** web-interface. The data need to be encoded to **Base64** format. This could be done by running **SUPG** with the `--base64-encode` command-line option.

## License

This project is licensed under the terms of the MIT license.