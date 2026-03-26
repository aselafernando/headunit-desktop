## Description
Plugin to provide a GPSD client to headunit-desktop

## Instructions
1. Clone repository within the 'modules' folder of headunit-desktop.
2. Modify headunit-desktop/headunit-desktop.pro and add in a line to include plugins/gpsd
3. Compile

## Options

| Variable  | Value      | Description                                                             |
| --------- | ---------- | ----------------------------------------------------------------------- |
| fenceXptY | "lat, lon" | Lat and lon of geo fence X. Each Fence has 4 points and max of 2 fences |
| host      | 127.0.0.1  | GPSD Server                                                             |
| port      | 0          | GPSD Port                                                               |
