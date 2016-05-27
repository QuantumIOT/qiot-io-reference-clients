/**
 */

var mqtt = require('mqtt');

function qiotMqttClient(qiotMqttClientOptions) {
	console.log("Starting Qiot Mqtt Client");
	var qiotMqttClient = this;
	qiotMqttClient.options = qiotMqttClientOptions;
	qiotMqttClient.mqttClient = mqtt.connect(qiotMqttClient.options.mqttHref, qiotMqttClient.options.mqttClientConnectionOptions);

	qiotMqttClient.mqttClient.on('connect', function(connack) {
		console.log("QIOT MQTT Client Connected");
		qiotMqttClient.mqttClient.subscribe(qiotMqttClient.options.mqttTopic);
		qiotMqttClient.clientConnected = true;
	});

	qiotMqttClient.mqttClient.on('close', function() {
		console.log("QIOT MQTT Client Connection Closed");
		qiotMqttClient.clientConnected = false;
	});

	qiotMqttClient.mqttClient.on('error', function(error) {
		console.log("QIOT MQTT Client Error", error);
	});

	qiotMqttClient.mqttClient.on('offline', function() {
		console.log("QIOT MQTT Client Offline");
		qiotMqttClient.clientConnected = false;
	});

	qiotMqttClient.mqttClient.on('message', function(topic, message) {
		console.log('QIOT MQTT Client received message %s %s', topic, message);
	});
}

qiotMqttClient.prototype.publishMessage = function(mqttTopic, valuesObject) {
	if (this.clientConnected !== true) {
		console.log("Can't publish message, QIOT MQTT client not connected!");
		return;
	}
	var qiotMqttClient = this;
	var time = new Date().toISOString();

	var message = {};
	message.messages = [];
	message.messages[0] = {};
	message.messages[0].time = new Date().toISOString();
	for (var attrname in valuesObject) {
		message.messages[0][attrname] = valuesObject[attrname];
	}
	var messageString = JSON.stringify(message);
	console.log("QIOT MQTT Client Publishin Topic:", mqttTopic);
	console.log("QIOT MQTT Client Publishing Message:", messageString);
	qiotMqttClient.mqttClient.publish(mqttTopic, messageString);
};


module.exports = function(mqttClientOptions) {
	var instance = new qiotMqttClient(mqttClientOptions);
	return instance;
};