import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;

//Created a public class that extends the JFrame (ClientGUI), this is where all the functions and the main method are held.
public class ClientGUI extends JFrame {
    private Socket socket;
    private OutputStream rawOutput; //rawOutput is used for output for bytes, this is important when sending our control variable.
    private PrintWriter output; //output is for sending strings - user messages - to the server.
    private BufferedReader input;
    private volatile boolean connected; //Volatile boolean variable for immediate updates to all threads
    public String[] users; //A string array containing all the users connected to the server

    // GUI components
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

    //This is where all the ActionEvent listeners are tied to their specific function
    public ClientGUI() {
        // Set up button actions
        sendButton.addActionListener((ActionEvent e) -> sendMessage());
        msgInput.addActionListener((ActionEvent e) -> sendMessage()); //This just allows the user to send messages by pressing the "Enter" key while typing, since that keystroke is viewed as an action event while in the text field.
        disconnectButton.addActionListener((ActionEvent e) -> disconnect());
        connectButton.addActionListener((ActionEvent e) -> connectToServer());

        //Setting disconnect and send buttons to false upon start of program so the user cannot crash the program by trying to send or disconnect before connection to the server
        disconnectButton.setEnabled(false);
        sendButton.setEnabled(false);

        users = new String[10]; //Setting the max user array to 10 since that is the max amount of users allowed to be connected to the server.
    }

    //Command function to standardize sending commands with other functions such as sendUsername()
    private void sendCommand(String arg) {
        byte[] payload = arg.getBytes(StandardCharsets.UTF_8); //Making a byte array encoded with UTF_8 based on the string argument
        byte[] command = new byte[payload.length + 1]; //Making a new byte array with the length of the payload plus one for the control byte
        command[0] = 0x02; //Control byte for sending commands set at position 0 in the array, the server will read this byte and understand that the following bytes are a command
        System.arraycopy(payload, 0, command, 1, payload.length); //Copies the payload bytes into the command byte array starting at position one

        try {
            rawOutput.write(command); //sending the bytes to the server
            rawOutput.flush(); //flushing the bytes so that they are sent immediately

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

//    private void isTyping() {
//        boolean isBlank = msgInput.getText().trim().isEmpty();
//
//        if (!isBlank && connected) {
//            String isTypingCommand = "TYPING" + true;
//            System.out.println(isTypingCommand);
//            sendCommand(isTypingCommand);
//        }
//    }

//    private void requestUserList() {
//        String listCommand = "LIST";
//        sendCommand(listCommand);
//    }

    //Command to send username to the server upon connection
    private void sendUsername() {
        String usernameCommand = "NAME" + username.getText(); //command or "payload" to be used in the sendCommand function
        users[0] = username.getText(); //setting the first user in the array to self
        sendCommand(usernameCommand);
    }

    private void updateMembers() {
        SwingUtilities.invokeLater(() -> {
            memberList.setText("");
            //Updating member list for all members in the array
            for (String user : users) {
                if (!user.isEmpty()) {memberList.append(user + "\n");}
            }
        });
    }

    private void parseServerInput(String buff){
        int userCount;
        // If not a command then return
        if (buff.indexOf(2) < 0) return;
        // Remove leading garbage
        buff = buff.substring(buff.indexOf(2));
        if (buff.startsWith("USER", 1)){
            // USER means that this is a message from a user, this function parses and posts the message
            appendMessage(buff.substring(5, buff.substring(1).indexOf(2) + 1) + ": " + buff.substring(buff.substring(1).indexOf(2) + 2));
        } else if (buff.startsWith("LIST", 1)){
            // LIST will parse list of active users and update the user array
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

    //Println function to server
    private void sendMessage() {
        String msg = msgInput.getText().trim(); //Getting text from msgInput in GUI form
        if (!msg.isEmpty() && connected) { //Making sure the user is connected and the msgInput is not null
            output.println(msg); //Sending message to server
            appendMessage("Me: " + msg); //Appending my msg to the TextArea
            msgInput.setText(""); //Clearing msgInput
        }
    }

    //Appending msgs to the TextArea
    private void appendMessage(String msg) {
        SwingUtilities.invokeLater(() -> msgArea.append("\n" + msg));
    }

    //Connecting to the server on a separate thread
    private void connectToServer() {
        new Thread(() -> {
            try {
                String ip = IPin.getText(); //Pulling IP from IPin
                int port = Integer.parseInt(portIn.getText()); //Pulling port string from portIn and parsing it to an int

                //Creating a new socket and connecting to it the user defined IP and Port
                socket = new Socket();
                socket.connect(new InetSocketAddress(ip, port), 5000);

                //Creating input and output streams that will properly read and send data to and from the server
                input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                output = new PrintWriter(socket.getOutputStream(), true);
                rawOutput = socket.getOutputStream(); //Raw output for sending control bytes
                sendUsername(); //Sending username upon connection

                //Enabling send and disconnect buttons after connection
                sendButton.setEnabled(true);
                connectButton.setEnabled(false);
                disconnectButton.setEnabled(true);

                //Disabling the edit-ability
                IPin.setEnabled(false);
                portIn.setEnabled(false);
                username.setEnabled(false);

                connected = true; //Setting valotile boolean to true upon connection
                appendMessage("Connected to server.");

                listenToServer(); //Starting the listen to server function

            } catch (IOException | NumberFormatException ex) {
                appendMessage("Could not connect to server. Please try again.");
            }
        }).start(); //Starting thread
    }

    //Listening and receiving input from server on a separate thread
    private void listenToServer() {
        new Thread(() -> {
            try {
                String msg; //Defining msg string
                while (connected && (msg = input.readLine()) != null) {
//                    System.out.println(msg);
                    parseServerInput(msg); //Using parseServerInput to receive msgs
                }
            } catch (IOException e) {
                appendMessage("Disconnected from server.");
            } finally {
                disconnect(); //Disconnect from server
            }
        }).start(); //Starting new thread
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
