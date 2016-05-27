// ****************************************************************
// Receive and parse lora packets from the LoRa Network Server
// Forwards packet information to qiot.io mqtt broker
// ****************************************************************

var mqtt = require("mqtt");
var winston = require("winston");
var loraGW = this;
var logger = new(winston.Logger)({
    transports: [
        new(winston.transports.Console)(),
        new(winston.transports.File)({
            filename: 'plugpower.log'
        })
    ]
});

var url = "mqtt://127.0.0.1";
loraGW.options = require("./config/");
console.log(loraGW.options.mqttClientOptions);
loraGW.qiotMqttClient = require('./lib/qiotMqttClient.js')(loraGW.options.mqttClientOptions);

loraGW.loraMqttClient = mqtt.connect(url);

loraGW.loraMqttClient.on('connect', function() {
    console.log("Lora Mqtt Client Connected & Subscribed");

    // subscribe to all upstream lora packets
    loraGW.loraMqttClient.subscribe('lora/+/up');
});

loraGW.loraMqttClient.on("error", function(error) {
    console.log("mqtt error: ", error);
    exit();
});

function send_to_node(eui, payload) {
    var message = {
        "data": payload
    };

    topic = "lora/" + eui + "/down"
    console.log("topic: ", topic);
    console.log("publishing: ", JSON.stringify(message));

    loraGW.loraMqttClient.publish(topic, JSON.stringify(message));
}

loraGW.loraMqttClient.on('message', function(topic, message) {
    console.log("topic: ", topic);
    //console.log("message: ", message.toString());

    // convert MQTT message to JSON object
    var packetInfo = JSON.parse(message.toString());
    packetInfo.eui = topic.split('/')[1];


    var freq = packetInfo.freq;
    var datarate = packetInfo.datr;
    var snr = packetInfo.lsnr;
    var rssi = packetInfo.rssi;
    var sequence_number = packetInfo.seqn;
    var timestamp = packetInfo.timestamp;
    var chan = packetInfo.chan;

    // decode base64 payload
    packetInfo.data = new Buffer(packetInfo.data, "base64");
    packetInfo.data = packetInfo.data.toString();

    console.log("data: ", packetInfo.data );
    console.log("freq: ", freq);
    console.log("datarate: ", datarate);
    console.log("snr: ", snr);
    console.log("rssi: ", rssi);
    console.log("seq number: ", sequence_number);
    console.log("timestamp: ", timestamp);

    //Log packet information 
    logger.info(packetInfo.eui, ",", rssi, ",", snr, ",", chan, ",", freq, ",", datarate, ",", sequence_number, ",", packetInfo.data);

    //Publish packet info to Qiot MQTT Broker
    loraGW.qiotMqttClient.publishMessage(loraGW.options.mqttClientOptions.mqttTopic, packetInfo);

    // uncomment to send the same message back to the dot
    // send_to_node(eui, json.data)
});