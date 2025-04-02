import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class ClientGUI extends javax.swing.JFrame {
    static ServerSocket serverSocket;
    static Socket socket;
    static DataInputStream dataInputStream;
    static DataOutputStream dataOutputStream;

    private JTextArea msgArea;
    private JTextField msgInput;
    private JButton sendButton;
    private JTextField IPin;
    private JTextField username;
    private JButton disconnectButton;
    private JPanel MainPanel;

    public ClientGUI() {
        sendButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    String msg = "";
                    msg = msgInput.getText();
                    msgArea.setText(msgArea.getText() + "\n" + username.getText() + ": " + msg);
                    dataOutputStream.writeUTF(msg);
                    msgInput.setText("");
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }
        });
    }

    public void main(String[] args) {
        JFrame frame = new ClientGUI();
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setTitle("Client GUI");
        frame.setContentPane(new ClientGUI().MainPanel);
        frame.pack();
        frame.setVisible(true);
        String msgIn = "";
        try {
            socket = new Socket("127.0.0.1", 8080);
            dataInputStream = new DataInputStream(socket.getInputStream());
            dataOutputStream = new DataOutputStream(socket.getOutputStream());

            while (!msgIn.equals("exit")) {
                msgIn = dataInputStream.readUTF();
                msgArea.setText(msgArea.getText() + "\n" + username.getText() + ": " + msgIn);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}


