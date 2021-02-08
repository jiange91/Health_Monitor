import org.eclipse.paho.client.mqttv3.*;

import java.util.ArrayList;

public class Listener implements MqttCallback {
    private MqttClient client;
    private ArrayList<String> topics;

    public Listener() throws MqttException {
        String host = "tcp://127.0.0.1:1883";
        String clientID = "ServerReceiver";
        topics = new ArrayList<>();
        topics.add("MQTT Examples");

        this.client = new MqttClient(host, clientID);
        MqttConnectOptions connOpts = new MqttConnectOptions();
        connOpts.setCleanSession(true);

        this.client.setCallback(this);
        this.client.connect(connOpts);
        for (String topic : topics) {
            client.subscribe(topic);
        }
    }

    @Override
    public void connectionLost(Throwable throwable) {
        System.out.println("The connection was lost :( due to " + throwable);
        System.exit(1);
    }

    @Override
    public void messageArrived(String topic, MqttMessage mqttMessage) throws Exception {
        System.out.println("Topic: " + topic + " Msg: " + new String(mqttMessage.getPayload()));
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {

    }

}
