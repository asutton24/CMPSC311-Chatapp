import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;

public class ClientGUI extends JFrame {
    private Socket socket;
    private OutputStream rawOutput;
    private PrintWriter output;
    private BufferedReader input;
    private volatile boolean connected;
    public String[] users;

    // GUI component
    private JTextArea msgArea;
    private JTextField msgInput;
    private JButton sendButton;
    private JTextField IPin;
    private JTextField username;
    private JButton disconnectButton;
    public JPanel MainPanel;
    private JTextField portIn;
    private JButton connectButton;
    private JTextArea memberList;

    public ClientGUI() {
        // Set up button actions
        sendButton.addActionListener((ActionEvent e) -> sendMessage());
        msgInput.addActionListener((ActionEvent e) -> sendMessage());
        disconnectButton.addActionListener((ActionEvent e) -> disconnect());
        connectButton.addActionListener((ActionEvent e) -> connectToServer());

        disconnectButton.setEnabled(false);
        sendButton.setEnabled(false);
        users = new String[10];
    }

    private void sendCommand(String arg) {
        byte[] payload = arg.getBytes(StandardCharsets.UTF_8);
        byte[] command = new byte[payload.length + 1];
        command[0] = 0x02;
        System.arraycopy(payload, 0, command, 1, payload.length);

        try {
            rawOutput.write(command);
            rawOutput.flush();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void isTyping() {
        boolean isBlank = msgInput.getText().trim().isEmpty();

        if (!isBlank && connected) {
            String isTypingCommand = "TYPING" + true;
            System.out.println(isTypingCommand);
            sendCommand(isTypingCommand);
        }
    }

    private void requestUserList(){
        String listCommand = "LIST";
        sendCommand(listCommand);
    }

    private void sendUsername() {
        String usernameCommand = "NAME" + username.getText();
        users[0] = username.getText();
        sendCommand(usernameCommand);
    }

    private void updateMembers() {
        SwingUtilities.invokeLater(() -> {
            memberList.setText("");
            for (String user : users) {
                if (!user.isEmpty()) {memberList.append(user + "\n");}
            }
        });
    }

    private void parseServerInput(String buff){
        int userCount;
        if (buff.indexOf(2) < 0) return;
        buff = buff.substring(buff.indexOf(2));
        if (buff.startsWith("USER", 1)){
            appendMessage(buff.substring(5, buff.substring(1).indexOf(2) + 1) + ": " + buff.substring(buff.substring(1).indexOf(2) + 2));
        } else if (buff.startsWith("LIST", 1)){
            userCount = (int)buff.charAt(5) - 47;
            buff = buff.substring(7);
            for (int i = 0; i < userCount; i++){
                users[i] = buff.substring(0, buff.indexOf(2));
                buff = buff.substring(buff.indexOf(2) + 1);
            }
            for (int i = userCount; i < 10; i++){
                users[i] = "";
            }
            System.out.println(userCount);
            updateMembers();
        }
    }

    private void sendMessage() {
        String msg = msgInput.getText().trim();
        if (!msg.isEmpty() && connected) {
            output.println(msg);
            appendMessage("Me: " + msg);
            msgInput.setText("");
        }
    }

    private void appendMessage(String msg) {
        SwingUtilities.invokeLater(() -> msgArea.append("\n" + msg));
    }

    private void connectToServer() {
        new Thread(() -> {
            try {
                String ip = IPin.getText();
                int port = Integer.parseInt(portIn.getText());

                socket = new Socket();
                socket.connect(new InetSocketAddress(ip, port), 5000);

                input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                output = new PrintWriter(socket.getOutputStream(), true);
                rawOutput = socket.getOutputStream();
                sendUsername();

                sendButton.setEnabled(true);
                connectButton.setEnabled(false);
                disconnectButton.setEnabled(true);

                IPin.setEnabled(false);
                portIn.setEnabled(false);
                username.setEnabled(false);

                connected = true;
                appendMessage("Connected to server.");

                listenToServer();

            } catch (IOException | NumberFormatException ex) {
                appendMessage("Could not connect to server. Please try again.");
            }
        }).start();
    }

    private void listenToServer() {
        new Thread(() -> {
            try {
                String msg;
                while (connected && (msg = input.readLine()) != null) {
                    System.out.println(msg);
                    parseServerInput(msg);
                }
            } catch (IOException e) {
                appendMessage("Disconnected from server.");
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

                sendButton.setEnabled(false);
                connectButton.setEnabled(true);
                disconnectButton.setEnabled(false);

                IPin.setEnabled(true);
                portIn.setEnabled(true);
                username.setEnabled(true);
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            ClientGUI frame = new ClientGUI();
            frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            frame.setTitle("Client GUI");
            frame.setContentPane(frame.MainPanel);
            frame.setMinimumSize(new Dimension(500, 250));
            frame.pack();
            frame.setVisible(true);

        });
    }
}
