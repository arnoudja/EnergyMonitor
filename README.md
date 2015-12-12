# EnergyMonitor
=====
EnergyMonitor is an application to monitor energy consumption using the P1-port
on a smart energy meter and solar power generation using the Omnik solar
inverter interface. The data will be displayed on a PiTFT and the energy consumption
will be uploaded to PVoutput.org.

I wrote this application as an addition to Omnikstats (https://github.com/arjenv/omnikstats) which I still use to upload the solar data itself.

## Hardware setup

This application uses the following hardware:

* A Raspberry Pi.
* A PiTFT installed on the raspberry pi (https://www.adafruit.com/product/2423).
* A P1-port serial connection to the smart meter.

## Installation and Setup

* The curl development library must be installed:
	sudo apt-get install libcurl4-openssl-dev
* The ftdi development library must be installed:
	sudo apt-get install libftdi-dev
* The cairo development library must be installed:
	sudo apt-get install libcairo2-dev
* wiringPi must be installed, see https://github.com/WiringPi/WiringPi
* Clone the EnergyMonitor source from github
* Edit energymonitor.conf with your API key, SystemID and Omnik configuration
* do a make all
* Grant access to the ftdi device files or run energymonitor as root

