import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class ATempServer {
    static ServerSocket serverSocket;
    static Socket socket;
    static DataInputStream dataInputStream;
    static DataOutputStream dataOutputStream;

    public void start(int port) {
        boolean connection = true;
        try {
            serverSocket = new ServerSocket(port);
            socket = serverSocket.accept();
            dataInputStream = new DataInputStream(socket.getInputStream());
            dataOutputStream = new DataOutputStream(socket.getOutputStream());

            while (connection) {
                String message = dataInputStream.readUTF();
                System.out.println(message);
                if (message.contains("exit")) {
                    System.out.println("Caught!");
                    dataOutputStream.writeUTF("Server: Terminated");
//                    connection = false;
                } else {
                    dataOutputStream.writeUTF("return");
                }
            }

            serverSocket.close();
        } catch (
                IOException e) {
            e.printStackTrace();
        }

    }

    public static void main(String args[]) {
        ATempServer server = new ATempServer();
        server.start(1234);
    }

}
