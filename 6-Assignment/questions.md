1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

A special EOF character sent via send_message_eof() after command execution completes is used to determine when a command's output is fully received from the server. The client stops reading from the server after this EOF character is read.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

Null characters (\0) mark the end of commands sent from client to server, and a special EOF character marks end of command output from server to client. Not handling this properly would cause several issues including fragmented messages (the server may read partial commands leading to errors or indefinite waits), concatenated messages (several commands could be combined together without a proper delimiter to separate them), deadlocks (server/client may wait indefinitely) and buffer overflows.

3. Describe the general differences between stateful and stateless protocols.

Stateful: A stateful protocol keeps track of the state of each communication session, meaning it remembers previous interactions.
The server or client retains information about past requests, making the session dependent on prior exchanges. Eg. TCP, FTP

Stateless: A stateless protocol does not retain any memory of previous interactions. Every request must contain all necessary information to be understood by the server, as no history is kept between requests. Eg. HTTP, UDP

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

Since UDP has lower latency and reduced overhead, it would be ideal in situations where a minor loss of packets is negligible compared to the timely delivery of packets. So, UDP is used in DNS lookups, video playbacks and real-time multiplayer online games.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

Socket interface/API is the primary abstraction/interface provided by the operating system to enable applications.