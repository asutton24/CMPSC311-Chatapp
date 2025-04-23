# Team #1

Andrew Sutton

Daniel Vasquez

Roberto Ramirez Galan

Jeremy Veloso






Steps to access the Server: 


1. You must have a C compiler like gcc on a terminal or command prompt. Linux is needed to run these; you can use VMware to run it on. 

2. To install the the gcc compiler for C in linux, type in on the terminal “sudo apt update” and “ sudo apt install build-essential”. 

3. Go to our website https://github.com/asutton24/CMPSC311-Chatapp.git or in GitHub type this “gh repo clone asutton24/CMPSC311-Chatapp” and download our ZIP. Unzip the folder on your computer. 

4. On your Linux terminal, change your directory to where “CMPSC311-Chatapp” is at. 

5. Compile it and run it. You can compile by typing “gcc server.c logs.c flags.c -o server.c -lpthread”. Logs.c and flags.c have functions that must be included in the server, which is why this is important.

   

Steps to access the Client GUI on Windows: 


1. Download and install Intellij IDEA.

2. In Intellij go to Files, settings, Plugins, search for "Swing” or “UI Designer” and install it. This allows you to create and modify a GUI design. 

![2222222222222](https://github.com/user-attachments/assets/cbb1d4d1-6d09-4dbb-8559-180cf3f19d56)

3. In Intellij, go to File, Open, and select where the downloaded Zip “CMPSC311-Chatapp-main” is. 

4. Compile and run ClientGUI.java. 

5. Once the application opens, type the Ip address and port that the server is at. To find the network to connect the GUI to, type “hostname -I” or “ip addr” on your terminal and locate the Ip address to where your server is running typically, on Linux. Since the port is already defined “8080” on the code, we can use that to connect the GUI.

![111111111111](https://github.com/user-attachments/assets/3b9130a9-d94f-40fc-b7bc-6b035ac44789)



 

 

 

  
