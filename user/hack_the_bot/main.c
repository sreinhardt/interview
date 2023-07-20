#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAX_BOTS 0x20
#define ADDREDIRECTORBOTURL 0xAA
#define ADDNEWBOT 0xBB
#define CHANGEBOTNAME 0xCC
#define FETCHBOTNAME 0xDD

#define MAX_DOMAIN_LEN 255


#define CHECK_PTR_NULL(p) do {  \
		if(!p)					\
			exit(EXIT_FAILURE);	\
		}while(0);
 


void setup() {
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
}


typedef struct C2Buffer {
	uint8_t* start;
	uint8_t* end;
} C2Buffer;


typedef struct C2Bot {
	size_t domainLen;
	uint8_t* domain;
	uint16_t port;
	uint16_t nameLen;
	uint8_t* name;
} C2Bot;

C2Bot* bots[MAX_BOTS];


C2Bot* getBotByID(uint8_t botID){
	for(uint8_t i = 0; i < MAX_BOTS; i++){
		if(bots[i] && botID == i)
		{	
			return bots[i];
		}	
	}
	return NULL;
}

int fetchBotName(C2Buffer* buf)
{

	uint8_t botID;
	uint8_t tmp_buf[4];
	C2Bot* c2Bot;

	buf->start += sizeof(uint8_t);

	memset(tmp_buf, '\x00', 4);
	memcpy(tmp_buf, buf->start, sizeof(uint8_t));
	botID = (uint8_t)(*tmp_buf);

	c2Bot = getBotByID(botID);
	CHECK_PTR_NULL(c2Bot);

	if(c2Bot->name)
	{
		printf("%d:%s\n", botID, c2Bot->name);
		return 0;
	}
	return 1;
}


int changeBotName(C2Buffer* buf) {
	uint8_t botID;
	uint8_t tmp_buf[4];
	uint16_t new_length;
	C2Bot* c2Bot;


	buf->start += sizeof(uint8_t);

	memset(tmp_buf, '\x00', 4);
	memcpy(tmp_buf, buf->start, sizeof(uint8_t));
	botID = (uint8_t)(*tmp_buf);

	c2Bot = getBotByID(botID);

	CHECK_PTR_NULL(c2Bot);

	buf->start += sizeof(uint8_t);

	memset(tmp_buf, '\x00', 4);
	memcpy(tmp_buf, buf->start, sizeof(uint8_t));

	new_length = (uint16_t)(*tmp_buf);

	buf->start += sizeof(uint16_t);

	if(!new_length)
	{
		return 1;
	}

	if(new_length > c2Bot->nameLen)
	{	

		CHECK_PTR_NULL(c2Bot->name);
		free(c2Bot->name);
		c2Bot->name = malloc(new_length);
	}

	strncpy(c2Bot->name, buf->start, new_length);

	return 0;
}

int addRedirectorBotURL(C2Buffer* buf){
	C2Bot* c2Bot;
	uint8_t botID;
	uint8_t tmp_buf[4];
	uint16_t domainlen;

	buf->start += sizeof(uint8_t);

	memset(tmp_buf, '\x00', 4);
	memcpy(tmp_buf, buf->start, sizeof(uint8_t));
	botID = (uint8_t)(*tmp_buf);

	c2Bot = getBotByID(botID);

	CHECK_PTR_NULL(c2Bot);

	memset(tmp_buf,'\x00',4);
	buf->start += sizeof(uint8_t);

	memcpy(tmp_buf, buf->start, sizeof(uint16_t));
	domainlen = (uint16_t)(*tmp_buf);

	if(!domainlen || domainlen > MAX_DOMAIN_LEN)
	{	
		return 1;
	}

	buf->start += sizeof(uint16_t);
	buf->end = buf->start;

	if(!c2Bot->domain)
	{
		c2Bot->domain = malloc(sizeof(uint8_t) * (MAX_DOMAIN_LEN + 1) ) ;
		CHECK_PTR_NULL(c2Bot->domain);
	} 
	else
	{
		memset(c2Bot->domain, 0, MAX_DOMAIN_LEN + 1);
	}

	buf->end = strstr(buf->start,".io");

	if(!buf->end && c2Bot->domain){
		free(c2Bot->domain);
		c2Bot->domain = NULL;
		return 1;
	}

	c2Bot->domainLen = buf->end - buf->start;
	memcpy(c2Bot->domain, buf->start, c2Bot->domainLen);


	buf->start += domainlen;
	memset(tmp_buf,'\x00',4);
	memcpy(tmp_buf, buf->start, sizeof(uint16_t));
	c2Bot->port = (uint16_t)(*tmp_buf);

	return 0;
}

int addNewBot(C2Buffer* buf){

	C2Bot* newbot = (C2Bot*)malloc(sizeof(C2Bot));
	CHECK_PTR_NULL(newbot);

	uint8_t tmp_buf[4];
	uint16_t namelen;
	uint8_t botID;

	buf->start += sizeof(uint8_t);

	memset(tmp_buf, '\x00', 4);
	memcpy(tmp_buf, buf->start, sizeof(uint8_t));
	botID = (uint8_t)(*tmp_buf);


	if(getBotByID(botID))
	{
		return 1;
	}

	buf->start += sizeof(uint8_t);

	memset(tmp_buf, '\x00', 4);
	memcpy(tmp_buf, buf->start, sizeof(uint16_t));
	namelen = (uint16_t)(*tmp_buf);


	if (!namelen)
	{	
		return 1;
	}

	newbot->nameLen = namelen;

	buf->start += sizeof(uint16_t);
	newbot->name = malloc(namelen);
	strncpy(newbot->name, buf->start, namelen);

	newbot->domain = NULL;
	newbot->domainLen = 0;

	bots[botID] = newbot;

	return 0;
}


int main(int argc, char const *argv[])
{
	
	setup();

	uint8_t tmp[1024];
	C2Buffer tmp_buf;
	uint8_t cmdId;


	while(1){
		memset(tmp, 0, 1024);
		fgets(tmp, 1024, stdin);
		tmp_buf.start = tmp;
		cmdId = (uint8_t)tmp[0];

		switch (cmdId){
			case ADDREDIRECTORBOTURL:
				if(addRedirectorBotURL(&tmp_buf)){
					puts("err:0xAA");
				}
				break;

			case ADDNEWBOT:
				if(addNewBot(&tmp_buf)){
					puts("err:0xBB");
				}
				break;

			case CHANGEBOTNAME:
				if(changeBotName(&tmp_buf))
				{
					puts("err:0xCC");
				}
				break;

			case FETCHBOTNAME:
				if(fetchBotName(&tmp_buf))
				{
					puts("err:0xDD");

				}
				break;
			default:
				puts("err:inv");
				break;
		}	
	}
	
	return 0;
}