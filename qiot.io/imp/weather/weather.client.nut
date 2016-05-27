
#require "APDS9007.class.nut:1.0.0"
#require "LPS25H.class.nut:2.0.1"
#require "Si702x.class.nut:1.0.0"

local fastLog = true
local retryFrequency = 2.0
local sleepDuration = 60.0

local lightOutputPin = hardware.pin5;
lightOutputPin.configure(ANALOG_IN);

local lightEnablePin = hardware.pin7;
lightEnablePin.configure(DIGITAL_OUT, 1);

local lightSensor = APDS9007(lightOutputPin, 47000, lightEnablePin);

hardware.i2c89.configure(CLOCK_SPEED_400_KHZ);
local pressureSensor = LPS25H(hardware.i2c89);
pressureSensor.enable(true)

local tempHumidSensor = Si702x(hardware.i2c89);

local led = hardware.pin2;
led.configure(DIGITAL_OUT, 0);

function deepSleep() {
    server.log("----")
    server.log("ENTER DEEP SLEEP")
    server.sleepfor(sleepDuration)
}

function log() {
    local rssi = imp.rssi()
    local now = date()
    local logMessage = {
        "messages": [{
            "time": format("%04u", now.year) + "-" + format("%02u", now.month) + "-" + format("%02u", now.day) + "T" + format("%02u", now.hour) + ":" + format("%02u", now.min) + ":" + format("%02u", now.sec) + ".000Z",
            "hardware_id": hardware.getdeviceid(),
            "mac": imp.getmacaddress(),
            "free_memory_bytes": imp.getmemoryfree(),
            "rssi_dbm": rssi,
            "firmware_version": imp.getsoftwareversion(),
            "ssid": imp.getssid(),
            "uptime": clock(),
            "liminosity_lux": lightSensor.read()
        }]
    }
    
    pressureSensor.read(function(pressure) {
        local pressure = pressure;
        logMessage.messages[0]["pressure_hpa"] <- pressure.pressure
        
        tempHumidSensor.read(function(reading) {
            logMessage.messages[0]["temperature_c"] <- reading.temperature
            logMessage.messages[0]["humidity_percent"] <- reading.humidity
            
            led.write(1)
            agent.send("qiot.io", logMessage)
            led.write(0)
            
            if (fastLog == true) {
                imp.onidle(log)    
            } else {
                imp.onidle(function() { server.sleepfor(sleepDuration) } )                
            }
        })
    })
}

log()