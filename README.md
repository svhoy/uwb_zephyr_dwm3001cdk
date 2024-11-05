# UWB Indoor Sport Tracking Project 
A Zephyr based Project for uwb module DWM3001cdk based on nRF52832 and DW3110 Transceiver. 

I like to implement an localization system for sport useage (espcially Badminton). 

This Project works with different distance measurment types, SS-TWR and ADS-TWR. Maybe more will be added in future. The main Project folder is Sports Indoor Tracking. Besides this I try to provide simple examples to work with differnt stages of the project. 

The main goal is to implent an system, where the devices connect to a gateway via BLE. The gateway will send setup informations, settings and start/stop the measurement process via GATT with a json msg. The DWM3001cdk than send all information from measurement back to the gateway via BLE notify. At the moment there will be send a struct with all data (WIP: this should be json sometime).

If you have any question, things to improve or you like to add feel free to open an new issue.

Main topic of this part is to provide the distance measurement process. Localization and calibration algorithmen can be found in the other modules of the whole project. 

# usage
```bash
west init -m https://github.com/svhoy/uwb_zephyr_dwm3001cdk --mr main
```

# TODO 
- Readme.md 
- Update old Samples to New Zephyr Version 
- Test PIN Descriptions in Board Description
- Update Project to newer Zepyhr Version

## Project Overview
In this Project all parts for my Project can be found: [Sports Indoor Tracking Project](https://github.com/users/svhoy/projects/4)

You will find the Gateway and the magic for localization and calibration in the Backend.

# Old Projects Takover from DWM1001-DEV
https://github.com/nRFMesh/sdk-uwb-zephyr
driver zephyr port from https://github.com/foldedtoad/dwm1001.git

* `uwb\drivers\dw1000\decadriver`
* `uwb\drivers\dw1000\platform`

Also thank you for your help by porting from DWM1001-DEV to DWM3001CDK
https://github.com/br101/zephyr-dw3000-decadriver
