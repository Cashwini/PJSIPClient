all: sip_client

sip_client: sip_client.c
	$(CC) -o $@ $< `pkg-config --cflags --libs libpjproject`
	
clean:
	rm -f sip_client.o sip_client
