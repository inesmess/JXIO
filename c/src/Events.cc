
#include <string.h>
#include <map>


#include "Events.h"



Events::Events(){

	this->size = 0;

}



int Events::writeOnSessionErrorEvent(char *buf, struct xio_session *session,
			struct xio_session_event_data *event_data,
			void *cb_prv_data){


	this->event.type = htonl (0);
	this->event.eventSpecific.sessionError.error_type = htonl(event_data->event);
	this->event.eventSpecific.sessionError.error_reason = htonl (event_data->reason);

	this->size = sizeof(int32_t) *3;

	memcpy(buf, &this->event, this->size);
	return this->size;
}


int Events::writeOnSessionEstablishedEvent (char *buf, struct xio_session *session,
			struct xio_new_session_rsp *rsp,
			void *cb_prv_data){

	event.type = htonl (2);
	this->size = sizeof(int32_t);
	memcpy(buf, &this->event, this->size);
	return this->size;

}


int Events::writeOnNewSessionEvent(char *buf, struct xio_session *session,
			struct xio_new_session_req *req,
			void *cb_prv_data){


	void* p1 =  session;

	event.type = htonl (4);
	event.eventSpecific.newSession.ptrSession = htobe64(intptr_t(p1));
	event.eventSpecific.newSession.lenUri = htonl(req->uri_len);

	//copy data so far
	this->size = 	sizeof(intptr_t) + sizeof(int32_t)*2;
	memcpy(buf, &this->event, this->size);

	//copy first string
	strcpy(buf +this->size,req->uri);
	size+=req->uri_len;

	//calculate ip address
	int len;

	struct sockaddr *ipStruct = (struct sockaddr *)&req->src_addr;

	if (ipStruct->sa_family == AF_INET) {
				static char addr[INET_ADDRSTRLEN];
				struct sockaddr_in *v4 = (struct sockaddr_in *)ipStruct;
				event.eventSpecific.newSession.ip = (char *)inet_ntop(AF_INET, &(v4->sin_addr),
							 addr, INET_ADDRSTRLEN);
				len = INET_ADDRSTRLEN;


	}else if (ipStruct->sa_family == AF_INET6) {
			static char addr[INET6_ADDRSTRLEN];
			struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)ipStruct;
			event.eventSpecific.newSession.ip = (char *)inet_ntop(AF_INET6, &(v6->sin6_addr),
						 addr, INET6_ADDRSTRLEN);
			len = INET6_ADDRSTRLEN;

	}else{
			fprintf(stderr, "can not get src ip");
			len = 0;

	}
	event.eventSpecific.newSession.ipLen = htonl (len);
	memcpy(buf + this->size, &event.eventSpecific.newSession.ipLen, sizeof(int32_t));

	this->size += sizeof(int32_t);
	strcpy(buf + this->size,event.eventSpecific.newSession.ip);

	this->size += len ;

	return this->size;


}
int Events::writeOnMsgSendCompleteEvent(char *buf, struct xio_session *session,
			struct xio_msg *msg,
			void *cb_prv_data){

	event.type = htonl (5);
	this->size = sizeof(int32_t);
	memcpy(buf, &this->event, this->size);
	return this->size;


}
int Events::writeOnMsgErrorEvent(char *buf, struct xio_session *session,
            enum xio_status error,
            struct xio_msg  *msg,
            void *conn_user_context){

	event.type = htonl (1);
	this->size = sizeof(int32_t);
	memcpy(buf, &this->event, this->size);
	return this->size;


}
















