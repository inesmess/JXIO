/*
** Copyright (C) 2013 Mellanox Technologies
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at:
**
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
** either express or implied. See the License for the specific language
** governing permissions and  limitations under the License.
**
*/

#include <sys/timerfd.h>

#include "Msg.h"
#include "MsgPool.h"

//Msg::Msg(void * buf, struct xio_mr 	*xio_mr, int in_buf_size, int out_buf_size)
Msg::Msg(void * buf, struct xio_mr 	*xio_mr, int in_buf_size, int out_buf_size, MsgPool* pool)
{
	this->buf = buf;
	this->xio_mr = xio_mr;
	this->in_buf_size = in_buf_size;
	this->out_buf_size = out_buf_size;
	this->xio_msg = (struct xio_msg *) calloc(1, sizeof(struct xio_msg));
	this->pool = pool;

	log (lsDEBUG, "****** buf inside msg out size is %d\n", this->out_buf_size);

	this->set_xio_msg_client_fields();
}

Msg::~Msg()
{
	free (this->xio_msg);
}

struct xio_msg 	* Msg::get_xio_msg(){
//	log (lsDEBUG, "this->xio_msg is %p, req is %p\n", this->xio_msg, this->xio_msg->request);
	return xio_msg;
}

void Msg::set_xio_msg_client_fields()
{
	this->xio_msg->user_context = this; //we will be able to recieve it back on responce from server

	this->xio_msg->out.header.iov_base = NULL;
	this->xio_msg->out.header.iov_len = 0;
	this->xio_msg->out.data_iovlen = 1;
	this->xio_msg->out.data_iov[0].iov_base = this->buf + in_buf_size;
	this->xio_msg->out.data_iov[0].iov_len = this->out_buf_size;
	this->xio_msg->out.data_iov[0].mr = this->xio_mr;

	this->xio_msg->in.header.iov_base = NULL;
	this->xio_msg->in.data_iovlen = 1;
	this->xio_msg->in.data_iov[0].iov_base = this->buf;
	this->xio_msg->in.data_iov[0].iov_len = this->in_buf_size;
	this->xio_msg->in.data_iov[0].mr = this->xio_mr;

	log (lsINFO, "in_size is %d out_size is %d\n", this->in_buf_size, this->out_buf_size);

}

void Msg::set_xio_msg_server_fields()
{
	this->xio_msg->out.data_iovlen = 1;
	this->xio_msg->out.data_iov[0].iov_base = this->buf + in_buf_size;
	this->xio_msg->out.data_iov[0].iov_len = this->out_buf_size;
	this->xio_msg->out.data_iov[0].mr = this->xio_mr;

	this->xio_msg->in.header.iov_base = NULL;
	this->xio_msg->in.data_iovlen = 1;
	this->xio_msg->in.data_iov[0].iov_base = this->buf;
	this->xio_msg->in.data_iov[0].iov_len = this->in_buf_size;
	this->xio_msg->in.data_iov[0].mr = this->xio_mr;
}


void Msg::set_xio_msg_fields_for_assign(struct xio_msg *msg)
{
	msg->in.data_iov[0].iov_base = buf;
	msg->in.data_iov[0].iov_len = in_size;
	msg->in.data_iov[0].mr = xio_mr;
	msg->user_context = this;
}



void Msg::dump()
{
	log (lsDEBUG, "*********************************************\n");
	log (lsDEBUG, "type:0x%x \n",  this->xio_msg->type);
	log (lsDEBUG, "status:%d \n",  this->xio_msg->status);
	if (this->xio_msg->type == XIO_MSG_TYPE_REQ)
		log (lsDEBUG, "serial number:%ld \n",  this->xio_msg->sn);
	else if (this->xio_msg->type == XIO_MSG_TYPE_RSP)
			log (lsDEBUG, "response:%p, serial number:%ld \n", this->xio_msg->request,
					((this->xio_msg->request) ? this->xio_msg->request->sn : -1));

	log (lsDEBUG, "in header: length:%d, address:%p, \n", this->xio_msg->in.header.iov_len, this->xio_msg->in.header.iov_base );
	log (lsDEBUG, "in data size:%d \n", this->xio_msg->in.data_iovlen);
	for (int i = 0; i < this->xio_msg->in.data_iovlen; i++)
		log (lsDEBUG, "in data[%d]: length:%d, address:%p, mr:%p\n", i,
				this->xio_msg->in.data_iov[i].iov_len, this->xio_msg->in.data_iov[i].iov_base, this->xio_msg->in.data_iov[i].mr);

	log (lsDEBUG, "out header: length:%d, address:%p, \n", this->xio_msg->out.header.iov_len, this->xio_msg->out.header.iov_base );
	log (lsDEBUG, "out data size:%d \n", this->xio_msg->out.data_iovlen);
		for (int i = 0; i < this->xio_msg->out.data_iovlen; i++)
			log (lsDEBUG, "out data[%d]: length:%d, address:%p, mr:%p\n", i,
					this->xio_msg->out.data_iov[i].iov_len, this->xio_msg->out.data_iov[i].iov_base, this->xio_msg->out.data_iov[i].mr);
	log (lsDEBUG, "*********************************************\n");
}


void Msg::dump(struct xio_msg *xio_msg)
{
	log (lsDEBUG, "*********************************************\n");
	log (lsDEBUG, "type:0x%x \n",  xio_msg->type);
	log (lsDEBUG, "status:%d \n",  xio_msg->status);
	if (xio_msg->type == XIO_MSG_TYPE_REQ)
		log (lsDEBUG, "serial number:%ld \n",  xio_msg->sn);
	else if (xio_msg->type == XIO_MSG_TYPE_RSP)
			log (lsDEBUG, "response:%p, serial number:%ld \n", xio_msg->request,
					((xio_msg->request) ? xio_msg->request->sn : -1));

	log (lsDEBUG, "in header: length:%d, address:%p, \n", xio_msg->in.header.iov_len, xio_msg->in.header.iov_base );
	log (lsDEBUG, "in data size:%d \n", xio_msg->in.data_iovlen);
	for (int i = 0; i < xio_msg->in.data_iovlen; i++)
		log (lsDEBUG, "in data[%d]: length:%d, address:%p, mr:%p\n", i,
				xio_msg->in.data_iov[i].iov_len, xio_msg->in.data_iov[i].iov_base, xio_msg->in.data_iov[i].mr);

	log (lsDEBUG, "out header: length:%d, address:%p, \n", xio_msg->out.header.iov_len, xio_msg->out.header.iov_base );
	log (lsDEBUG, "out data size:%d \n", xio_msg->out.data_iovlen);
		for (int i = 0; i < xio_msg->out.data_iovlen; i++)
			log (lsDEBUG, "out data[%d]: length:%d, address:%p, mr:%p\n", i,
					xio_msg->out.data_iov[i].iov_len, xio_msg->out.data_iov[i].iov_base, xio_msg->out.data_iov[i].mr);
	log (lsDEBUG, "*********************************************\n");
}



void Msg::set_xio_msg_req(struct xio_msg *m )
{
	this->xio_msg->request = m;
//	log (lsDEBUG, "inside set_req_xio_msg msg is %p req is %p\n",this->xio_msg,  this->xio_msg->request);
}


void Msg::release_to_pool()
{
	this->pool->add_msg_to_pool(this);
}

