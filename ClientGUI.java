import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.io.*;
import java.net.*;

public class ClientGUI extends JFrame {
    private Socket socket;
    private DataInputStream dataInputStream;
    private DataOutputStream dataOutputStream;
    private volatile boolean connected;

    // GUI components
    private JTextArea msgArea;
    private JTextField msgInput;
    private JButton sendButton;
    private JTextField IPin;
    private JTextField username;
    private JButton disconnectButton;
    public JPanel MainPanel;

    public ClientGUI() {
        // Set up button actions
        sendButton.addActionListener((ActionEvent e) -> sendMessage());
        disconnectButton.addActionListener((ActionEvent e) -> disconnect());
    }

    private void sendMessage() {
        try {
            String msg = msgInput.getText().trim();
            if (!msg.isEmpty()) {
                String out = username.getText() + ": " + msg;
                dataOutputStream.writeUTF(out);
                appendMessage("Me: " + msg);
                msgInput.setText("");
            }
        } catch (IOException e) {
            appendMessage("Error sending message.");
            e.printStackTrace();
        }
    }

    private void appendMessage(String msg) {
        SwingUtilities.invokeLater(() -> msgArea.append("\n" + msg));
    }

    private void connectToServer(String ip, int port) {
        while (true) {
            try {
                socket = new Socket();
                socket.connect(new InetSocketAddress(ip, port), 5000);
                dataInputStream = new DataInputStream(socket.getInputStream());
                dataOutputStream = new DataOutputStream(socket.getOutputStream());
                connected = true;
                appendMessage("Connected to server.");
                break;
            } catch (IOException ex) {
                appendMessage("Could not connect to server. Retrying in 5 seconds...");
                try {
                    Thread.sleep(5000);
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                    return;
                }
            }
        }
    }

    private void listenToServer() {
        new Thread(() -> {
            try {
                while (connected) {
                    String msgIn = dataInputStream.readUTF();
                    appendMessage("Server: " + msgIn);
                }
            } catch (IOException e) {
                appendMessage("Disconnected from server.");
                try {
                    Thread.sleep(3000);
                } catch (InterruptedException ex) {
                    throw new RuntimeException(ex);
                }
            } finally {
                disconnect();
            }
        }).start();
    }

    private void disconnect() {
        connected = false;
        try {
            if (socket != null && !socket.isClosed()) {
                socket.close();
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        dispatchEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            ClientGUI frame = new ClientGUI();
            frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            frame.setTitle("Client GUI");
            frame.setContentPane(frame.MainPanel);
            frame.pack();
            frame.setVisible(true);

            // Run connection + listening on a separate thread
            new Thread(() -> {
                final String serverIP = "192.168.0.192";
                final int port = 8080;

                frame.connectToServer(serverIP, port);
                frame.listenToServer();
            }).start();
        });
    }
}

///OLD CODE REFERENCE ONLY
//import javax.swing.*;
//import java.awt.event.ActionEvent;
//import java.awt.event.ActionListener;
//import java.awt.event.WindowEvent;
//import java.io.*;
//import java.net.*;
//
//public class ClientGUI extends javax.swing.JFrame {
//    static Socket socket; //IP Address and port
//    static DataInputStream dataInputStream; //Read from server
//    static DataOutputStream dataOutputStream; //Write to server
//    private static boolean connected; //Flag for connection loop
//
//    //GUI form assets
//    private JTextArea msgArea;
//    private JTextField msgInput;
//    private JButton sendButton;
//    private JTextField IPin;
//    private JTextField username;
//    private JButton disconnectButton;
//    private JPanel MainPanel;
//
//    public ClientGUI() {
//        //Send Button Action Listener
//        sendButton.addActionListener(new ActionListener() {
//            @Override
//            public void actionPerformed(ActionEvent e) {
//                try {
//                    String msg = "";
//                    String out = "";
//                    msg = msgInput.getText();
//                    out = (username.getText() + ": " + msg);
//                    msgArea.setText(msgArea.getText() + "\n" + username.getText() + ": " + msg); //Updating client input
//                    dataOutputStream.writeUTF(out); //Sending client input to server
//                    msgInput.setText("");
//                } catch (IOException e1) {
//                    e1.printStackTrace();
//                }
//            }
//        });
//
//        //Disconnect Button Action Listener
//        disconnectButton.addActionListener(new ActionListener() {
//            @Override
//            public void actionPerformed(ActionEvent e) {
//                //Breaking client's connection to server immediately upon button press
//                connected = false;
//                try {
//                    if (socket != null && !socket.isClosed()) {
//                        socket.close();
//                    }
//                } catch (IOException ex) {
//                    ex.printStackTrace();
//                }
//            }
//        });
//    }
//
//    //Main Method
//    public static void main(String[] args) throws IOException {
//        try {
//            socket = new Socket(); //Connecting to server
//            socket.connect(new InetSocketAddress("192.168.0.192", 8080), 5000);
//            dataInputStream = new DataInputStream(socket.getInputStream()); //Read from server
//            dataOutputStream = new DataOutputStream(socket.getOutputStream()); //Write to server
//        } catch (IOException ex) {
//            System.out.println("Could not connect to server");
//            try {
//                Thread.sleep(5000);
//            } catch (InterruptedException e) {
//                throw new RuntimeException(e);
//            }
//            main(args);
//        }
//
//        //Creating and filling JFrame with content
//        ClientGUI frame = new ClientGUI();
//        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
//        frame.setTitle("Client GUI");
//        frame.setContentPane(frame.MainPanel);
//        frame.pack();
//        frame.setVisible(true);
//
//        connected = true;
//        String msgIn = "";
//        while (connected) {
//            try {
//                msgIn = dataInputStream.readUTF();
//                System.out.println(connected);
//                    String dispMsg = (frame.msgArea.getText() + "\n" + "Server: " + msgIn); //Converting readUTF to string to display to msgArea
//                    SwingUtilities.invokeLater(() -> frame.msgArea.setText(dispMsg)); //Displaying server message in msgArea
//                } catch (IOException ex) {
//                    String disconnect = frame.msgArea.getText() + "\n" + "Client: Exiting...";
//                    SwingUtilities.invokeLater(() -> frame.msgArea.setText(disconnect)); //Displaying Disconnect
//                    try {
//                        Thread.sleep(1000);
//                    } catch (InterruptedException ex1) {
//                        ex1.printStackTrace();
//                    }
//                }
//            }
//
//            socket.close(); //Disconnecting client from server
//            frame.dispatchEvent(new WindowEvent(frame, WindowEvent.WINDOW_CLOSING)); //Exiting GUI
//
//        }
//    }