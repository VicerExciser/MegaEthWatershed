# MegaEthWatershed
Arduino Mega + Ethernet Shield Sensor Node - Watershed Monitoring for Halfway Crooks Brewery

- Reads water level & pH every 15 seconds
- Sends batch result of 60 timestamped sensor data entries (as JSON string) every 15 minutes via HTTP GET request
- Data payload sent to Pushingbox scenario to be relayed to a Google Apps Script (~96 payloads / day)
- The script (`Code.gs`) parses JSON string payload into a Google Spreadsheet for that particular month/year
- Volumetric flow computations are also handled & logged by the DataLogger GAS
