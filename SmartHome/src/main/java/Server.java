import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;

public class Server {
    public static void main(String[] args) throws MqttException {
        Listener ls = new Listener();
    }
}
