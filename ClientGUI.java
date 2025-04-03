import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.io.*;
import java.net.*;

public class ClientGUI extends javax.swing.JFrame {
    static Socket socket; //IP Address and port
    static DataInputStream dataInputStream; //Read from server
    static DataOutputStream dataOutputStream; //Write to server
    private static boolean connected; //Flag for connection loop

    //GUI form assets
    private JTextArea msgArea;
    private JTextField msgInput;
    private JButton sendButton;
    private JTextField IPin;
    private JTextField username;
    private JButton disconnectButton;
    private JPanel MainPanel;

    public ClientGUI() {
        //Send Button Action Listener
        sendButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    String msg = "";
                    String out = "";
                    msg = msgInput.getText();
                    out = (username.getText() + ": " + msg);
                    msgArea.setText(msgArea.getText() + "\n" + username.getText() + ": " + msg); //Updating client input
                    dataOutputStream.writeUTF(out); //Sending client input to server
                    msgInput.setText("");
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }
        });

        //Disconnect Button Action Listener
        disconnectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                //Breaking client's connection to server immediately upon button press
                connected = false;
                try {
                    if (socket != null && !socket.isClosed()) {
                        socket.close();
                    }
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
            }
        });
    }

    //Main Method
    public static void main(String[] args) throws IOException {
        connected = true;

        //Creating and filling JFrame with content
        ClientGUI frame = new ClientGUI();
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setTitle("Client GUI");
        frame.setContentPane(frame.MainPanel);
        frame.pack();
        frame.setVisible(true);

        String msgIn = "";
        try {
            socket = new Socket("127.0.0.1", 1234); //Connecting to server
            dataInputStream = new DataInputStream(socket.getInputStream()); //Read from server
            dataOutputStream = new DataOutputStream(socket.getOutputStream()); //Write to server
        } catch (IOException ex) {
            System.out.println("Could not connect to server");
        }

            while (connected) {
                try {
                    msgIn = dataInputStream.readUTF();
                    System.out.println(connected);
                    String dispMsg = (frame.msgArea.getText() + "\n" + "Server: " + msgIn); //Converting readUTF to string to display to msgArea
                    SwingUtilities.invokeLater(() -> frame.msgArea.setText(dispMsg)); //Displaying server message in msgArea
                } catch (IOException ex) {
                    String disconnect = frame.msgArea.getText() + "\n" + "Client: Exiting...";
                    SwingUtilities.invokeLater(() -> frame.msgArea.setText(disconnect)); //Displaying Disconnect
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException ex1) {
                        ex1.printStackTrace();
                    }
                }
            }

            socket.close(); //Disconnecting client from server
            frame.dispatchEvent(new WindowEvent(frame, WindowEvent.WINDOW_CLOSING)); //Exiting GUI

        }
    }



