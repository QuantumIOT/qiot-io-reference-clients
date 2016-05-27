/**
 * Definition of options object
 * @author Felix Galindo
 */

const options = {
	mqttClientOptions: {
		mqttTopic: "1/l/t/85284622-3940-754b-d5sy-g168416j115n", //Add support for mapping topics to sensors
		mqttHref: "mqtt://qiot.io:1883",
		mqttClientConnectionOptions: {
			username: "mqtt-api-user",
			password: "902d22c0-cc6d-470e-91aa-477863701457",
			clientId: "85284622-3940-754b-d5sy-g168416j115n"
		}
	}
	// mqttClientOptions: {
	// 	mqttTopic: "plugpowerLora-3981u39867", //Add support for mapping topics to sensors
	// 	mqttHref: "mqtt://broker.hivemq.com:1883",
	// 	mqttClientConnectionOptions: {
	// 		clientId: "lens_7OHrNPRrWtuF0NorkXCnFFTQPPL"
	// 	}
	// }
};

module.exports = options;