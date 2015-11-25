# PJSIPClient
SIP Client with PJSIP - Use with ASTERISK Server

sip_client is a basic client program with SIP functionalities developed using
PJSIP open source library. This client application is capable to add account,
register and unregister, make a call and terminate calls, handle incoming calls
and busy lines, add a buddy and subscribe for presence. The server compatible
for this client is asterisk server.

The Makefile attached can be used to compile the program. In order for proper 
compilation make sure that sip_client.c and makefile are in PJproject directory.
This program accepts SIP server domain username and password as arguments. 
Please see below for an example:

Ex: ./sip_client 169.254.1.4 username password

The username and password should match one of the entries of sip.conf and 
extensions.conf files of the server.

Once, the program is executed, an account is created with the username provided
and the user's presence is updated to online meaning that the account is 
visible as online to whoever subscribes for presence.

The user then gets a choice menu with options below:

'r' -- To register the account with the SIP server
'u' -- To unregister the account
'b' -- To add a buddy and subscribe for presence
'm' -- To make a call
'h' -- To hangup all calls
'q' -- To quit the application

When the user wants to add a buddy, after entering option 'b', user gets prompt
to enter destination URI. This should include the port number.

Ex: sip:username@sipserver:portnum 

When the user wants to make a call, after entering option 'm', user gets prompt
to enter destination URI. 

Ex: sip:username@sipserver

When the account gets an incoming call, the user is presented with a choice to
either accept the call or reject the call if no other call is onging.

'a' -- To accept the call with 200 OK 
'c' -- To reject the call with 486 Busy Here

If there is an ongoing call, the incoming call is automatically rejected with 
486 Busy Here status code.

When the user chooses to quit the application, all the ongoing activities will
be terminated. If the client is registered, unregistration happens, active
calls are terminated, user presence is destroyed and status is updated to 
offline and then all pjsua libraries are destroyed.

