# __Description__
I have implemented a program - a server. Two clients can connect to the server and communicate with each other, as in a chat. If any of the clients, or even both of them, disconnect from the server, the chat history will be saved on the server. When the client reconnects to the server, the chat history will appear on their screen.

# __Implementation__
The server is given an IPv4 address in standard decimal notation and a port number. Two clients connect to the server using the `netcat` utility and can then send messages to each other in the console. The chat history is saved on the server. If a client disconnects from the server, when they reconnect, the chat history is displayed on their screen. I save the chat history in a dynamically expanding array. To terminate the server, press Ctrl-C. The program will exit without an error, freeing the dynamically allocated memory, as a signal handler is set up. I created a thread for each client, ensuring data integrity and faster program performance.

# __Launch Instructions__

Clone the repository:
```bash
   git clone https://github.com/Sasha-nagibator/Chat -b dev
   cd Chat
```

Start the server. Specify the host number and port in the command line arguments:
```bash
   gcc -pthread main.c
   ./a.out <host> <port>
```

If netcat is not installed:
Команда установки в Ubuntu/Debian:
```bash
   sudo aptitude install netcat
```

Install command on CentOS:
```bash
   sudo yum install netcat
```

Then open 2 terminal tabs and run the following command in both:
```bash
   netcat <host> <port>
```
Specify the same host and port as when starting main.c.

Now the clients can chat with each other. Enter text in one terminal tab, and it will appear in the other. To disconnect the client from the server, press Ctrl-C in the respective tab. To terminate the server, press Ctrl-C in the tab where the server was started.
