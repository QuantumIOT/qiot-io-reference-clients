var config = require('./config')
var tessel = require('tessel')
var accel = require('accel-mma84').use(tessel.port['A'])

var http = require('http')
var mqtt = require('mqtt')

var client = mqtt.connect(config.mqtt.host, config.mqtt.credentials)

var dweetG = null
var connected = false

var topicName = '1/l/t/' + tessel.deviceId()

client.on('message', function(topic, message) {
    try {    
        console.log('recv config: ' + JSON.stringify(config.mqtt) + ', topic: ' + topic + ', message: ' + message)
    } catch (e) {
        console.log('e:' + e.stack)
    }        
})
 
client.on('connect', function() {
    try {    
    	console.log('connected')
    	client.subscribe(topicName)
        connected = true
    } catch (e) {
        console.log('e:' + e.stack)
    }        
})

client.on('close', function() {
    try {
        connected = false
        console.log('disconnected')
        client.unsubscribe(topicName)
        client = mqtt.connect(config.mqtt.host, config.mqtt.credentials)
    } catch (e) {
        console.log('e:' + e.stack)
    }        
})

accel.on('data', function(xyz) {
    try {
    	var message = { "messages": [{
                "time": new Date().toISOString(),
                "acceleration_x": parseFloat(xyz[0].toFixed(4)),
                "acceleration_y": parseFloat(xyz[1].toFixed(4)),
                "acceleration_z": parseFloat(xyz[2].toFixed(4))
            }]
        }

        if (connected === true) {
            console.log('sent config: ' + JSON.stringify(config.mqtt) + ', topic: ' + topicName + ', message: ' + JSON.stringify(message))
            client.publish(topicName, JSON.stringify(message))
        } else {
            console.log('not connected yet')
        }
    } catch (e) {
        console.log('e:' + e.stack)
    }
})
